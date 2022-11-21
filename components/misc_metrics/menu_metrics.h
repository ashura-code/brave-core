/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_MISC_METRICS_MENU_METRICS_H_
#define BRAVE_COMPONENTS_MISC_METRICS_MENU_METRICS_H_

#include <utility>

#include "base/memory/raw_ptr.h"
#include "base/timer/timer.h"
#include "brave/components/time_period_storage/weekly_storage.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

class PrefRegistrySimple;
class PrefService;

namespace misc_metrics {

enum class MenuGroup {
  kTabWindow,
  kBraveFeatures,
  kBrowserViews,
};

extern const char kFrequentMenuGroupHistogramName[];
extern const char kMenuDismissRateHistogramName[];

class MenuMetrics {
 public:
  explicit MenuMetrics(PrefService* local_state);
  ~MenuMetrics();

  MenuMetrics(const MenuMetrics&) = delete;
  MenuMetrics& operator=(const MenuMetrics&) = delete;

  static void RegisterPrefs(PrefRegistrySimple* registry);

  // Records a usage of the relevant menu group, and records an answer
  // for the following P3A question:
  // What menu functionality group do you use the most?
  // 0. Tab & window actions (new tab/new window)
  // 1. Brave features (Wallet, Rewards, Sync)
  // 2. Browser views (History, Bookmarks, Extensions, Settings)
  void RecordMenuGroupAction(MenuGroup group);

  void RecordMenuShown();
  void RecordMenuDismiss();

 private:
  void RecordMenuDismissRate();

  void Update();

  absl::optional<std::pair<MenuGroup, int>> current_max_group_;

  raw_ptr<PrefService> local_state_ = nullptr;
  WeeklyStorage menu_shown_storage_;
  WeeklyStorage menu_dismiss_storage_;

  base::RepeatingTimer update_timer_;
};

}  // namespace misc_metrics

#endif  // BRAVE_COMPONENTS_MISC_METRICS_MENU_METRICS_H_
