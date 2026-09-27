#include "watch_notifications_app.h"

#include <cstdio>

void WatchNotificationsApp::OnResume() {
    notifications_.MarkAllRead();
    WatchAppBase::OnResume();
}

void WatchNotificationsApp::BuildContent(lv_obj_t* content) {
    empty_ = AddLabel(content, "NO NOTIFICATIONS", 120, 0x929AA5);
    for (size_t index = 0; index < rows_.size(); ++index) {
        rows_[index] = AddLabel(content, "", 10 + static_cast<int>(index) * 92);
        lv_obj_set_style_text_align(rows_[index], LV_TEXT_ALIGN_LEFT, 0);
        lv_label_set_long_mode(rows_[index], LV_LABEL_LONG_MODE_DOTS);
    }
    AddButton(content, "CLEAR ALL", 95, 300, 160, 54, ClearCallback, this);
}

void WatchNotificationsApp::Refresh() {
    const auto snapshot = notifications_.GetSnapshot();
    if (snapshot.count == 0) {
        lv_obj_remove_flag(empty_, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(empty_, LV_OBJ_FLAG_HIDDEN);
    }
    for (size_t index = 0; index < rows_.size(); ++index) {
        if (index >= snapshot.count) {
            lv_label_set_text(rows_[index], "");
            continue;
        }
        char text[220];
        snprintf(text, sizeof(text), "%s  %s\n%s", snapshot.items[index].source.data(),
                 snapshot.items[index].title.data(), snapshot.items[index].body.data());
        lv_label_set_text(rows_[index], text);
    }
}

void WatchNotificationsApp::ClearCallback(lv_event_t* event) {
    auto* app = static_cast<WatchNotificationsApp*>(lv_event_get_user_data(event));
    app->notifications_.Clear();
    app->Refresh();
}
