#ifndef _MYWATCH_WATCH_UI_TOKENS_H_
#define _MYWATCH_WATCH_UI_TOKENS_H_

#include <cstdint>

namespace watch_ui {

inline constexpr char kBrandText[] = "MYWATCH";
inline constexpr char kTalkButtonText[] = "AI";
inline constexpr char kTalkHintText[] = "TAP TO TALK";
inline constexpr char kWaitingForTimeText[] = "Waiting for time";

inline constexpr uint32_t kBackgroundColor = 0x000000;
inline constexpr uint32_t kPrimaryColor = 0x315CFF;
inline constexpr uint32_t kPrimaryBorderColor = 0x8BA4FF;
inline constexpr uint32_t kPrimaryShadowColor = 0x2448CC;
inline constexpr uint32_t kBrandColor = 0x7F8CFF;
inline constexpr uint32_t kSecondaryTextColor = 0x9AA0AA;
inline constexpr uint32_t kHintTextColor = 0x8A8F99;

inline constexpr int kBrandTopOffset = 62;
inline constexpr int kTimeCenterOffsetY = -92;
inline constexpr int kDateCenterOffsetY = -43;
inline constexpr int kTalkButtonCenterOffsetY = 72;
inline constexpr int kTalkButtonSize = 116;
inline constexpr int kTalkButtonBorderWidth = 2;
inline constexpr int kTalkButtonShadowWidth = 18;
inline constexpr int kHintBottomOffset = -54;
inline constexpr uint32_t kClockRefreshPeriodMs = 1000;
inline constexpr int kMinimumValidYear = 2025;

}  // namespace watch_ui

#endif  // _MYWATCH_WATCH_UI_TOKENS_H_
