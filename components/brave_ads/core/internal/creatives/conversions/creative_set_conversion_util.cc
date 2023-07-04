/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/creatives/conversions/creative_set_conversion_util.h"

#include <set>

#include "base/containers/contains.h"
#include "base/ranges/algorithm.h"
#include "brave/components/brave_ads/core/confirmation_type.h"
#include "brave/components/brave_ads/core/internal/common/url/url_util.h"
#include "brave/components/brave_ads/core/internal/conversions/conversions_util.h"
#include "brave/components/brave_ads/core/internal/conversions/types/default_conversion/creative_set_conversion_url_pattern/creative_set_conversion_url_pattern_util.h"
#include "brave/components/brave_ads/core/internal/creatives/conversions/creative_set_conversion_info.h"
#include "url/gurl.h"

namespace brave_ads {

namespace {

std::set<std::string> GetAlreadyConvertedCreativeSets(
    const AdEventList& ad_events) {
  std::set<std::string> creative_set_ids;

  for (const auto& ad_event : ad_events) {
    if (ad_event.confirmation_type == ConfirmationType::kConversion) {
      creative_set_ids.insert(ad_event.creative_set_id);
    }
  }

  return creative_set_ids;
}

}  // namespace

void FilterAlreadyConvertedCreativeSets(
    CreativeSetConversionList& creative_set_conversions,
    const AdEventList& ad_events) {
  if (creative_set_conversions.empty()) {
    return;
  }

  const std::set<std::string> converted_creative_sets =
      GetAlreadyConvertedCreativeSets(ad_events);
  creative_set_conversions.erase(
      base::ranges::remove_if(
          creative_set_conversions,
          [&converted_creative_sets](
              const CreativeSetConversionInfo& creative_set_conversion) {
            return base::Contains(converted_creative_sets,
                                  creative_set_conversion.id);
          }),
      creative_set_conversions.cend());
}

void FilterCreativeSetConversionsWithNonMatchingUrlPattern(
    CreativeSetConversionList& creative_set_conversions,
    const std::vector<GURL>& redirect_chain) {
  if (creative_set_conversions.empty()) {
    return;
  }

  creative_set_conversions.erase(
      base::ranges::remove_if(
          creative_set_conversions,
          [&redirect_chain](
              const CreativeSetConversionInfo& creative_set_conversion) {
            return !MatchUrlPattern(redirect_chain,
                                    creative_set_conversion.url_pattern);
          }),
      creative_set_conversions.cend());
}

CreativeSetConversionBuckets
FilterAndSortMatchingCreativeSetConversionsIntoBuckets(
    const CreativeSetConversionList& creative_set_conversions,
    const std::vector<GURL>& redirect_chain) {
  CreativeSetConversionBuckets buckets;

  for (const auto& creative_set_conversion : creative_set_conversions) {
    if (DoesCreativeSetConversionUrlPatternMatchRedirectChain(
            creative_set_conversion, redirect_chain)) {
      buckets[creative_set_conversion.id].push_back(creative_set_conversion);
    }
  }

  return buckets;
}

absl::optional<CreativeSetConversionInfo> FindNonExpiredCreativeSetConversion(
    const CreativeSetConversionList& creative_set_conversions,
    const AdEventInfo& ad_event) {
  const auto iter = base::ranges::find_if(
      creative_set_conversions,
      [&ad_event](const CreativeSetConversionInfo& creative_set_conversion) {
        return !HasObservationWindowForAdEventExpired(
            creative_set_conversion.observation_window, ad_event);
      });

  if (iter == creative_set_conversions.cend()) {
    return absl::nullopt;
  }

  return *iter;
}

}  // namespace brave_ads
