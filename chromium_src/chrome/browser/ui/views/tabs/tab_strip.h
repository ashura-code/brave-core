/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_CHROME_BROWSER_UI_VIEWS_TABS_TAB_STRIP_H_
#define BRAVE_CHROMIUM_SRC_CHROME_BROWSER_UI_VIEWS_TABS_TAB_STRIP_H_

#include "chrome/browser/ui/views/tabs/tab_slot_controller.h"

#define UpdateHoverCard           \
  UpdateHoverCard_Unused();       \
  friend class BraveTabHoverTest; \
  friend class BraveTabStrip;     \
  friend class BraveTabDragContext;           \
  void UpdateHoverCard

#define ShouldDrawStrokes     \
  UnUsed() { return true; }   \
  virtual bool ShouldDrawStrokes

#define GetDragContext                        \
  Unused_GetDragContext() { return nullptr; } \
  virtual TabDragContext* GetDragContext
#include "src/chrome/browser/ui/views/tabs/tab_strip.h"

#undef ShouldDrawStrokes
#undef UpdateHoverCard
#undef GetDragContext

#endif  // BRAVE_CHROMIUM_SRC_CHROME_BROWSER_UI_VIEWS_TABS_TAB_STRIP_H_
