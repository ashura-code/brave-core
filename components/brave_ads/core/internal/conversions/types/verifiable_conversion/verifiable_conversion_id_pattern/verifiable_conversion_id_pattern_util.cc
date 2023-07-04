/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/conversions/types/verifiable_conversion/verifiable_conversion_id_pattern/verifiable_conversion_id_pattern_util.h"

#include "brave/components/brave_ads/core/internal/conversions/types/verifiable_conversion/verifiable_conversion_id_pattern/parsers/verifiable_conversion_id_html_meta_tag_parser_util.h"
#include "brave/components/brave_ads/core/internal/conversions/types/verifiable_conversion/verifiable_conversion_id_pattern/parsers/verifiable_conversion_id_html_parser_util.h"
#include "brave/components/brave_ads/core/internal/conversions/types/verifiable_conversion/verifiable_conversion_id_pattern/parsers/verifiable_conversion_id_url_redirects_parser_util.h"
#include "brave/components/brave_ads/core/internal/resources/behavioral/conversions/conversion_resource_id_pattern_info.h"
#include "brave/components/brave_ads/core/internal/resources/behavioral/conversions/conversion_resource_id_pattern_search_in_types.h"
#include "url/gurl.h"

namespace brave_ads {

absl::optional<std::string> MaybeParseVerifiableConversionId(
    const std::string& html,
    const std::vector<GURL>& redirect_chain,
    const ConversionResourceIdPatternInfo& resource_id_pattern) {
  switch (resource_id_pattern.search_in_type) {
    case ConversionResourceIdPatternSearchInType::kUrlRedirect: {
      return MaybeParseVerifableConversionIdFromUrlRedirects(
          redirect_chain, resource_id_pattern);
    }

    case ConversionResourceIdPatternSearchInType::kHtml: {
      return MaybeParseVerifableConversionIdFromHtml(html, resource_id_pattern);
    }

    default: {
      return MaybeParseVerifableConversionIdFromHtmlMetaTag(html);
    }
  }
}

}  // namespace brave_ads
