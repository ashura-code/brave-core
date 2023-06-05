/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_rewards/core/wallet/oauth_login_manager.h"

#include "brave/components/brave_rewards/core/common/random_util.h"

namespace brave_rewards::internal::wallet {

OAuthLoginManager::OAuthLoginManager() {
  StartLoginRequest();
}

OAuthLoginManager::~OAuthLoginManager() = default;

void OAuthLoginManager::StartLoginRequest() {
  ClearLoginRequest();
  one_time_string_ = util::GenerateRandomHexString();
  code_verifier_ = util::GeneratePKCECodeVerifier();
}

void OAuthLoginManager::ClearLoginRequest() {
  one_time_string_.clear();
  code_verifier_.clear();
}

}  // namespace brave_rewards::internal::wallet
