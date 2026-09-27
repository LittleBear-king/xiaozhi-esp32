set(MYWATCH_BOARD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD_DIR}")

list(APPEND SOURCES
    "${MYWATCH_BOARD_ROOT}/board/smartwatch_board.cc"
    "${MYWATCH_BOARD_ROOT}/controller/watch_controller.cc"
    "${MYWATCH_BOARD_ROOT}/hal/watch_backlight.cc"
    "${MYWATCH_BOARD_ROOT}/hal/watch_power.cc"
    "${MYWATCH_BOARD_ROOT}/model/watch_model.cc"
    "${MYWATCH_BOARD_ROOT}/ui/watch_display.cc"
    "${MYWATCH_BOARD_ROOT}/ui/watch_face.cc"
)

list(APPEND INCLUDE_DIRS "${MYWATCH_BOARD_ROOT}")
