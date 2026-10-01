#ifndef _MYWATCH_WATCH_UI_TOKENS_H_
#define _MYWATCH_WATCH_UI_TOKENS_H_

#include <cstdint>

namespace watch_ui {

inline constexpr char kBrandText[] = "MYWATCH";
inline constexpr char kTalkButtonText[] = "AI";
inline constexpr char kTalkHintText[] = "点击开始对话";
inline constexpr char kWaitingForTimeText[] = "正在同步时间";
inline constexpr char kNetworkOnlineText[] = "在线";
inline constexpr char kNetworkConnectingText[] = "...";
inline constexpr char kNetworkOfflineText[] = "离线";

inline constexpr uint32_t kBackgroundColor = 0x000000;
inline constexpr uint32_t kPrimaryColor = 0x315CFF;
inline constexpr uint32_t kPrimaryBorderColor = 0x8BA4FF;
inline constexpr uint32_t kPrimaryShadowColor = 0x2448CC;
inline constexpr uint32_t kBrandColor = 0x7F8CFF;
inline constexpr uint32_t kSecondaryTextColor = 0x9AA0AA;
inline constexpr uint32_t kHintTextColor = 0x8A8F99;
inline constexpr uint32_t kSuccessColor = 0x2FD17B;
inline constexpr uint32_t kWarningColor = 0xF1B84B;
inline constexpr uint32_t kDangerColor = 0xFF5F68;

inline constexpr int kScreenCornerRadius = 34;
inline constexpr int kStatusTopOffset = 42;
inline constexpr int kStatusSideOffset = 50;
inline constexpr int kBrandTopOffset = 82;
inline constexpr int kTimeCenterOffsetY = -84;
inline constexpr int kDateCenterOffsetY = -35;
inline constexpr int kTalkButtonCenterOffsetY = 75;
inline constexpr int kTalkButtonSize = 116;
inline constexpr int kTalkButtonBorderWidth = 2;
inline constexpr int kTalkButtonShadowWidth = 18;
inline constexpr int kHintBottomOffset = -47;
inline constexpr int kLowBatteryThreshold = 15;
inline constexpr uint32_t kRefreshPeriodMs = 100;
inline constexpr int kMinimumValidYear = 2025;

}  // namespace watch_ui

#endif  // _MYWATCH_WATCH_UI_TOKENS_H_
