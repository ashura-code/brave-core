/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/conversions/types/default_conversion/creative_set_conversion_url_pattern/creative_set_conversion_url_pattern_util.h"

#include "brave/components/brave_ads/core/internal/ads/ad_unittest_constants.h"
#include "brave/components/brave_ads/core/internal/common/unittest/unittest_base.h"
#include "brave/components/brave_ads/core/internal/common/unittest/unittest_time_util.h"
#include "brave/components/brave_ads/core/internal/conversions/types/verifiable_conversion/verifiable_conversion_unittest_constants.h"

// npm run test -- brave_unit_tests --filter=BraveAds*

namespace brave_ads {

class BraveAdsCreativeConversionSetUrlPatternUtilTest : public UnitTestBase {};

TEST_F(BraveAdsCreativeConversionSetUrlPatternUtilTest,
       DoesCreativeSetConversionUrlPatternMatchRedirectChain) {
  // Arrange
  CreativeSetConversionInfo creative_set_conversion;
  creative_set_conversion.id = kCreativeSetId;
  creative_set_conversion.url_pattern = "https://foo.com/*";
  creative_set_conversion.extract_verifiable_id = true;
  creative_set_conversion.verifiable_advertiser_public_key_base64 =
      kVerifiableConversionAdvertiserPublicKey;
  creative_set_conversion.observation_window = base::Days(3);
  creative_set_conversion.expire_at =
      Now() + creative_set_conversion.observation_window;

  // Act

  // Assert
  EXPECT_TRUE(DoesCreativeSetConversionUrlPatternMatchRedirectChain(
      creative_set_conversion,
      /*redirect_chain*/ {GURL("https://foo.com/bar")}));
}

TEST_F(BraveAdsCreativeConversionSetUrlPatternUtilTest,
       DoesCreativeSetConversionUrlPatternNotMatchRedirectChain) {
  // Arrange
  // TODO(tmancey): DRY.
  CreativeSetConversionInfo creative_set_conversion;
  creative_set_conversion.id = kCreativeSetId;
  creative_set_conversion.url_pattern = "https://foo.com/*";
  creative_set_conversion.extract_verifiable_id = true;
  creative_set_conversion.verifiable_advertiser_public_key_base64 =
      kVerifiableConversionAdvertiserPublicKey;
  creative_set_conversion.observation_window = base::Days(3);
  creative_set_conversion.expire_at =
      Now() + creative_set_conversion.observation_window;

  // Act

  // Assert
  EXPECT_FALSE(DoesCreativeSetConversionUrlPatternMatchRedirectChain(
      creative_set_conversion,
      /*redirect_chain*/ {GURL("https://bar.com/foo")}));
}

}  // namespace brave_ads
