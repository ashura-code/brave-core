# Since recent changes upstream, Chromium is now using IDs up to ~46310
# (see end of //out/Component/gen/tools/gritsettings/default_resource_ids),
# so let's leave some padding after that and start assigning them on 47500.
{
  "SRCDIR": "../..",
  "brave/common/extensions/api/brave_api_resources.grd": {
    "includes": [50500],
  },
  "brave/components/resources/brave_components_resources.grd": {
    "includes": [50550],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-brave_adblock/brave_adblock.grd": {
    "META": {"sizes": {"includes": [50]}},
    "includes": [50600],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-brave_new_tab/brave_new_tab.grd": {
    "META": {"sizes": {"includes": [350]}},
    "includes": [50650],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-brave_welcome/brave_welcome.grd": {
    "META": {"sizes": {"includes": [200]}},
    "includes": [52000],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/browser/resources/settings/brave_settings_resources.grd": {
    "META": {"sizes": {"includes": [400]}},
    "includes": [52200],
  },
  "brave/app/brave_generated_resources.grd": {
    "includes": [52600],
    "messages": [52100],
  },
  "brave/app/theme/brave_theme_resources.grd": {
    "structures": [53600],
  },
  "brave/app/theme/brave_unscaled_resources.grd": {
    "includes": [54100],
  },
  "brave/components/brave_sync/resources.grd": {
    "includes": [54600],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-brave_sync/brave_sync.grd": {
    "META": {"sizes": {"includes": [500]}},
    "includes": [54700],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-brave_webtorrent/brave_webtorrent.grd": {
    "META": {"sizes": {"includes": [500]}},
    "includes": [55200],
  },
  "brave/components/brave_rewards/resources/extension/extension_static_resources.grd": {
    "includes": [55700],
  },
  "brave/components/brave_webtorrent/resources.grd": {
    "includes": [56300],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-brave_rewards_extension_panel/brave_rewards_extension_panel.grd": {
    "META": {"sizes": {"includes": [100]}},
    "includes": [56400],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-brave_rewards_panel/brave_rewards_panel.grd": {
    "META": {"sizes": {"includes": [100]}},
    "includes": [56500],
  },
  "brave/components/brave_rewards/resources/brave_rewards_static_resources.grd": {
    "includes": [56600],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-brave_rewards_page/brave_rewards_page.grd": {
    "META": {"sizes": {"includes": [500]}},
    "includes": [56800],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-brave_rewards_internals/brave_rewards_internals.grd": {
    "META": {"sizes": {"includes": [300]}},
    "includes": [57300],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-brave_rewards_tip/brave_rewards_tip.grd": {
    "META": {"sizes": {"includes": [200]}},
    "includes": [57600],
  },
  "brave/components/resources/brave_components_strings.grd": {
    "messages": [57800],
  },
  "brave/components/brave_ads/resources/bat_ads_resources.grd": {
    "includes": [57900]
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-brave_wallet_page/brave_wallet_page.grd": {
    "META": {"sizes": {"includes": [200]}},
    "includes": [58800],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-ethereum_remote_client_page/ethereum_remote_client_page.grd": {
    "META": {"sizes": {"includes": [100]}},
    "includes": [58900],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-brave_wallet_panel/brave_wallet_panel.grd": {
    "META": {"sizes": {"includes": [200]}},
    "includes": [59000],
  },
  "brave/components/brave_extension/extension/resources.grd": {
    "includes": [59200],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-brave_extension/brave_extension.grd": {
    "META": {"sizes": {"includes": [500]}},
    "includes": [59700],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-webcompat_reporter/webcompat_reporter.grd": {
    "META": {"sizes": {"includes": [500]}},
    "includes": [60200],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-ipfs/ipfs.grd": {
    "META": {"sizes": {"includes": [500]}},
    "includes": [60700],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-cosmetic_filters/cosmetic_filters.grd": {
    "META": {"sizes": {"includes": [250]}},
    "includes": [61200],
  },
  "brave/components/tor/resources/tor_static_resources.grd": {
    "includes": [61450],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-tor_internals/tor_internals.grd": {
    "META": {"sizes": {"includes": [250]}},
    "includes": [61700],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-brave_wallet_script/brave_wallet_script.grd": {
    "META": {"sizes": {"includes": [250]}},
    "includes": [61950],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-brave_vpn_panel/brave_vpn_panel.grd": {
    "META": {"sizes": {"includes": [50]}},
    "includes": [62200],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-brave_shields_panel/brave_shields_panel.grd": {
    "META": {"sizes": {"includes": [200]}},
    "includes": [62250],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-trezor_bridge/trezor_bridge.grd": {
    "META": {"sizes": {"includes": [250]}},
    "includes": [62450],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/browser/resources/sidebar/sidebar_resources.grd": {
    "META": {"sizes": {"includes": [250]}},
    "includes": [62700],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/browser/resources/federated_internals/federated_internals_resources.grd": {
    "META": {"sizes": {"includes": [250]}},
    "includes": [62950],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-market_display/market_display.grd": {
    "META": {"sizes": {"includes": [250]}},
    "includes": [63200],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-brave_private_new_tab/brave_private_new_tab.grd": {
    "META": {"sizes": {"includes": [20]}},
    "includes": [63220],
  },
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-playlist/playlist.grd": {
    "META": {"sizes": {"includes": [250]}},
    "includes": [63470],
  },
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-ledger_bridge/ledger_bridge.grd": {
    "META": {"sizes": {"includes": [250]}},
    "includes": [63720],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-nft_display/nft_display.grd": {
    "META": {"sizes": {"includes": [250]}},
    "includes": [63970],
  },
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-cookie_list_opt_in/cookie_list_opt_in.grd": {
    "META": {"sizes": {"includes": [30]}},
    "includes": [64000],
  },
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-brave_speedreader_panel/brave_speedreader_panel.grd": {
    "META": {"sizes": {"includes": [20]}},
    "includes": [64020],
  },
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-brave_adblock_internals/brave_adblock_internals.grd": {
    "META": {"sizes": {"includes": [50]}},
    "includes": [64040],
  },
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-commands/commands.grd": {
    "META": {"sizes": {"includes": [250]}},
    "includes": [64290],
  },
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-brave_wallet_swap_page/brave_wallet_swap_page.grd": {
    "META": {"sizes": {"includes": [250]}},
    "includes": [64300],
  },
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-brave_wallet_send_page/brave_wallet_send_page.grd": {
    "META": {"sizes": {"includes": [250]}},
    "includes": [64350],
  },
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-tip_panel/tip_panel.grd": {
    "META": {"sizes": {"includes": [100]}},
    "includes": [64400]
  },
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-ai_chat_ui/ai_chat_ui.grd": {
    "META": {"sizes": {"includes": [50]}},
    "includes": [64500],
  },
  "<(SHARED_INTERMEDIATE_DIR)/brave/web-ui-skus_internals/skus_internals.grd": {
    "META": {"sizes": {"includes": [100]}},
    "includes": [64550],
  }
}
