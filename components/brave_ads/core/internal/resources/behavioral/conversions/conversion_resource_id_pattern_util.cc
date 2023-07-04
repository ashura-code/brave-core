/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/resources/behavioral/conversions/conversion_resource_id_pattern_util.h"

#include "brave/components/brave_ads/core/internal/common/url/url_util.h"
#include "url/gurl.h"

namespace brave_ads {

absl::optional<ConversionResourceIdPatternInfo>
FindMatchingConversionResourceIdPattern(
    const ConversionResourceIdPatternMap& resource_id_patterns,
    const std::vector<GURL>& redirect_chain) {
  for (const auto& [url_pattern, resource_id_pattern] : resource_id_patterns) {
    if (MatchUrlPattern(redirect_chain, url_pattern)) {
      return resource_id_pattern;
    }
  }

  return absl::nullopt;
}

}  // namespace brave_ads
