/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_rewards/core/wallet/wallet_util.h"

#include <algorithm>
#include <utility>

#include "base/functional/overloaded.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/notreached.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "brave/components/brave_rewards/core/bitflyer/bitflyer.h"
#include "brave/components/brave_rewards/core/bitflyer/bitflyer_util.h"
#include "brave/components/brave_rewards/core/database/database.h"
#include "brave/components/brave_rewards/core/gemini/gemini.h"
#include "brave/components/brave_rewards/core/gemini/gemini_util.h"
#include "brave/components/brave_rewards/core/global_constants.h"
#include "brave/components/brave_rewards/core/ledger_impl.h"
#include "brave/components/brave_rewards/core/logging/event_log_keys.h"
#include "brave/components/brave_rewards/core/notifications/notification_keys.h"
#include "brave/components/brave_rewards/core/state/state.h"
#include "brave/components/brave_rewards/core/state/state_keys.h"
#include "brave/components/brave_rewards/core/uphold/uphold.h"
#include "brave/components/brave_rewards/core/uphold/uphold_util.h"

namespace brave_rewards::internal::wallet {

namespace {

void OnWalletStatusChange(LedgerImpl& ledger,
                          const std::string& wallet_type,
                          absl::optional<mojom::WalletStatus> from,
                          mojom::WalletStatus to) {
  std::ostringstream oss{};
  if (from) {
    oss << *from << ' ';
  }
  oss << "==> " << to;

  ledger.database()->SaveEventLog(log::kWalletStatusChange,
                                  oss.str() + " (" + wallet_type + ')');
}

}  // namespace

mojom::ExternalWalletPtr GetWallet(LedgerImpl& ledger,
                                   const std::string& wallet_type) {
  DCHECK(!wallet_type.empty());
  auto wallet = ledger.state()->GetExternalWallet();
  if (wallet) {
    if (wallet->type != wallet_type) {
      return nullptr;
    }
    wallet = GenerateLinks(std::move(wallet));
  }
  return wallet;
}

mojom::ExternalWalletPtr GetWalletIf(
    LedgerImpl& ledger,
    const std::string& wallet_type,
    const std::set<mojom::WalletStatus>& statuses) {
  if (statuses.empty()) {
    return nullptr;
  }

  auto wallet = GetWallet(ledger, wallet_type);
  if (!wallet) {
    BLOG(9, wallet_type << " wallet is null!");
    return nullptr;
  }

  if (!statuses.count(wallet->status)) {
    std::ostringstream oss;
    auto cend = statuses.cend();
    std::copy(statuses.cbegin(), --cend,
              std::ostream_iterator<mojom::WalletStatus>(oss, ", "));
    oss << *cend;

    BLOG(9, "Unexpected state for " << wallet_type << " wallet (currently in "
                                    << wallet->status
                                    << ", expected was: " << oss.str() << ")!");
    return nullptr;
  }

  return wallet;
}

bool SetWallet(LedgerImpl& ledger, mojom::ExternalWalletPtr wallet) {
  return ledger.state()->SetExternalWallet(*wallet);
}

// Valid transition:
// - ==> kNotConnected:
//   - on wallet creation
//
// Invariants:
// - kNotConnected: token and address are cleared
mojom::ExternalWalletPtr EnsureValidCreation(const std::string& wallet_type,
                                             mojom::WalletStatus to) {
  if (to != mojom::WalletStatus::kNotConnected) {
    BLOG(0, "Attempting to create " << wallet_type << " wallet as " << to
                                    << " (a status other than "
                                       "kNotConnected)!");
    return nullptr;
  }

  auto wallet = mojom::ExternalWallet::New();
  wallet->type = wallet_type;
  wallet->status = to;

  return wallet;
}

// Valid transitions:
// - kNotConnected ==> kConnected:
//    - on successful wallet connection
// - kConnected ==> kLoggedOut:
//    - on access token expiry
//    - on losing eligibility for wallet connection (Uphold-only)
// - kLoggedOut ==> kConnected:
//    - on successful (re)connection
//
// Invariants:
// - kConnected: needs !token.empty() && !address.empty()
// - kLoggedOut: token and address are cleared
mojom::ExternalWalletPtr EnsureValidTransition(mojom::ExternalWalletPtr wallet,
                                               mojom::WalletStatus to) {
  DCHECK(wallet);
  const auto wallet_type = wallet->type;
  const auto from = wallet->status;

  // kNotConnected ==> kConnected
  const bool wallet_connection = from == mojom::WalletStatus::kNotConnected &&
                                 to == mojom::WalletStatus::kConnected;
  // kConnected ==> kLoggedOut
  const bool wallet_logout = from == mojom::WalletStatus::kConnected &&
                             to == mojom::WalletStatus::kLoggedOut;
  // kLoggedOut ==> kConnected
  const bool wallet_reconnection = from == mojom::WalletStatus::kLoggedOut &&
                                   to == mojom::WalletStatus::kConnected;

  if (!wallet_connection && !wallet_logout && !wallet_reconnection) {
    BLOG(0, "Invalid " << wallet_type << " wallet status transition: " << from
                       << " ==> " << to << '!');
    return nullptr;
  }

  switch (to) {
    case mojom::WalletStatus::kConnected:
      if (wallet->token.empty() || wallet->address.empty()) {
        BLOG(0, "Invariant violation when attempting to transition "
                    << wallet->type << " wallet status (" << from << " ==> "
                    << to << ")!");
        return nullptr;
      }

      break;
    case mojom::WalletStatus::kLoggedOut:
      // token.empty() && address.empty()
      wallet = mojom::ExternalWallet::New();
      wallet->type = wallet_type;
      break;
    case mojom::WalletStatus::kNotConnected:
      NOTREACHED()
          << "No transitions to kNotConnected are permitted (except for "
             "when the wallet is being created)!";
      return nullptr;
  }

  wallet->status = to;

  return wallet;
}

mojom::ExternalWalletPtr TransitionWallet(
    LedgerImpl& ledger,
    absl::variant<mojom::ExternalWalletPtr, std::string> wallet_info,
    mojom::WalletStatus to) {
  absl::optional<mojom::WalletStatus> from;

  auto wallet = absl::visit(
      base::Overloaded{
          [&](const std::string& wallet_type) -> mojom::ExternalWalletPtr {
            auto wallet = GetWallet(ledger, wallet_type);
            if (wallet) {
              BLOG(0, wallet_type << " wallet already exists!");
              return nullptr;
            }

            return EnsureValidCreation(wallet_type, to);
          },
          [&](mojom::ExternalWalletPtr wallet) -> mojom::ExternalWalletPtr {
            DCHECK(wallet);
            if (!wallet) {
              BLOG(0, "Wallet is null!");
              return nullptr;
            }

            from = wallet->status;

            return EnsureValidTransition(std::move(wallet), to);
          }},
      std::move(wallet_info));

  if (!wallet) {
    return nullptr;
  }

  wallet->one_time_string = ledger.oauth_login_manager()->one_time_string();
  wallet->code_verifier = ledger.oauth_login_manager()->code_verifier();

  wallet = GenerateLinks(std::move(wallet));
  if (!wallet) {
    BLOG(0, "Failed to generate links for wallet!");
    return nullptr;
  }

  if (!SetWallet(ledger, wallet->Clone())) {
    BLOG(0, "Failed to set " << wallet->type << " wallet!");
    return nullptr;
  }

  OnWalletStatusChange(ledger, wallet->type, from, to);

  return wallet;
}

mojom::ExternalWalletPtr MaybeCreateWallet(LedgerImpl& ledger,
                                           const std::string& wallet_type) {
  auto wallet = GetWallet(ledger, wallet_type);
  if (!wallet) {
    wallet = TransitionWallet(ledger, wallet_type,
                              mojom::WalletStatus::kNotConnected);
    if (!wallet) {
      BLOG(0, "Failed to create " << wallet_type << " wallet!");
    }
  }

  return wallet;
}

bool LogOutWallet(LedgerImpl& ledger,
                  const std::string& wallet_type,
                  const std::string& notification) {
  DCHECK(!wallet_type.empty());

  BLOG(1, "Logging out " << wallet_type << " wallet...");

  auto wallet =
      GetWalletIf(ledger, wallet_type, {mojom::WalletStatus::kConnected});
  if (!wallet) {
    return false;
  }

  const std::string abbreviated_address = wallet->address.substr(0, 5);
  wallet = TransitionWallet(ledger, std::move(wallet),
                            mojom::WalletStatus::kLoggedOut);
  if (!wallet) {
    return false;
  }

  ledger.database()->SaveEventLog(log::kWalletDisconnected,
                                  wallet_type + abbreviated_address);

  if (!ledger.IsShuttingDown()) {
    ledger.client()->ExternalWalletLoggedOut();
    ledger.client()->ShowNotification(notification.empty()
                                          ? notifications::kWalletDisconnected
                                          : notification,
                                      {}, base::DoNothing());
  }

  return true;
}

mojom::ExternalWalletPtr GenerateLinks(mojom::ExternalWalletPtr wallet) {
  if (wallet->type == constant::kWalletBitflyer) {
    return bitflyer::GenerateLinks(std::move(wallet));
  } else if (wallet->type == constant::kWalletGemini) {
    return gemini::GenerateLinks(std::move(wallet));
  } else if (wallet->type == constant::kWalletUphold) {
    return uphold::GenerateLinks(std::move(wallet));
  } else if (wallet->type == "test") {
    return wallet;
  } else {
    NOTREACHED() << "Unexpected wallet type " << wallet->type << '!';
    return nullptr;
  }
}

void FetchBalance(LedgerImpl& ledger,
                  const std::string& wallet_type,
                  base::OnceCallback<void(mojom::Result, double)> callback) {
  if (wallet_type == constant::kWalletBitflyer) {
    ledger.bitflyer()->FetchBalance(std::move(callback));
  } else if (wallet_type == constant::kWalletGemini) {
    ledger.gemini()->FetchBalance(std::move(callback));
  } else if (wallet_type == constant::kWalletUphold) {
    ledger.uphold()->FetchBalance(std::move(callback));
  } else {
    NOTREACHED() << "Unexpected wallet type " << wallet_type << '!';
    std::move(callback).Run(mojom::Result::LEDGER_ERROR, 0.0);
  }
}

}  // namespace brave_rewards::internal::wallet
