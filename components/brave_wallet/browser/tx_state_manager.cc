/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_wallet/browser/tx_state_manager.h"

#include <utility>

#include "base/json/values_util.h"
#include "base/values.h"
#include "brave/components/brave_wallet/browser/brave_wallet_constants.h"
#include "brave/components/brave_wallet/browser/brave_wallet_utils.h"
#include "brave/components/brave_wallet/browser/pref_names.h"
#include "brave/components/brave_wallet/browser/solana_message.h"
#include "brave/components/brave_wallet/browser/tx_meta.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "components/value_store/value_store_frontend.h"
#include "url/origin.h"

namespace brave_wallet {

namespace {

constexpr size_t kMaxConfirmedTxNum = 500;
constexpr size_t kMaxRejectedTxNum = 500;
constexpr char kStorageTransactions[] = "transactions";
constexpr size_t kMaxInitRetryAttempts = 3;
constexpr int kDelayedInitRetryMsec = 500;

}  // namespace

// static
bool TxStateManager::ValueToTxMeta(const base::Value::Dict& value,
                                   TxMeta* meta) {
  const std::string* id = value.FindString("id");
  if (!id) {
    return false;
  }
  meta->set_id(*id);

  absl::optional<int> status = value.FindInt("status");
  if (!status) {
    return false;
  }
  meta->set_status(static_cast<mojom::TransactionStatus>(*status));
  const std::string* from = value.FindString("from");
  if (!from) {
    return false;
  }
  meta->set_from(*from);

  const base::Value* created_time = value.Find("created_time");
  if (!created_time) {
    return false;
  }
  absl::optional<base::Time> created_time_from_value =
      base::ValueToTime(created_time);
  if (!created_time_from_value) {
    return false;
  }
  meta->set_created_time(*created_time_from_value);

  const base::Value* submitted_time = value.Find("submitted_time");
  if (!submitted_time) {
    return false;
  }
  absl::optional<base::Time> submitted_time_from_value =
      base::ValueToTime(submitted_time);
  if (!submitted_time_from_value) {
    return false;
  }
  meta->set_submitted_time(*submitted_time_from_value);

  const base::Value* confirmed_time = value.Find("confirmed_time");
  if (!confirmed_time) {
    return false;
  }
  absl::optional<base::Time> confirmed_time_from_value =
      base::ValueToTime(confirmed_time);
  if (!confirmed_time_from_value) {
    return false;
  }
  meta->set_confirmed_time(*confirmed_time_from_value);

  const std::string* tx_hash = value.FindString("tx_hash");
  if (!tx_hash) {
    return false;
  }
  meta->set_tx_hash(*tx_hash);

  const std::string* origin_spec = value.FindString("origin");
  // That's ok not to have origin.
  if (origin_spec) {
    meta->set_origin(url::Origin::Create(GURL(*origin_spec)));
    DCHECK(!meta->origin()->opaque());
  }

  const std::string* group_id = value.FindString("group_id");
  if (group_id) {
    meta->set_group_id(*group_id);
  }

  const auto* chain_id_string = value.FindString("chain_id");
  if (!chain_id_string) {
    return false;
  }
  meta->set_chain_id(*chain_id_string);

  return true;
}

TxStateManager::TxStateManager(PrefService* prefs,
                               value_store::ValueStoreFrontend* store)
    : prefs_(prefs), initialized_(false), store_(store), weak_factory_(this) {
  Initialize();
}

TxStateManager::~TxStateManager() = default;

void TxStateManager::Initialize() {
  store_->Get(kStorageTransactions, base::BindOnce(&TxStateManager::OnTxsRead,
                                                   weak_factory_.GetWeakPtr()));
}

void TxStateManager::OnTxsRead(absl::optional<base::Value> txs) {
  if (txs) {
    txs_ = std::move(txs->GetDict());
  }
  initialized_ = true;
}

void TxStateManager::ScheduleWrite(size_t retry_attempts) {
  // Schedule a delayed task with maximum retry.
  if (!initialized_) {
    if (retry_attempts < kMaxInitRetryAttempts) {
      base::SequencedTaskRunner::GetCurrentDefault()->PostDelayedTask(
          FROM_HERE,
          base::BindOnce(&TxStateManager::ScheduleWrite,
                         weak_factory_.GetWeakPtr(), retry_attempts + 1),
          base::Milliseconds(kDelayedInitRetryMsec));
    } else {
      LOG(ERROR)
          << "TxStateManager wait for init maximum retry attempts reached.";
      // Skip and wait for the next one.
      return;
    }
  }
  store_->Set(kStorageTransactions, base::Value(txs_.Clone()));
}

void TxStateManager::AddOrUpdateTx(const TxMeta& meta) {
  const std::string path =
      base::JoinString({GetTxPrefPathPrefix(meta.chain_id()), meta.id()}, ".");

  bool is_add = txs_.FindByDottedPath(path) == nullptr;
  txs_.SetByDottedPath(path, meta.ToValue());
  ScheduleWrite(0);
  if (!is_add) {
    for (auto& observer : observers_) {
      observer.OnTransactionStatusChanged(meta.ToTransactionInfo());
    }
  } else {
    for (auto& observer : observers_) {
      observer.OnNewUnapprovedTx(meta.ToTransactionInfo());
    }

    // We only keep most recent 1k confirmed plus rejected tx metas per network
    RetireTxByStatus(meta.chain_id(), mojom::TransactionStatus::Confirmed,
                     kMaxConfirmedTxNum);
    RetireTxByStatus(meta.chain_id(), mojom::TransactionStatus::Rejected,
                     kMaxRejectedTxNum);
  }
}

std::unique_ptr<TxMeta> TxStateManager::GetTx(const std::string& chain_id,
                                              const std::string& id) {
  const base::Value::Dict* value = txs_.FindDictByDottedPath(
      base::JoinString({GetTxPrefPathPrefix(chain_id), id}, "."));
  if (!value) {
    return nullptr;
  }

  return ValueToTxMeta(*value);
}

void TxStateManager::DeleteTx(const std::string& chain_id,
                              const std::string& id) {
  txs_.RemoveByDottedPath(
      base::JoinString({GetTxPrefPathPrefix(chain_id), id}, "."));
  ScheduleWrite(0);
}

void TxStateManager::WipeTxs() {
  txs_.RemoveByDottedPath(GetTxPrefPathPrefix(absl::nullopt));
  ScheduleWrite(0);
}

std::vector<std::unique_ptr<TxMeta>> TxStateManager::GetTransactionsByStatus(
    const absl::optional<std::string>& chain_id,
    const absl::optional<mojom::TransactionStatus>& status,
    const absl::optional<std::string>& from) {
  std::vector<std::unique_ptr<TxMeta>> result;
  const base::Value::Dict* network_dict =
      txs_.FindDictByDottedPath(GetTxPrefPathPrefix(chain_id));
  if (!network_dict) {
    return result;
  }

  for (const auto it : *network_dict) {
    if (chain_id.has_value()) {
      std::unique_ptr<TxMeta> meta = ValueToTxMeta(it.second.GetDict());
      if (!meta) {
        continue;
      }
      if (!status.has_value() || meta->status() == *status) {
        if (from.has_value() && meta->from() != *from) {
          continue;
        }
        result.push_back(std::move(meta));
      }
    } else {
      auto chain_id_from_pref =
          GetChainIdByNetworkId(prefs_, GetCoinType(), it.first);
      if (!chain_id_from_pref) {
        continue;
      }
      auto metas = GetTransactionsByStatus(chain_id_from_pref, status, from);
      result.insert(result.end(), std::make_move_iterator(metas.begin()),
                    std::make_move_iterator(metas.end()));
    }
  }
  return result;
}

void TxStateManager::RetireTxByStatus(const std::string& chain_id,
                                      mojom::TransactionStatus status,
                                      size_t max_num) {
  if (status != mojom::TransactionStatus::Confirmed &&
      status != mojom::TransactionStatus::Rejected) {
    return;
  }
  auto tx_metas = GetTransactionsByStatus(chain_id, status, absl::nullopt);
  if (tx_metas.size() > max_num) {
    TxMeta* oldest_meta = nullptr;
    for (const auto& tx_meta : tx_metas) {
      if (!oldest_meta) {
        oldest_meta = tx_meta.get();
      } else {
        if (tx_meta->status() == mojom::TransactionStatus::Confirmed &&
            tx_meta->confirmed_time() < oldest_meta->confirmed_time()) {
          oldest_meta = tx_meta.get();
        } else if (tx_meta->status() == mojom::TransactionStatus::Rejected &&
                   tx_meta->created_time() < oldest_meta->created_time()) {
          oldest_meta = tx_meta.get();
        }
      }
    }
    DCHECK(oldest_meta);
    DeleteTx(chain_id, oldest_meta->id());
  }
}

void TxStateManager::AddObserver(TxStateManager::Observer* observer) {
  observers_.AddObserver(observer);
}

void TxStateManager::RemoveObserver(TxStateManager::Observer* observer) {
  observers_.RemoveObserver(observer);
}

void TxStateManager::MigrateAddChainIdToTransactionInfo(PrefService* prefs) {
  if (prefs->GetBoolean(kBraveWalletTransactionsChainIdMigrated)) {
    return;
  }
  if (!prefs->HasPrefPath(kBraveWalletTransactions)) {
    prefs->SetBoolean(kBraveWalletTransactionsChainIdMigrated, true);
    return;
  }

  ScopedDictPrefUpdate txs_update(prefs, kBraveWalletTransactions);
  auto& all_txs = txs_update.Get();

  auto set_chain_id = [&](base::Value::Dict* tx_by_network_ids,
                          const mojom::CoinType& coin) {
    for (auto tnid : *tx_by_network_ids) {
      auto chain_id = GetChainIdByNetworkId(prefs, coin, tnid.first);
      if (!chain_id.has_value()) {
        continue;
      }

      auto* txs = tnid.second.GetIfDict();

      if (!txs || txs->empty()) {
        return;
      }
      for (auto tx : *txs) {
        auto* ptx = tx.second.GetIfDict();

        if (!ptx) {
          continue;
        }

        ptx->Set("chain_id", chain_id.value());
      }
    }
  };

  for (auto txs_coin_type : all_txs) {
    auto coin = GetCoinTypeFromPrefKey(txs_coin_type.first);

    if (!coin.has_value()) {
      continue;
    }

    if (!txs_coin_type.second.is_dict()) {
      continue;
    }

    set_chain_id(&txs_coin_type.second.GetDict(), coin.value());
  }
  prefs->SetBoolean(kBraveWalletTransactionsChainIdMigrated, true);
}

void TxStateManager::MigrateSolanaTransactionsForV0TransactionsSupport(
    PrefService* prefs) {
  if (prefs->GetBoolean(kBraveWalletSolanaTransactionsV0SupportMigrated)) {
    return;
  }

  if (!prefs->HasPrefPath(kBraveWalletTransactions)) {
    prefs->SetBoolean(kBraveWalletSolanaTransactionsV0SupportMigrated, true);
    return;
  }

  // Get message dict via solana.network_name.tx_id.tx.message. (example path:
  // solana.devnet.tx_id1.tx.message)
  // Then update message using SolanaMessage::FromDeprecatedLegacyValue and
  // SolanaMessage::ToValue.
  ScopedDictPrefUpdate update(prefs, kBraveWalletTransactions);
  base::Value::Dict* sol_txs = update.Get().FindDict(kSolanaPrefKey);
  if (!sol_txs) {
    prefs->SetBoolean(kBraveWalletSolanaTransactionsV0SupportMigrated, true);
    return;
  }

  for (auto txs_by_networks : *sol_txs) {
    if (!txs_by_networks.second.is_dict()) {
      continue;
    }

    for (auto txs_by_ids : txs_by_networks.second.GetDict()) {
      if (!txs_by_ids.second.is_dict()) {
        continue;
      }

      auto* tx_message =
          txs_by_ids.second.GetDict().FindDictByDottedPath("tx.message");
      if (!tx_message) {
        continue;
      }

      auto message = SolanaMessage::FromDeprecatedLegacyValue(*tx_message);
      if (!message) {
        continue;
      }

      *tx_message = message->ToValue();
    }
  }

  prefs->SetBoolean(kBraveWalletSolanaTransactionsV0SupportMigrated, true);
}

// static
void TxStateManager::MigrateTransactionsFromPrefsToDB(
    PrefService* prefs,
    value_store::ValueStoreFrontend* store) {
  if (prefs->GetBoolean(kBraveWalletTransactionsFromPrefsToDBMigrated)) {
    return;
  }

  if (!prefs->HasPrefPath(kBraveWalletTransactions)) {
    prefs->SetBoolean(kBraveWalletTransactionsFromPrefsToDBMigrated, true);
    return;
  }

  const auto& txs = prefs->GetDict(kBraveWalletTransactions);
  store->Set(kStorageTransactions, base::Value(txs.Clone()));

  // Keep kBraveWalletTransactions in case we need to revert the migration and
  // remove it when we delete the pref
  prefs->SetBoolean(kBraveWalletTransactionsFromPrefsToDBMigrated, true);
}

}  // namespace brave_wallet
