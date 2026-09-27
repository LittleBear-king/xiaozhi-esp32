set(MYWATCH_BOARD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD_DIR}")

list(APPEND SOURCES
    "${MYWATCH_BOARD_ROOT}/apps/watch_app_router.cc"
    "${MYWATCH_BOARD_ROOT}/board/smartwatch_board.cc"
    "${MYWATCH_BOARD_ROOT}/controller/watch_controller.cc"
    "${MYWATCH_BOARD_ROOT}/hal/watch_backlight.cc"
    "${MYWATCH_BOARD_ROOT}/hal/watch_motion.cc"
    "${MYWATCH_BOARD_ROOT}/hal/watch_power.cc"
    "${MYWATCH_BOARD_ROOT}/hal/watch_rtc.cc"
    "${MYWATCH_BOARD_ROOT}/model/watch_model.cc"
    "${MYWATCH_BOARD_ROOT}/services/watch_health_service.cc"
    "${MYWATCH_BOARD_ROOT}/services/watch_motion_service.cc"
    "${MYWATCH_BOARD_ROOT}/services/watch_notification_service.cc"
    "${MYWATCH_BOARD_ROOT}/services/watch_phone_service.cc"
    "${MYWATCH_BOARD_ROOT}/services/watch_power_policy.cc"
    "${MYWATCH_BOARD_ROOT}/services/watch_reliability_service.cc"
    "${MYWATCH_BOARD_ROOT}/services/watch_settings_service.cc"
    "${MYWATCH_BOARD_ROOT}/services/watch_time_service.cc"
    "${MYWATCH_BOARD_ROOT}/ui/apps/watch_activity_app.cc"
    "${MYWATCH_BOARD_ROOT}/ui/apps/watch_app_base.cc"
    "${MYWATCH_BOARD_ROOT}/ui/apps/watch_launcher_app.cc"
    "${MYWATCH_BOARD_ROOT}/ui/apps/watch_notifications_app.cc"
    "${MYWATCH_BOARD_ROOT}/ui/apps/watch_settings_app.cc"
    "${MYWATCH_BOARD_ROOT}/ui/apps/watch_tools_app.cc"
    "${MYWATCH_BOARD_ROOT}/ui/watch_display.cc"
    "${MYWATCH_BOARD_ROOT}/ui/watch_face.cc"
)

list(APPEND INCLUDE_DIRS "${MYWATCH_BOARD_ROOT}")
