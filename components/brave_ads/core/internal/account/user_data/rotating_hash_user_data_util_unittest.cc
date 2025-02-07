/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/account/user_data/rotating_hash_user_data_util.h"

#include "brave/components/brave_ads/core/internal/account/transactions/transaction_info.h"
#include "brave/components/brave_ads/core/internal/ads/ad_unittest_constants.h"
#include "brave/components/brave_ads/core/internal/common/unittest/unittest_base.h"
#include "brave/components/brave_ads/core/internal/common/unittest/unittest_time_util.h"
#include "brave/components/brave_ads/core/internal/global_state/global_state.h"

// npm run test -- brave_unit_tests --filter=BraveAds*

namespace brave_ads {

class BraveAdsRotatingHashUserDataUtilTest : public UnitTestBase {};

TEST_F(BraveAdsRotatingHashUserDataUtilTest,
       DoNotBuildRotatingHashIfMissingDeviceId) {
  // Arrange
  AdvanceClockTo(TimeFromString("2 June 2022 11:00", /*is_local*/ false));

  TransactionInfo transaction;
  transaction.creative_instance_id = kCreativeInstanceId;

  // Act
  const absl::optional<std::string> rotating_hash =
      BuildRotatingHash(transaction);

  // Assert
  EXPECT_FALSE(rotating_hash);
}

TEST_F(BraveAdsRotatingHashUserDataUtilTest, BuildRotatingHash) {
  // Arrange
  auto& sys_info = GlobalState::GetInstance()->SysInfo();
  sys_info.device_id =
      "21b4677de1a9b4a197ab671a1481d3fcb24f826a4358a05aafbaee5a9a51b57e";

  AdvanceClockTo(TimeFromString("2 June 2022 11:00", /*is_local*/ false));

  TransactionInfo transaction;
  transaction.creative_instance_id = kCreativeInstanceId;

  // Act
  const absl::optional<std::string> rotating_hash =
      BuildRotatingHash(transaction);
  EXPECT_TRUE(rotating_hash);

  // Assert
  EXPECT_EQ("j9D7eKSoPLYNfxkG2Mx+SbgKJ9hcKg1QwDB8B5qxlpk=", rotating_hash);
}

TEST_F(BraveAdsRotatingHashUserDataUtilTest,
       BuildRotatingHashIfWithinSameHour) {
  // Arrange
  auto& sys_info = GlobalState::GetInstance()->SysInfo();
  sys_info.device_id =
      "21b4677de1a9b4a197ab671a1481d3fcb24f826a4358a05aafbaee5a9a51b57e";

  TransactionInfo transaction;
  transaction.creative_instance_id = kCreativeInstanceId;

  AdvanceClockTo(TimeFromString("2 June 2022 11:00", /*is_local*/ false));

  const absl::optional<std::string> rotating_hash_before =
      BuildRotatingHash(transaction);
  EXPECT_TRUE(rotating_hash_before);

  // Act
  AdvanceClockBy(base::Hours(1) - base::Milliseconds(1));

  const absl::optional<std::string> rotating_hash_after =
      BuildRotatingHash(transaction);
  EXPECT_TRUE(rotating_hash_after);

  // Assert
  EXPECT_EQ(rotating_hash_before, rotating_hash_after);
}

TEST_F(BraveAdsRotatingHashUserDataUtilTest,
       BuildRotatingHashForDifferentHours) {
  // Arrange
  auto& sys_info = GlobalState::GetInstance()->SysInfo();
  sys_info.device_id =
      "21b4677de1a9b4a197ab671a1481d3fcb24f826a4358a05aafbaee5a9a51b57e";

  TransactionInfo transaction;
  transaction.creative_instance_id = kCreativeInstanceId;

  AdvanceClockTo(TimeFromString("2 June 2022 11:00", /*is_local*/ false));

  const absl::optional<std::string> rotating_hash_before =
      BuildRotatingHash(transaction);
  EXPECT_TRUE(rotating_hash_before);

  // Act
  AdvanceClockBy(base::Hours(1));

  const absl::optional<std::string> rotating_hash_after =
      BuildRotatingHash(transaction);
  EXPECT_TRUE(rotating_hash_after);

  // Assert
  EXPECT_NE(rotating_hash_before, rotating_hash_after);
}

TEST_F(BraveAdsRotatingHashUserDataUtilTest,
       BuildRotatingHashForSameHourButDifferentDay) {
  // Arrange
  auto& sys_info = GlobalState::GetInstance()->SysInfo();
  sys_info.device_id =
      "21b4677de1a9b4a197ab671a1481d3fcb24f826a4358a05aafbaee5a9a51b57e";

  TransactionInfo transaction;
  transaction.creative_instance_id = kCreativeInstanceId;

  const absl::optional<std::string> rotating_hash_before =
      BuildRotatingHash(transaction);
  EXPECT_TRUE(rotating_hash_before);

  // Act
  AdvanceClockBy(base::Days(1));

  const absl::optional<std::string> rotating_hash_after =
      BuildRotatingHash(transaction);
  EXPECT_TRUE(rotating_hash_after);

  // Assert
  EXPECT_NE(rotating_hash_before, rotating_hash_after);
}

}  // namespace brave_ads
