/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_REWARDS_CORE_WALLET_OAUTH_LOGIN_MANAGER_H_
#define BRAVE_COMPONENTS_BRAVE_REWARDS_CORE_WALLET_OAUTH_LOGIN_MANAGER_H_

#include <string>

namespace brave_rewards::internal::wallet {

class OAuthLoginManager {
 public:
  OAuthLoginManager();
  ~OAuthLoginManager();

  OAuthLoginManager(const OAuthLoginManager&) = delete;
  OAuthLoginManager& operator=(const OAuthLoginManager&) = delete;

  void StartLoginRequest();
  void ClearLoginRequest();

  const std::string& one_time_string() const { return one_time_string_; }
  const std::string& code_verifier() const { return code_verifier_; }

 private:
  std::string one_time_string_;
  std::string code_verifier_;
};

}  // namespace brave_rewards::internal::wallet

#endif  // BRAVE_COMPONENTS_BRAVE_REWARDS_CORE_WALLET_OAUTH_LOGIN_MANAGER_H_
