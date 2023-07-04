/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/creatives/conversions/creative_set_conversion_util.h"

#include "base/time/time.h"
#include "brave/components/brave_ads/core/internal/ads/ad_events/ad_event_builder.h"
#include "brave/components/brave_ads/core/internal/ads/ad_unittest_constants.h"
#include "brave/components/brave_ads/core/internal/ads/ad_unittest_util.h"
#include "brave/components/brave_ads/core/internal/common/unittest/unittest_base.h"
#include "brave/components/brave_ads/core/internal/common/unittest/unittest_time_util.h"
#include "brave/components/brave_ads/core/internal/creatives/conversions/creative_set_conversion_unittest_util.h"
#include "url/gurl.h"

// npm run test -- brave_unit_tests --filter=BraveAds*

namespace brave_ads {

class BraveAdsCreativeSetConversionUtilTest : public UnitTestBase {};

TEST_F(BraveAdsCreativeSetConversionUtilTest,
       FilterAndSortMatchingCreativeSetConversionsIntoBuckets) {
  // Arrange
  CreativeSetConversionList creative_set_conversions;

  CreativeSetConversionInfo creative_set_conversion_1 =
      BuildCreativeSetConversion(
          kCreativeSetId,
          /*url_pattern*/ "https://foo.com/*",
          /*observation_window*/ base::Days(3));  // Bucket #1
  creative_set_conversions.push_back(creative_set_conversion_1);

  CreativeSetConversionInfo creative_set_conversion_2 =
      BuildCreativeSetConversion(
          /*creative_set_id*/ "1e945c25-98a2-443c-a7f5-e695110d2b84",
          /*url_pattern*/ "https://www.qux.com/",
          /*observation_window*/ base::Days(7));  // Bucket #2
  creative_set_conversions.push_back(creative_set_conversion_2);

  CreativeSetConversionInfo creative_set_conversion_3 =
      BuildCreativeSetConversion(
          kCreativeSetId,
          /*url_pattern*/ "https://baz.com/",
          /*observation_window*/ base::Days(30));  // Bucket #1
  creative_set_conversions.push_back(creative_set_conversion_3);

  CreativeSetConversionInfo creative_set_conversion_4 =
      BuildCreativeSetConversion(
          /*creative_set_id*/ "75d4cbac-b661-4126-9ccb-7bbb6ee56ef3",
          /*url_pattern*/ "https://garbly.com/fred",
          /*observation_window*/ base::Days(1));
  creative_set_conversions.push_back(creative_set_conversion_4);

  const std::vector<GURL> redirect_chain = {
      GURL("https://baz.com/"), GURL("https://foo.com/bar"),
      GURL("https://www.qux.com/"), GURL("https://quux.com/corge/grault"),
      GURL("https://garbly.com/waldo")};

  // Act
  const CreativeSetConversionBuckets buckets =
      FilterAndSortMatchingCreativeSetConversionsIntoBuckets(
          creative_set_conversions, redirect_chain);

  // Assert
  CreativeSetConversionBuckets expected_buckets;
  expected_buckets.insert(  // Bucket #1
      {kCreativeSetId, {creative_set_conversion_1, creative_set_conversion_3}});
  expected_buckets.insert(  // Bucket #2
      {creative_set_conversion_2.id, {creative_set_conversion_2}});
  EXPECT_EQ(expected_buckets, buckets);
}

TEST_F(BraveAdsCreativeSetConversionUtilTest,
       SortEmptyCreativeSetConversionsIntoBuckets) {
  // Arrange
  const CreativeSetConversionList creative_set_conversions;

  const std::vector<GURL> redirect_chain = {GURL("https://brave.com/")};

  // Act
  const CreativeSetConversionBuckets buckets =
      FilterAndSortMatchingCreativeSetConversionsIntoBuckets(
          creative_set_conversions, redirect_chain);

  // Assert
  EXPECT_TRUE(buckets.empty());
}

TEST_F(BraveAdsCreativeSetConversionUtilTest,
       FilterAlreadyConvertedCreativeSets) {
  // Arrange
  const AdInfo ad =
      BuildAd(AdType::kNotificationAd, /*should_use_random_uuids*/ true);

  AdEventList ad_events;
  const AdEventInfo ad_event =
      BuildAdEvent(ad, ConfirmationType::kConversion, /*created_at*/ Now());
  ad_events.push_back(ad_event);

  CreativeSetConversionList creative_set_conversions;

  CreativeSetConversionInfo creative_set_conversion_1;
  creative_set_conversion_1.id = kCreativeSetId;
  creative_set_conversion_1.url_pattern = "https://foo.com/*";
  creative_set_conversion_1.observation_window = base::Days(3);
  creative_set_conversion_1.expire_at =
      Now() + creative_set_conversion_1.observation_window;
  creative_set_conversions.push_back(creative_set_conversion_1);

  CreativeSetConversionInfo creative_set_conversion_2;  // Converted
  creative_set_conversion_2.id = ad.creative_set_id;
  creative_set_conversion_2.url_pattern = "https://www.qux.com/";
  creative_set_conversion_2.observation_window = base::Days(7);
  creative_set_conversion_2.expire_at =
      Now() + creative_set_conversion_2.observation_window;
  creative_set_conversions.push_back(creative_set_conversion_2);

  // Act
  FilterAlreadyConvertedCreativeSets(creative_set_conversions, ad_events);

  // Assert
  const CreativeSetConversionList expected_creative_set_conversions = {
      creative_set_conversion_1};
  EXPECT_EQ(expected_creative_set_conversions, creative_set_conversions);
}

TEST_F(BraveAdsCreativeSetConversionUtilTest,
       FilterCreativeSetConversionsWithNonMatchingUrlPattern) {
  // Arrange
  CreativeSetConversionList creative_set_conversions;

  CreativeSetConversionInfo creative_set_conversion_1;
  creative_set_conversion_1.id = kCreativeSetId;
  creative_set_conversion_1.url_pattern = "https://foo.com/*";
  creative_set_conversion_1.observation_window = base::Days(3);
  creative_set_conversion_1.expire_at =
      Now() + creative_set_conversion_1.observation_window;
  creative_set_conversions.push_back(creative_set_conversion_1);

  CreativeSetConversionInfo creative_set_conversion_2;  // Non matching
  creative_set_conversion_2.id = "1e945c25-98a2-443c-a7f5-e695110d2b84";
  creative_set_conversion_2.url_pattern = "https://www.qux.com/";
  creative_set_conversion_2.observation_window = base::Days(7);
  creative_set_conversion_2.expire_at =
      Now() + creative_set_conversion_2.observation_window;
  creative_set_conversions.push_back(creative_set_conversion_2);

  // Act
  FilterCreativeSetConversionsWithNonMatchingUrlPattern(
      creative_set_conversions,
      /*redirect_chain*/ {GURL("https://foo.com/bar")});

  // Assert
  const CreativeSetConversionList expected_creative_set_conversions = {
      creative_set_conversion_1};
  EXPECT_EQ(expected_creative_set_conversions, creative_set_conversions);
}

}  // namespace brave_ads
