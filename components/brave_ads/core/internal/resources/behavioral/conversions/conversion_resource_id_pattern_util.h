/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_RESOURCES_BEHAVIORAL_CONVERSIONS_CONVERSION_RESOURCE_ID_PATTERN_UTIL_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_RESOURCES_BEHAVIORAL_CONVERSIONS_CONVERSION_RESOURCE_ID_PATTERN_UTIL_H_

#include <vector>

#include "brave/components/brave_ads/core/internal/resources/behavioral/conversions/conversion_resource_id_pattern_info.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

class GURL;

namespace brave_ads {

absl::optional<ConversionResourceIdPatternInfo>
FindMatchingConversionResourceIdPattern(
    const ConversionResourceIdPatternMap& resource_id_patterns,
    const std::vector<GURL>& redirect_chain);

}  // namespace brave_ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_RESOURCES_BEHAVIORAL_CONVERSIONS_CONVERSION_RESOURCE_ID_PATTERN_UTIL_H_
