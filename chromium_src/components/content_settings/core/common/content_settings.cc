/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "components/content_settings/core/common/content_settings.h"

// Leave a gap between Chromium values and our values in the kHistogramValue
// array so that we don't have to renumber when new content settings types are
// added upstream.
namespace {

// Do not change the value arbitrarily. This is used to validate we have a gap
// between Chromium's and Brave's histograms. This value must be less than 1000
// as upstream performs a sanity check that the total number of buckets isn't
// unreasonably large.
constexpr int kBraveValuesStart = 900;

constexpr int brave_value(int incr) {
  return kBraveValuesStart + incr;
}

}  // namespace

static_assert(static_cast<int>(ContentSettingsType::NUM_TYPES) <
                  kBraveValuesStart,
              "There must a gap between the histograms used by Chromium, and "
              "the ones used by Brave.");

// clang-format off
#define BRAVE_HISTOGRAM_VALUE_LIST                                        \
  {ContentSettingsType::BRAVE_ADS, brave_value(0)},                       \
  {ContentSettingsType::BRAVE_COSMETIC_FILTERING, brave_value(1)},        \
  {ContentSettingsType::BRAVE_TRACKERS, brave_value(2)},                  \
  {ContentSettingsType::BRAVE_HTTP_UPGRADABLE_RESOURCES, brave_value(3)}, \
  {ContentSettingsType::BRAVE_FINGERPRINTING_V2, brave_value(4)},         \
  {ContentSettingsType::BRAVE_SHIELDS, brave_value(5)},                   \
  {ContentSettingsType::BRAVE_REFERRERS, brave_value(6)},                 \
  {ContentSettingsType::BRAVE_COOKIES, brave_value(7)},                   \
  {ContentSettingsType::BRAVE_SPEEDREADER, brave_value(8)},               \
  {ContentSettingsType::BRAVE_ETHEREUM, brave_value(9)},                  \
  {ContentSettingsType::BRAVE_SOLANA, brave_value(10)},                   \
  {ContentSettingsType::BRAVE_GOOGLE_SIGN_IN, brave_value(11)},           \
  {ContentSettingsType::BRAVE_HTTPS_UPGRADE, brave_value(12)},            \
  {ContentSettingsType::BRAVE_REMEMBER_1P_STORAGE, brave_value(13)},      \
  {ContentSettingsType::BRAVE_LOCALHOST_ACCESS, brave_value(14)}
// clang-format on

#define RendererContentSettingRules RendererContentSettingRules_ChromiumImpl

#include "src/components/content_settings/core/common/content_settings.cc"

#undef RendererContentSettingRules
#undef BRAVE_HISTOGRAM_VALUE_LIST

RendererContentSettingRules::RendererContentSettingRules() = default;
RendererContentSettingRules::~RendererContentSettingRules() = default;

RendererContentSettingRules::RendererContentSettingRules(
    const RendererContentSettingRules&) = default;

RendererContentSettingRules::RendererContentSettingRules(
    RendererContentSettingRules&& rules) = default;

RendererContentSettingRules& RendererContentSettingRules::operator=(
    const RendererContentSettingRules& rules) = default;

RendererContentSettingRules& RendererContentSettingRules::operator=(
    RendererContentSettingRules&& rules) = default;

// static
bool RendererContentSettingRules::IsRendererContentSetting(
    ContentSettingsType content_type) {
  return RendererContentSettingRules_ChromiumImpl::IsRendererContentSetting(
             content_type) ||
         content_type == ContentSettingsType::AUTOPLAY ||
         content_type == ContentSettingsType::BRAVE_COSMETIC_FILTERING ||
         content_type == ContentSettingsType::BRAVE_FINGERPRINTING_V2 ||
         content_type == ContentSettingsType::BRAVE_GOOGLE_SIGN_IN ||
         content_type == ContentSettingsType::BRAVE_LOCALHOST_ACCESS ||
         content_type == ContentSettingsType::BRAVE_SHIELDS;
}

void RendererContentSettingRules::FilterRulesByOutermostMainFrameURL(
    const GURL& outermost_main_frame_url) {
  RendererContentSettingRules_ChromiumImpl::FilterRulesByOutermostMainFrameURL(
      outermost_main_frame_url);
  FilterRulesForType(autoplay_rules, outermost_main_frame_url);
  FilterRulesForType(brave_shields_rules, outermost_main_frame_url);
  // FilterRulesForType has a DCHECK on the size and these fail (for now)
  // because they incorrectly use CONTENT_SETTINGS_DEFAULT as a distinct setting
  base::EraseIf(
      cosmetic_filtering_rules,
      [&outermost_main_frame_url](const ContentSettingPatternSource& source) {
        return !source.primary_pattern.Matches(outermost_main_frame_url);
      });
  base::EraseIf(
      fingerprinting_rules,
      [&outermost_main_frame_url](const ContentSettingPatternSource& source) {
        return !source.primary_pattern.Matches(outermost_main_frame_url);
      });
}

namespace content_settings {
namespace {

bool IsExplicitSetting(const ContentSettingsPattern& primary_pattern,
                       const ContentSettingsPattern& secondary_pattern) {
  return !primary_pattern.MatchesAllHosts() ||
         !secondary_pattern.MatchesAllHosts();
}

}  // namespace

bool IsExplicitSetting(const ContentSettingPatternSource& setting) {
  return IsExplicitSetting(setting.primary_pattern, setting.secondary_pattern);
}

bool IsExplicitSetting(const SettingInfo& setting) {
  return IsExplicitSetting(setting.primary_pattern, setting.secondary_pattern);
}

}  // namespace content_settings
