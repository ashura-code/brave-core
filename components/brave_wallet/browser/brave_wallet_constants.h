/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_WALLET_BROWSER_BRAVE_WALLET_CONSTANTS_H_
#define BRAVE_COMPONENTS_BRAVE_WALLET_BROWSER_BRAVE_WALLET_CONSTANTS_H_

#include <vector>

#include "base/no_destructor.h"
#include "brave/components/brave_wallet/common/brave_wallet.mojom.h"

namespace brave_wallet {

enum class Web3ProviderTypes {
  ASK,
  NONE,
  CRYPTO_WALLETS,
  METAMASK,
  BRAVE_WALLET
};

extern const char kAssetRatioBaseURL[];
extern const char kSwapBaseURL[];

// List of assets from Wyre, available to buy
static base::NoDestructor<std::vector<mojom::ERCToken>> kBuyTokens(
    {{"", "Basic Attention Token", true, false, "BAT", 0, true},
     {"", "Ethereum", false, false, "ETH", 0, true},
     {"", "USD Coin", true, false, "USDC", 0, true},
     {"", "DAI", true, false, "DAI", 0, true},
     {"", "AAVE", true, false, "AAVE", 0, true},
     {"", "Binance USD", true, false, "BUSD", 0, true},
     {"", "Compound", true, false, "Comp", 0, true},
     {"", "Curve", true, false, "CRV", 0, true},
     {"", "Gemini Dollar", true, false, "GUSD", 0, true},
     {"", "Chainlink", true, false, "LINK", 0, true},
     {"", "Maker", true, false, "MKR", 0, true},
     {"", "Paxos Standard", true, false, "PAX", 0, true},
     {"", "Synthetix", true, false, "SNX", 0, true},
     {"", "UMA", true, false, "UMA", 0, true},
     {"", "Uniswap", true, false, "UNI", 0, true},
     {"", "Stably Dollar", true, false, "USDS", 0, true},
     {"", "Tether", true, false, "USDT", 0, true},
     {"", "Wrapped Bitcoin", true, false, "WBTC", 0, true},
     {"", "Year.Finance", true, false, "YFI", 0, true},
     {"", "Palm DAI", true, false, "PDAI", 0, true},
     {"", "Matic USDC", true, false, "MUSDC", 0, true}});

}  // namespace brave_wallet

#endif  // BRAVE_COMPONENTS_BRAVE_WALLET_BROWSER_BRAVE_WALLET_CONSTANTS_H_
