/*
 * Copyright (c) 2026 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 *
 * Periodic full-screen LVGL invalidation, for slow-refresh displays
 * (e-ink/EPD) that accumulate ghosting after repeated partial refreshes.
 *
 * Implemented entirely in a ZMK module — no patches to ZMK core required.
 * Uses the public zmk_display_work_q() entrypoint to submit work onto the
 * display thread, and listens for activity-state changes so the timer is
 * paused while the keyboard is idle/asleep (matches blank-on-idle behavior).
 */

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>

#include <lvgl.h>

#include <zmk/display.h>
#include <zmk/activity.h>
#include <zmk/event_manager.h>
#include <zmk/events/activity_state_changed.h>

LOG_MODULE_REGISTER(zen_display_full_refresh, CONFIG_ZMK_LOG_LEVEL);

#define REFRESH_PERIOD K_SECONDS(CONFIG_ZEN_DISPLAY_FULL_REFRESH_PERIOD)

static void full_refresh_work_cb(struct k_work *work) {
    ARG_UNUSED(work);
    lv_obj_invalidate(lv_scr_act());
}
static K_WORK_DEFINE(full_refresh_work, full_refresh_work_cb);

static void full_refresh_timer_cb(struct k_timer *timer) {
    ARG_UNUSED(timer);
    if (zmk_display_is_initialized()) {
        k_work_submit_to_queue(zmk_display_work_q(), &full_refresh_work);
    }
}
static K_TIMER_DEFINE(full_refresh_timer, full_refresh_timer_cb, NULL);

static void start_timer(void) {
    k_timer_start(&full_refresh_timer, REFRESH_PERIOD, REFRESH_PERIOD);
}

static int zen_display_full_refresh_init(void) {
    LOG_INF("Starting full-refresh timer (period=%ds)",
            CONFIG_ZEN_DISPLAY_FULL_REFRESH_PERIOD);
    start_timer();
    return 0;
}
SYS_INIT(zen_display_full_refresh_init, APPLICATION, 91);

static int activity_listener(const zmk_event_t *eh) {
    struct zmk_activity_state_changed *ev = as_zmk_activity_state_changed(eh);
    if (ev == NULL) {
        return ZMK_EV_EVENT_BUBBLE;
    }
    if (ev->state == ZMK_ACTIVITY_ACTIVE) {
        start_timer();
    } else {
        k_timer_stop(&full_refresh_timer);
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(zen_display_full_refresh, activity_listener);
ZMK_SUBSCRIPTION(zen_display_full_refresh, zmk_activity_state_changed);
