/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/creatives/conversions/creative_set_conversion_database_table.h"

#include "brave/components/brave_ads/core/internal/ads/ad_unittest_constants.h"
#include "brave/components/brave_ads/core/internal/common/unittest/unittest_base.h"
#include "brave/components/brave_ads/core/internal/common/unittest/unittest_container_util.h"
#include "brave/components/brave_ads/core/internal/common/unittest/unittest_time_util.h"
#include "brave/components/brave_ads/core/internal/conversions/types/verifiable_conversion/verifiable_conversion_unittest_constants.h"
#include "brave/components/brave_ads/core/internal/creatives/conversions/creative_set_conversion_database_table_util.h"

// npm run test -- brave_unit_tests --filter=BraveAds*

namespace brave_ads {

class BraveAdsCreativeSetConversionDatabaseTableTest : public UnitTestBase {
 protected:
  database::table::CreativeSetConversions database_table_;
};

TEST_F(BraveAdsCreativeSetConversionDatabaseTableTest, EmptySave) {
  // Arrange

  // Act
  database::SaveCreativeSetConversions({});

  // Assert
  database_table_.GetAll(base::BindOnce(
      [](const bool success,
         const CreativeSetConversionList& creative_set_conversions) {
        ASSERT_TRUE(success);
        EXPECT_TRUE(creative_set_conversions.empty());
      }));
}

TEST_F(BraveAdsCreativeSetConversionDatabaseTableTest,
       SaveCreativeSetConversions) {
  // Arrange
  CreativeSetConversionList creative_set_conversions;

  CreativeSetConversionInfo creative_set_conversion_1;
  creative_set_conversion_1.id = kCreativeSetId;
  creative_set_conversion_1.url_pattern = "https://www.brave.com/*";
  creative_set_conversion_1.extract_verifiable_id = true;
  creative_set_conversion_1.verifiable_advertiser_public_key_base64 =
      kVerifiableConversionAdvertiserPublicKey;
  creative_set_conversion_1.observation_window = base::Days(3);
  creative_set_conversion_1.expire_at =
      Now() + creative_set_conversion_1.observation_window;
  creative_set_conversions.push_back(creative_set_conversion_1);

  CreativeSetConversionInfo creative_set_conversion_2;
  creative_set_conversion_2.id = "4e83a23c-1194-40f8-8fdc-2f38d7ed75c8";
  creative_set_conversion_2.url_pattern = "https://www.brave.com/signup/*";
  creative_set_conversion_2.observation_window = base::Days(30);
  creative_set_conversion_2.expire_at =
      Now() + creative_set_conversion_2.observation_window;
  creative_set_conversions.push_back(creative_set_conversion_2);

  // Act
  database::SaveCreativeSetConversions(creative_set_conversions);

  // Assert
  database_table_.GetAll(base::BindOnce(
      [](const CreativeSetConversionList& expected_creative_set_conversions,
         const bool success,
         const CreativeSetConversionList& creative_set_conversions) {
        ASSERT_TRUE(success);
        EXPECT_TRUE(ContainersEq(expected_creative_set_conversions,
                                 creative_set_conversions));
      },
      creative_set_conversions));
}

TEST_F(BraveAdsCreativeSetConversionDatabaseTableTest,
       DoNotSaveDuplicateConversion) {
  // Arrange
  CreativeSetConversionList creative_set_conversions;
  CreativeSetConversionInfo creative_set_conversion;
  creative_set_conversion.id = kCreativeSetId;
  creative_set_conversion.url_pattern = "https://www.brave.com/*";
  creative_set_conversion.extract_verifiable_id = true;
  creative_set_conversion.verifiable_advertiser_public_key_base64 =
      kVerifiableConversionAdvertiserPublicKey;
  creative_set_conversion.observation_window = base::Days(3);
  creative_set_conversion.expire_at =
      Now() + creative_set_conversion.observation_window;
  creative_set_conversions.push_back(creative_set_conversion);

  database::SaveCreativeSetConversions(creative_set_conversions);

  // Act
  database::SaveCreativeSetConversions(creative_set_conversions);

  // Assert
  database_table_.GetAll(base::BindOnce(
      [](const CreativeSetConversionList& expected_creative_set_conversions,
         const bool success,
         const CreativeSetConversionList& creative_set_conversions) {
        ASSERT_TRUE(success);
        EXPECT_EQ(expected_creative_set_conversions, creative_set_conversions);
      },
      creative_set_conversions));
}

TEST_F(BraveAdsCreativeSetConversionDatabaseTableTest,
       PurgeExpiredConversions) {
  // Arrange
  CreativeSetConversionList creative_set_conversions;

  CreativeSetConversionInfo creative_set_conversion_1;
  creative_set_conversion_1.id = kCreativeSetId;
  creative_set_conversion_1.url_pattern = "https://www.brave.com/*";
  creative_set_conversion_1.observation_window = base::Days(7);
  creative_set_conversion_1.expire_at =
      Now() + creative_set_conversion_1.observation_window;
  creative_set_conversions.push_back(creative_set_conversion_1);

  CreativeSetConversionInfo creative_set_conversion_2;  // Should be purged
  creative_set_conversion_2.id = "4e83a23c-1194-40f8-8fdc-2f38d7ed75c8";
  creative_set_conversion_2.url_pattern = "https://www.brave.com/signup/*";
  creative_set_conversion_2.observation_window = base::Days(3);
  creative_set_conversion_2.expire_at =
      Now() + creative_set_conversion_2.observation_window;
  creative_set_conversions.push_back(creative_set_conversion_2);

  database::SaveCreativeSetConversions(creative_set_conversions);

  // Act
  AdvanceClockBy(base::Days(4));

  database::PurgeExpiredCreativeSetConversions();

  // Assert
  const CreativeSetConversionList expected_creative_set_conversions = {
      creative_set_conversion_1};

  database_table_.GetAll(base::BindOnce(
      [](const CreativeSetConversionList& expected_creative_set_conversions,
         const bool success,
         const CreativeSetConversionList& creative_set_conversions) {
        ASSERT_TRUE(success);
        EXPECT_TRUE(ContainersEq(expected_creative_set_conversions,
                                 creative_set_conversions));
      },
      expected_creative_set_conversions));
}

TEST_F(BraveAdsCreativeSetConversionDatabaseTableTest,
       SaveConversionWithMatchingCreativeSetIdAndType) {
  // Arrange
  CreativeSetConversionList creative_set_conversions;

  CreativeSetConversionInfo creative_set_conversion_1;
  creative_set_conversion_1.id = kCreativeSetId;
  creative_set_conversion_1.url_pattern = "https://www.brave.com/1";
  creative_set_conversion_1.observation_window = base::Days(3);
  creative_set_conversion_1.expire_at =
      Now() + creative_set_conversion_1.observation_window;
  creative_set_conversions.push_back(creative_set_conversion_1);

  database::SaveCreativeSetConversions(creative_set_conversions);

  // Act
  CreativeSetConversionInfo
      creative_set_conversion_2;  // Should supersede creative_set_conversion_1
  creative_set_conversion_2.id = kCreativeSetId;
  creative_set_conversion_2.url_pattern = "https://www.brave.com/2";
  creative_set_conversion_2.extract_verifiable_id = true;
  creative_set_conversion_2.verifiable_advertiser_public_key_base64 =
      kVerifiableConversionAdvertiserPublicKey;
  creative_set_conversion_2.observation_window = base::Days(30);
  creative_set_conversion_2.expire_at =
      Now() + creative_set_conversion_2.observation_window;
  creative_set_conversions.push_back(creative_set_conversion_2);

  database::SaveCreativeSetConversions(creative_set_conversions);

  // Assert
  const CreativeSetConversionList expected_creative_set_conversions = {
      creative_set_conversion_2};

  database_table_.GetAll(base::BindOnce(
      [](const CreativeSetConversionList& expected_creative_set_conversions,
         const bool success,
         const CreativeSetConversionList& creative_set_conversions) {
        ASSERT_TRUE(success);
        EXPECT_EQ(expected_creative_set_conversions, creative_set_conversions);
      },
      expected_creative_set_conversions));
}

TEST_F(BraveAdsCreativeSetConversionDatabaseTableTest, TableName) {
  // Arrange

  // Act

  // Assert
  EXPECT_EQ("creative_set_conversions", database_table_.GetTableName());
}

}  // namespace brave_ads
