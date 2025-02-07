/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/studies/studies.h"

#include "brave/components/brave_ads/core/internal/ads_client_helper.h"
#include "brave/components/brave_ads/core/internal/studies/studies_util.h"

namespace brave_ads {

Studies::Studies() {
  AdsClientHelper::AddObserver(this);
}

Studies::~Studies() {
  AdsClientHelper::RemoveObserver(this);
}

///////////////////////////////////////////////////////////////////////////////

void Studies::OnNotifyDidInitializeAds() {
  LogActiveStudies();
}

}  // namespace brave_ads
