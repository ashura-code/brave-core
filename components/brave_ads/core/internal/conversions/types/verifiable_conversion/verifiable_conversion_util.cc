/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/conversions/types/verifiable_conversion/verifiable_conversion_util.h"

#include "brave/components/brave_ads/core/internal/conversions/types/verifiable_conversion/verifiable_conversion_id_pattern/verifiable_conversion_id_pattern_util.h"
#include "brave/components/brave_ads/core/internal/conversions/types/verifiable_conversion/verifiable_conversion_info.h"
#include "brave/components/brave_ads/core/internal/creatives/conversions/creative_set_conversion_info.h"
#include "brave/components/brave_ads/core/internal/resources/behavioral/conversions/conversion_resource_id_pattern_util.h"
#include "url/gurl.h"

namespace brave_ads {

namespace {

bool ShouldExtractVerifiableConversionId(
    const CreativeSetConversionInfo& creative_set_conversion) {
  return creative_set_conversion.extract_verifiable_id &&
         !creative_set_conversion.verifiable_advertiser_public_key_base64
              .empty();
}

absl::optional<std::string> MaybeGetVerifiableConversionId(
    const std::vector<GURL>& redirect_chain,
    const ConversionResourceIdPatternMap& resource_id_patterns,
    const std::string& html) {
  const absl::optional<ConversionResourceIdPatternInfo> resource_id_pattern =
      FindMatchingConversionResourceIdPattern(resource_id_patterns,
                                              redirect_chain);
  if (!resource_id_pattern) {
    return absl::nullopt;
  }

  return MaybeParseVerifiableConversionId(html, redirect_chain,
                                          *resource_id_pattern);
}

}  // namespace

absl::optional<VerifiableConversionInfo> MaybeBuildVerifiableConversion(
    const std::vector<GURL>& redirect_chain,
    const ConversionResourceIdPatternMap& resource_id_patterns,
    const std::string& html,
    const CreativeSetConversionInfo& creative_set_conversion) {
  if (!ShouldExtractVerifiableConversionId(creative_set_conversion)) {
    return absl::nullopt;
  }

  const absl::optional<std::string> verifiable_conversion_id =
      MaybeGetVerifiableConversionId(redirect_chain, resource_id_patterns,
                                     html);
  if (!verifiable_conversion_id) {
    return absl::nullopt;
  }

  return VerifiableConversionInfo{
      *verifiable_conversion_id,
      creative_set_conversion.verifiable_advertiser_public_key_base64};
}

}  // namespace brave_ads
