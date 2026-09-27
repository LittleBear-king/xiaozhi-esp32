#ifndef _MYWATCH_WATCH_APP_BASE_H_
#define _MYWATCH_WATCH_APP_BASE_H_

#include "apps/watch_app.h"

#include <lvgl.h>

class WatchAppBase : public WatchApp {
public:
    explicit WatchAppBase(const char* title) : title_(title) {}
    ~WatchAppBase() override;

    void Create(lv_obj_t* parent, WatchAppNavigator& navigator) final;
    void OnResume() override;
    void OnPause() override;

protected:
    virtual void BuildContent(lv_obj_t* content) = 0;
    virtual void Refresh() {}
    WatchAppNavigator& Navigator() const { return *navigator_; }
    lv_obj_t* Root() const { return root_; }

    lv_obj_t* AddLabel(lv_obj_t* parent, const char* text, int y, uint32_t color = 0xF7F8FA);
    lv_obj_t* AddButton(lv_obj_t* parent, const char* text, int x, int y, int width, int height,
                        lv_event_cb_t callback, void* user_data);

private:
    static void BackButtonCallback(lv_event_t* event);

    const char* title_;
    WatchAppNavigator* navigator_ = nullptr;
    lv_obj_t* root_ = nullptr;
    lv_obj_t* content_ = nullptr;
};

#endif  // _MYWATCH_WATCH_APP_BASE_H_
