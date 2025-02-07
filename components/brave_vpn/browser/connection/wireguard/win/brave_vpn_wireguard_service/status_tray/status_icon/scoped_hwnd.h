/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_VPN_BROWSER_CONNECTION_WIREGUARD_WIN_BRAVE_VPN_WIREGUARD_SERVICE_STATUS_TRAY_STATUS_ICON_SCOPED_HWND_H_
#define BRAVE_COMPONENTS_BRAVE_VPN_BROWSER_CONNECTION_WIREGUARD_WIN_BRAVE_VPN_WIREGUARD_SERVICE_STATUS_TRAY_STATUS_ICON_SCOPED_HWND_H_

#include "base/win/windows_types.h"

#include "base/scoped_generic.h"

namespace brave_vpn {

namespace win {

namespace internal {

template <class T>
struct ScopedHWNDObjectTraits {
  static T InvalidValue() { return nullptr; }
  static void Free(T object) { DestroyWindow(object); }
};

}  // namespace internal

template <class T>
using ScopedHWNDObject =
    base::ScopedGeneric<T, internal::ScopedHWNDObjectTraits<T>>;

typedef ScopedHWNDObject<HWND> ScopedHWND;

}  // namespace win
}  // namespace brave_vpn

#endif  // BRAVE_COMPONENTS_BRAVE_VPN_BROWSER_CONNECTION_WIREGUARD_WIN_BRAVE_VPN_WIREGUARD_SERVICE_STATUS_TRAY_STATUS_ICON_SCOPED_HWND_H_
