/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/views/sidebar/sidebar_container_view.h"

#include <memory>
#include <utility>

#include "base/auto_reset.h"
#include "base/functional/bind.h"
#include "base/time/time.h"
#include "brave/browser/ui/brave_browser.h"
#include "brave/browser/ui/color/brave_color_id.h"
#include "brave/browser/ui/sidebar/sidebar_controller.h"
#include "brave/browser/ui/sidebar/sidebar_model.h"
#include "brave/browser/ui/sidebar/sidebar_service_factory.h"
#include "brave/browser/ui/sidebar/sidebar_utils.h"
#include "brave/browser/ui/views/frame/brave_browser_view.h"
#include "brave/browser/ui/views/side_panel/brave_side_panel.h"
#include "brave/browser/ui/views/sidebar/sidebar_control_view.h"
#include "brave/components/constants/pref_names.h"
#include "brave/components/sidebar/sidebar_item.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/browser/ui/color/chrome_color_id.h"
#include "chrome/browser/ui/exclusive_access/exclusive_access_manager.h"
#include "chrome/browser/ui/exclusive_access/fullscreen_controller.h"
#include "chrome/browser/ui/views/frame/browser_view.h"
#include "chrome/browser/ui/views/side_panel/side_panel_entry.h"
#include "chrome/browser/ui/views/toolbar/toolbar_view.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/web_contents.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/events/event_observer.h"
#include "ui/events/types/event_type.h"
#include "ui/gfx/geometry/point.h"
#include "ui/views/border.h"
#include "ui/views/event_monitor.h"
#include "ui/views/widget/widget.h"
#include "url/gurl.h"

namespace {

using ShowSidebarOption = sidebar::SidebarService::ShowSidebarOption;

sidebar::SidebarService* GetSidebarService(BraveBrowser* browser) {
  return sidebar::SidebarServiceFactory::GetForProfile(browser->profile());
}

}  // namespace

class SidebarContainerView::BrowserWindowEventObserver
    : public ui::EventObserver {
 public:
  explicit BrowserWindowEventObserver(SidebarContainerView& host)
      : host_(host) {}
  ~BrowserWindowEventObserver() override = default;
  BrowserWindowEventObserver(const BrowserWindowEventObserver&) = delete;
  BrowserWindowEventObserver& operator=(const BrowserWindowEventObserver&) =
      delete;

  void OnEvent(const ui::Event& event) override {
    DCHECK(event.IsMouseEvent());
    const auto* mouse_event = event.AsMouseEvent();

    gfx::Point window_event_position = mouse_event->location();
    // Convert window position to sidebar view's coordinate and check whether
    // it's included in sidebar ui or not.
    // If it's not included and sidebar could be hidden, stop monitoring and
    // hide UI.
    views::View::ConvertPointFromWidget(host_->sidebar_control_view_,
                                        &window_event_position);
    if (!host_->sidebar_control_view_->GetLocalBounds().Contains(
            window_event_position) &&
        !host_->ShouldForceShowSidebar()) {
      host_->StopBrowserWindowEventMonitoring();
      host_->HideSidebar(true);
    }
  }

 private:
  const raw_ref<SidebarContainerView> host_;
};

SidebarContainerView::SidebarContainerView(
    BraveBrowser* browser,
    SidePanelCoordinator* side_panel_coordinator,
    std::unique_ptr<BraveSidePanel> side_panel)
    : views::AnimationDelegateViews(this),
      browser_(browser),
      side_panel_coordinator_(side_panel_coordinator),
      browser_window_event_observer_(
          std::make_unique<BrowserWindowEventObserver>(*this)) {
  constexpr int kAnimationDurationMS = 150;
  show_animation_.SetSlideDuration(base::Milliseconds(kAnimationDurationMS));
  hide_animation_.SetSlideDuration(base::Milliseconds(kAnimationDurationMS));

  SetNotifyEnterExitOnChild(true);
  side_panel_ = AddChildView(std::move(side_panel));
}

SidebarContainerView::~SidebarContainerView() = default;

void SidebarContainerView::Init() {
  initialized_ = true;

  sidebar_model_ = browser_->sidebar_controller()->model();
  sidebar_model_observation_.Observe(sidebar_model_);

  auto* browser_view = BrowserView::GetBrowserViewForBrowser(browser_);
  DCHECK(browser_view);

  auto* side_panel_registry =
      SidePanelCoordinator::GetGlobalSidePanelRegistry(browser_);
  panel_registry_observation_.Observe(side_panel_registry);

  for (const auto& entry : side_panel_registry->entries()) {
    DVLOG(1) << "Observing panel entry in ctor: " << entry->name();
    panel_entry_observations_.AddObservation(entry.get());
  }

  show_side_panel_button_.Init(
      kShowSidePanelButton, browser_->profile()->GetPrefs(),
      base::BindRepeating(&SidebarContainerView::UpdateToolbarButtonVisibility,
                          base::Unretained(this)));

  AddChildViews();
  UpdateToolbarButtonVisibility();
  SetSidebarShowOption(GetSidebarService(browser_)->GetSidebarShowOption());
}

void SidebarContainerView::SetSidebarOnLeft(bool sidebar_on_left) {
  DCHECK(initialized_);

  if (sidebar_on_left_ == sidebar_on_left) {
    return;
  }

  sidebar_on_left_ = sidebar_on_left;

  DCHECK(sidebar_control_view_);
  sidebar_control_view_->SetSidebarOnLeft(sidebar_on_left_);

  DCHECK(side_panel_);
  side_panel_->SetHorizontalAlignment(
      sidebar_on_left ? BraveSidePanel::kHorizontalAlignLeft
                      : BraveSidePanel::kHorizontalAlignRight);

  GetEventDetectWidget()->SetSidebarOnLeft(sidebar_on_left_);
}

bool SidebarContainerView::IsSidebarVisible() const {
  return sidebar_control_view_ && sidebar_control_view_->GetVisible();
}

void SidebarContainerView::SetSidebarShowOption(ShowSidebarOption show_option) {
  DVLOG(2) << __func__;

  // Hide event detect widget when option is chaged from mouse over to others.
  if (show_sidebar_option_ == ShowSidebarOption::kShowOnMouseOver) {
    ShowOptionsEventDetectWidget(false);
  }

  show_sidebar_option_ = show_option;

  const bool is_panel_visible = side_panel_->GetVisible();
  if (show_sidebar_option_ == ShowSidebarOption::kShowAlways) {
    ShowSidebar(is_panel_visible);
    return;
  }

  if (show_sidebar_option_ == ShowSidebarOption::kShowNever) {
    if (!is_panel_visible) {
      HideSidebar(true);
    }
    return;
  }

  if (IsMouseHovered() || is_panel_visible) {
    ShowSidebar(is_panel_visible);
    return;
  }

  HideSidebar(true);
}

void SidebarContainerView::UpdateSidebarItemsState() {
  // control view has items.
  sidebar_control_view_->Update();
}

void SidebarContainerView::MenuClosed() {
  DVLOG(1) << __func__;

  // Don't need to to auto hide sidebar UI for other options.
  if (show_sidebar_option_ != ShowSidebarOption::kShowOnMouseOver) {
    return;
  }

  // Don't hide sidebar with below conditions.
  if (IsMouseHovered() || ShouldForceShowSidebar()) {
    return;
  }

  HideSidebar(true);
}

void SidebarContainerView::UpdateBackground() {
  if (const ui::ColorProvider* color_provider = GetColorProvider()) {
    // Fill background because panel's color uses alpha value.
    SetBackground(
        views::CreateSolidBackground(color_provider->GetColor(kColorToolbar)));
  }
}

void SidebarContainerView::AddChildViews() {
  // Insert to index 0 because |side_panel_| will already be at 0 but
  // we want the controls first.
  sidebar_control_view_ =
      AddChildViewAt(std::make_unique<SidebarControlView>(this, browser_), 0);

  // Hide by default. Visibility will be controlled by show options callback
  // later.
  sidebar_control_view_->SetVisible(false);
}

void SidebarContainerView::Layout() {
  if (!initialized_)
    return View::Layout();

  const int control_view_preferred_width =
      sidebar_control_view_->GetPreferredSize().width();

  int control_view_x = 0;
  int side_panel_x = control_view_x + control_view_preferred_width;
  if (!sidebar_on_left_) {
    control_view_x = width() - control_view_preferred_width;
    side_panel_x = 0;
  }

  sidebar_control_view_->SetBounds(control_view_x, 0,
                                   control_view_preferred_width, height());
  if (side_panel_->GetVisible()) {
    side_panel_->SetBounds(side_panel_x, 0,
                           width() - control_view_preferred_width, height());
  }
}

gfx::Size SidebarContainerView::CalculatePreferredSize() const {
  if (!initialized_ || !sidebar_control_view_->GetVisible() ||
      IsFullscreenByTab())
    return View::CalculatePreferredSize();

  if (show_animation_.is_animating()) {
    return {gfx::Tween::IntValueBetween(show_animation_.GetCurrentValue(),
                                        animation_start_width_,
                                        animation_end_width_),
            0};
  }

  if (hide_animation_.is_animating()) {
    return {gfx::Tween::IntValueBetween(hide_animation_.GetCurrentValue(),
                                        animation_start_width_,
                                        animation_end_width_),
            0};
  }

  int preferred_width = 0;
  if (sidebar_control_view_->GetVisible()) {
    preferred_width = sidebar_control_view_->GetPreferredSize().width();
  }

  if (side_panel_->GetVisible()) {
    preferred_width += side_panel_->GetPreferredSize().width();
  }

  return {preferred_width, 0};
}

void SidebarContainerView::OnThemeChanged() {
  View::OnThemeChanged();

  UpdateBackground();
}

bool SidebarContainerView::IsFullscreenByTab() const {
  DCHECK(browser_->exclusive_access_manager() &&
         browser_->exclusive_access_manager()->fullscreen_controller());
  return browser_->exclusive_access_manager()
      ->fullscreen_controller()
      ->IsWindowFullscreenForTabOrPending();
}

bool SidebarContainerView::ShouldForceShowSidebar() const {
  // It is more reliable to check whether coordinator has current entry rather
  // than checking if side_panel_ is visible.
  return side_panel_coordinator_->GetCurrentEntryId() ||
         sidebar_control_view_->IsItemReorderingInProgress() ||
         sidebar_control_view_->IsBubbleWidgetVisible();
}

void SidebarContainerView::OnMouseEntered(const ui::MouseEvent& event) {
  if (show_sidebar_option_ != ShowSidebarOption::kShowOnMouseOver) {
    return;
  }

  // Cancel hide schedule when mouse entered again quickly.
  sidebar_hide_timer_.Stop();
}

void SidebarContainerView::OnMouseExited(const ui::MouseEvent& event) {
  if (show_sidebar_option_ != ShowSidebarOption::kShowOnMouseOver) {
    return;
  }

  // When context menu is shown, this view can get this exited callback.
  // In that case, ignore this callback because mouse is still in this view.
  if (IsMouseHovered())
    return;

  if (ShouldForceShowSidebar()) {
    StartBrowserWindowEventMonitoring();
    return;
  }

  // Give some delay for hiding to prevent flickering by open/hide quickly.
  // when mouse is moved around the sidebar.
  constexpr int kHideDelayInMS = 400;
  sidebar_hide_timer_.Start(FROM_HERE, base::Milliseconds(kHideDelayInMS),
                            base::BindOnce(&SidebarContainerView::HideSidebar,
                                           base::Unretained(this), true));
}

void SidebarContainerView::AnimationProgressed(
    const gfx::Animation* animation) {
  PreferredSizeChanged();
}

void SidebarContainerView::AnimationEnded(const gfx::Animation* animation) {
  PreferredSizeChanged();

  // Handle children's visibility after hiding animation completion.
  if (animation == &show_animation_) {
    ShowOptionsEventDetectWidget(false);
  }

  if (animation == &hide_animation_) {
    if (animation_end_width_ == 0) {
      ShowOptionsEventDetectWidget(true);
      sidebar_control_view_->SetVisible(false);
      side_panel_->SetVisible(false);
    } else {
      sidebar_control_view_->SetVisible(true);
      side_panel_->SetVisible(false);
    }
  }

  animation_start_width_ = animation_end_width_ = 0;
}

void SidebarContainerView::OnActiveIndexChanged(
    absl::optional<size_t> old_index,
    absl::optional<size_t> new_index) {
  DVLOG(1) << "OnActiveIndexChanged: "
           << (old_index ? std::to_string(*old_index) : "none") << " to "
           << (new_index ? std::to_string(*new_index) : "none");
  if (new_index) {
    ShowSidebar(true);
  } else {
    // Special handling for side panel extension.
    // If sidebar extension is active, active index is none now.
    // Don't hide sidebar to make sidebar extension visible.
    if (side_panel_coordinator_->GetCurrentEntryId() ==
        SidePanelEntryId::kExtension) {
      return;
    }
    HideSidebarForShowOption();
  }
}

void SidebarContainerView::OnItemAdded(const sidebar::SidebarItem& item,
                                       size_t index,
                                       bool user_gesture) {
  UpdateToolbarButtonVisibility();
}

void SidebarContainerView::OnItemRemoved(size_t index) {
  UpdateToolbarButtonVisibility();
}

SidebarShowOptionsEventDetectWidget*
SidebarContainerView::GetEventDetectWidget() {
  if (!show_options_widget_) {
    show_options_widget_ =
        std::make_unique<SidebarShowOptionsEventDetectWidget>(
            *static_cast<BraveBrowserView*>(
                BrowserView::GetBrowserViewForBrowser(browser_)),
            *this);
    show_options_widget_->Hide();
  }

  return show_options_widget_.get();
}

void SidebarContainerView::ShowOptionsEventDetectWidget(bool show) {
  if (show_sidebar_option_ != ShowSidebarOption::kShowOnMouseOver) {
    return;
  }

  show ? GetEventDetectWidget()->Show() : GetEventDetectWidget()->Hide();
}

void SidebarContainerView::ShowSidebarControlView() {
  DVLOG(1) << __func__;
  ShowSidebar(false);
}

void SidebarContainerView::ShowSidebar(bool show_side_panel) {
  DVLOG(1) << __func__ << ": show panel: " << show_side_panel;

  if (show_animation_.is_animating() || hide_animation_.is_animating()) {
    DVLOG(1) << __func__ << ": Finish previous show/hide animation.";
    show_animation_.End();
    hide_animation_.End();
  }

  show_animation_.Reset();

  // Calculate the start & end width for animation. Both are used when
  // calculating preferred width during the show animation.
  animation_start_width_ = 0;
  animation_end_width_ = 0;

  // Don't need event detect widget when sidebar gets visible.
  ShowOptionsEventDetectWidget(false);

  if (sidebar_control_view_->GetVisible()) {
    animation_start_width_ = sidebar_control_view_->GetPreferredSize().width();
  }

  if (side_panel_->GetVisible()) {
    animation_start_width_ += side_panel_->GetPreferredSize().width();
  }

  animation_end_width_ = sidebar_control_view_->GetPreferredSize().width();
  if (show_side_panel) {
    animation_end_width_ += side_panel_->GetPreferredSize().width();
  }

  if (animation_start_width_ == animation_end_width_) {
    DVLOG(1) << __func__ << ": don't need show animation.";
    return;
  }

  DVLOG(1) << __func__ << ": show animation (start, end) width: ("
           << animation_start_width_ << ", " << animation_end_width_ << ")";

  sidebar_control_view_->SetVisible(true);
  if (show_side_panel) {
    side_panel_->SetVisible(true);
  }

  // Animation will trigger layout by changing preferred size.
  if (ShouldUseAnimation()) {
    DVLOG(1) << __func__ << ": show with animation";
    show_animation_.Show();
    return;
  }

  DVLOG(1) << __func__ << ": show w/o animation";
  // Otherwise, layout should be requested here.
  InvalidateLayout();
}

void SidebarContainerView::HideSidebar(bool hide_sidebar_control) {
  DVLOG(1) << __func__ << ": hide control: " << hide_sidebar_control;

  if (show_animation_.is_animating() || hide_animation_.is_animating()) {
    DVLOG(1) << __func__ << ": Finish previous show/hide animation.";
    show_animation_.End();
    hide_animation_.End();
  }

  hide_animation_.Reset();

  // Calculate the start & end width for animation. Both are used when
  // calculating preferred width during the hide animation.
  animation_start_width_ = 0;
  animation_end_width_ = 0;

  if (sidebar_control_view_->GetVisible()) {
    animation_start_width_ = sidebar_control_view_->GetPreferredSize().width();
  }

  if (side_panel_->GetVisible()) {
    animation_start_width_ += side_panel_->GetPreferredSize().width();
  }

  if (!hide_sidebar_control) {
    animation_end_width_ = sidebar_control_view_->GetPreferredSize().width();
  }

  if (animation_start_width_ == animation_end_width_) {
    DVLOG(1) << __func__ << ": don't need hide animation.";

    // At startup, make event detect widget visible even if children's
    // visibility state is not changed.
    if (animation_end_width_ == 0) {
      ShowOptionsEventDetectWidget(true);
    }

    return;
  }

  DVLOG(1) << __func__ << ": hide animation (start, end) width: ("
           << animation_start_width_ << ", " << animation_end_width_ << ")";

  GetFocusManager()->ClearFocus();

  if (ShouldUseAnimation()) {
    DVLOG(1) << __func__ << ": hide with animation";
    hide_animation_.Show();
    return;
  }

  DVLOG(1) << __func__ << ": hide w/o animation";
  if (animation_end_width_ == 0) {
    ShowOptionsEventDetectWidget(true);
  }

  sidebar_control_view_->SetVisible(!hide_sidebar_control);
  side_panel_->SetVisible(false);
  InvalidateLayout();
}

void SidebarContainerView::HideSidebarForShowOption() {
  if (show_sidebar_option_ == ShowSidebarOption::kShowAlways) {
    HideSidebar(false);
  }

  if (show_sidebar_option_ == ShowSidebarOption::kShowOnMouseOver) {
    // Hide all if mouse is outside of control view.
    HideSidebar(!sidebar_control_view_->IsMouseHovered());
  }

  if (show_sidebar_option_ == ShowSidebarOption::kShowNever) {
    HideSidebar(true);
  }
}

bool SidebarContainerView::ShouldUseAnimation() const {
  auto* controller = browser_->sidebar_controller();
  return !controller->IsPanelOperationFromActiveTabChange() &&
         gfx::Animation::ShouldRenderRichAnimation();
}

void SidebarContainerView::UpdateToolbarButtonVisibility() {
  // Coordinate sidebar toolbar button visibility based on
  // whether there are any sibar items with a sidepanel.
  // This is similar to how chromium's side_panel_coordinator View
  // also has some control on the toolbar button.
  auto has_panel_item =
      GetSidebarService(browser_)->GetDefaultPanelItem().has_value();
  auto* browser_view = BrowserView::GetBrowserViewForBrowser(browser_);
  browser_view->toolbar()->GetSidePanelButton()->SetVisible(
      has_panel_item && show_side_panel_button_.GetValue());
}

void SidebarContainerView::StartBrowserWindowEventMonitoring() {
  if (browser_window_event_monitor_)
    return;

  DVLOG(1) << __func__;
  browser_window_event_monitor_ = views::EventMonitor::CreateWindowMonitor(
      browser_window_event_observer_.get(), GetWidget()->GetNativeWindow(),
      {ui::ET_MOUSE_MOVED});
}

void SidebarContainerView::StopBrowserWindowEventMonitoring() {
  DVLOG(1) << __func__;
  browser_window_event_monitor_.reset();
}

void SidebarContainerView::OnEntryShown(SidePanelEntry* entry) {
  // Make sure item is selected. We need to observe the SidePanel system
  // as well as Sidebar as there are other ways than Sidebar for SidePanel
  // items to be shown and hidden, e.g. toolbar button.
  DVLOG(1) << "Panel shown: " << entry->name();
  auto* controller = browser_->sidebar_controller();

  // Handling side panel extension till we have item for it.
  if (entry->key().extension_id()) {
    // If side panel is shown by side panel extension, showing should
    // be done here because side panel extension is not controlled by our
    // sidebar model.
    controller->SetBrowserActivePanelKey(entry->key());
    ShowSidebar(true);
    return;
  }

  for (const auto& item : sidebar_model_->GetAllSidebarItems()) {
    if (!item.open_in_panel) {
      continue;
    }
    if (entry->key().id() == sidebar::SidePanelIdFromSideBarItem(item)) {
      auto side_bar_index = sidebar_model_->GetIndexOf(item);
      controller->ActivateItemAt(side_bar_index);
      break;
    }
  }
}

void SidebarContainerView::OnEntryHidden(SidePanelEntry* entry) {
  // Make sure item is deselected
  DVLOG(1) << "Panel hidden: " << entry->name();
  auto* controller = browser_->sidebar_controller();

  if (entry->key().extension_id()) {
    if (!side_panel_coordinator_->GetCurrentEntryId()) {
      // If side panel is closed by togging side panel extension, hiding should
      // be done here because side panel extension is not controlled by our
      // sidebar model.
      controller->SetBrowserActivePanelKey();
      HideSidebarForShowOption();
    }
    return;
  }

  for (const auto& item : sidebar_model_->GetAllSidebarItems()) {
    if (!item.open_in_panel) {
      continue;
    }

    if (entry->key().id() == sidebar::SidePanelIdFromSideBarItem(item)) {
      auto side_bar_index = sidebar_model_->GetIndexOf(item);
      if (controller->IsActiveIndex(side_bar_index)) {
        controller->ActivateItemAt(absl::nullopt);
      }
      break;
    }
  }
}

void SidebarContainerView::OnEntryRegistered(SidePanelRegistry* registry,
                                             SidePanelEntry* entry) {
  // Observe when it's shown or hidden
  DVLOG(1) << "Observing panel entry in registry observer: " << entry->name();
  panel_entry_observations_.AddObservation(entry);
}

void SidebarContainerView::OnEntryWillDeregister(SidePanelRegistry* registry,
                                                 SidePanelEntry* entry) {
  // Stop observing
  DVLOG(1) << "Unobserving panel entry in registry observer: " << entry->name();
  panel_entry_observations_.RemoveObservation(entry);
}

BEGIN_METADATA(SidebarContainerView, views::View)
END_METADATA
