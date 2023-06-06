/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

// Need to expect more items at that place, because Brave has additional item
// `More...` which redirects to brave://history/syncedTabs
#define BRAVE_MAX_TABS_PER_SESSION_AND_RECENCY \
  {ui::MenuModel::TYPE_COMMAND, true},

// The case when number of tabs on other device is <=4 so we do not add
// `More...` item is tested by RecentTabsSubMenuModelTest.MaxSessionsAndRecency

#include "src/chrome/browser/ui/toolbar/recent_tabs_sub_menu_model_unittest.cc"
