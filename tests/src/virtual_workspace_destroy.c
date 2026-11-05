/* Lifecycle fixtures run the production policy, including separate event turns. */
static struct {
    struct { uint32_t wid; pid_t pid; uint64_t sid; bool unsupported; int moves; } windows[8];
    int count;
    uint32_t fail_wid;
    bool unknown;
    bool fail_collect;
    bool fail_settle;
    bool focus_visibility;
    bool restore_focus;
    int settles;
    int fail_settle_at;
    int restores;
    int completed;
    int result;
    int moved;
    uint64_t scheduled;
} virtual_workspace_destroy_test;
static struct virtual_workspace_source virtual_workspace_destroy_test_source;

static enum virtual_workspace_occupancy virtual_workspace_destroy_test_occupancy(uint64_t sid)
{
    if (virtual_workspace_destroy_test.unknown) return VIRTUAL_WORKSPACE_UNKNOWN;
    for (int i = 0; i < virtual_workspace_destroy_test.count; ++i) {
        if (virtual_workspace_destroy_test.windows[i].sid == sid) return VIRTUAL_WORKSPACE_OCCUPIED;
    }
    return VIRTUAL_WORKSPACE_EMPTY;
}

static bool virtual_workspace_destroy_test_collect(uint64_t sid, struct virtual_workspace_window **windows)
{
    if (virtual_workspace_destroy_test.unknown || virtual_workspace_destroy_test.fail_collect) return false;
    for (int i = 0; i < virtual_workspace_destroy_test.count; ++i) {
        if (virtual_workspace_destroy_test.windows[i].sid != sid) continue;
        if (virtual_workspace_destroy_test.windows[i].unsupported) return false;
        buf_push(*windows, ((struct virtual_workspace_window) {
            .wid = virtual_workspace_destroy_test.windows[i].wid,
            .pid = virtual_workspace_destroy_test.windows[i].pid
        }));
    }
    return true;
}

static enum virtual_workspace_window_state virtual_workspace_destroy_test_window_state(struct virtual_workspace_window *window, uint64_t src, uint64_t dst)
{
    for (int i = 0; i < virtual_workspace_destroy_test.count; ++i) {
        if (virtual_workspace_destroy_test.windows[i].wid != window->wid || virtual_workspace_destroy_test.windows[i].pid != window->pid) continue;
        if (virtual_workspace_destroy_test.windows[i].unsupported) return VIRTUAL_WORKSPACE_WINDOW_UNKNOWN;
        uint64_t sid = virtual_workspace_destroy_test.windows[i].sid;
        return sid == src ? VIRTUAL_WORKSPACE_WINDOW_SOURCE : sid == dst ? VIRTUAL_WORKSPACE_WINDOW_TARGET : VIRTUAL_WORKSPACE_WINDOW_LEFT;
    }
    return VIRTUAL_WORKSPACE_WINDOW_LEFT;
}

static void virtual_workspace_destroy_test_move(struct virtual_workspace_window *window, uint64_t sid)
{
    for (int i = 0; i < virtual_workspace_destroy_test.count; ++i) {
        if (virtual_workspace_destroy_test.windows[i].wid != window->wid) continue;
        ++virtual_workspace_destroy_test.windows[i].moves;
        if (window->wid != virtual_workspace_destroy_test.fail_wid) virtual_workspace_destroy_test.windows[i].sid = sid;
    }
}

static bool virtual_workspace_destroy_test_settle(struct virtual_workspace_window *window)
{
    (void) window;
    ++virtual_workspace_destroy_test.settles;
    return !virtual_workspace_destroy_test.fail_settle && virtual_workspace_destroy_test.settles != virtual_workspace_destroy_test.fail_settle_at;
}

static void virtual_workspace_destroy_test_capture(struct virtual_workspace_window *window) { (void) window; }
static bool virtual_workspace_destroy_test_can_destroy(uint32_t src, uint32_t dst) { (void) src; (void) dst; return true; }
static bool virtual_workspace_destroy_test_restore(struct virtual_workspace_window *window, bool request)
{
    (void) window;
    if (request) ++virtual_workspace_destroy_test.restores;
    return virtual_workspace_destroy_test.restore_focus;
}

static enum space_op_error virtual_workspace_destroy_test_focus(uint64_t sid)
{
    enum space_op_error result = virtual_workspace_test_focus(sid);
    if (result == SPACE_OP_ERROR_SUCCESS && virtual_workspace_destroy_test.focus_visibility) {
        for (int i = 0; i < 2; ++i) {
            if (virtual_workspace_test_state.displays[i] == virtual_workspace_test_space_display(sid)) virtual_workspace_test_state.visible_sid[i] = sid;
        }
    }
    return result;
}

static void virtual_workspace_destroy_test_schedule(uint64_t token) { virtual_workspace_destroy_test.scheduled = token; }
static void virtual_workspace_destroy_test_complete(struct virtual_workspace_destroy *operation, enum virtual_workspace_error result)
{
    ++virtual_workspace_destroy_test.completed;
    virtual_workspace_destroy_test.result = result;
    for (int i = 0; i < buf_len(operation->windows); ++i) if (operation->windows[i].moved) ++virtual_workspace_destroy_test.moved;
}

static bool virtual_workspace_destroy_test_begin(struct virtual_workspace_manager *vm)
{
    virtual_workspace_test_reset();
    memset(&virtual_workspace_destroy_test, 0, sizeof(virtual_workspace_destroy_test));
    virtual_workspace_destroy_test.focus_visibility = true;
    virtual_workspace_destroy_test.restore_focus = true;
    if (!virtual_workspace_manager_begin(vm, &virtual_workspace_test_source)) return false;
    virtual_workspace_destroy_test_source = virtual_workspace_test_source;
    virtual_workspace_destroy_test_source.occupancy = virtual_workspace_destroy_test_occupancy;
    virtual_workspace_destroy_test_source.can_destroy = virtual_workspace_destroy_test_can_destroy;
    virtual_workspace_destroy_test_source.collect_windows = virtual_workspace_destroy_test_collect;
    virtual_workspace_destroy_test_source.window_state = virtual_workspace_destroy_test_window_state;
    virtual_workspace_destroy_test_source.move_window = virtual_workspace_destroy_test_move;
    virtual_workspace_destroy_test_source.settle_window = virtual_workspace_destroy_test_settle;
    virtual_workspace_destroy_test_source.capture_focus = virtual_workspace_destroy_test_capture;
    virtual_workspace_destroy_test_source.restore_focus = virtual_workspace_destroy_test_restore;
    virtual_workspace_destroy_test_source.focus_space = virtual_workspace_destroy_test_focus;
    virtual_workspace_destroy_test_source.schedule_destroy = virtual_workspace_destroy_test_schedule;
    vm->source = &virtual_workspace_destroy_test_source;
    return true;
}

static void virtual_workspace_destroy_test_add(uint32_t wid, uint64_t sid)
{
    int i = virtual_workspace_destroy_test.count++;
    virtual_workspace_destroy_test.windows[i].wid = wid;
    virtual_workspace_destroy_test.windows[i].pid = 50;
    virtual_workspace_destroy_test.windows[i].sid = sid;
}

static enum virtual_workspace_error virtual_workspace_destroy_test_start(struct virtual_workspace_manager *vm, int src, int dst)
{
    return virtual_workspace_manager_destroy(vm, src, dst, NULL, virtual_workspace_destroy_test_complete);
}

static void virtual_workspace_destroy_test_drain(struct virtual_workspace_manager *vm)
{
    for (int i = 0; vm->destroy.token && i < 500; ++i) virtual_workspace_manager_destroy_continue(vm, vm->destroy.token);
}

TEST_FUNC(virtual_workspace_destroy_hidden_and_create,
{
    struct virtual_workspace_manager vm = {0};
    TEST_CHECK(virtual_workspace_destroy_test_begin(&vm), true);
    struct virtual_workspace survivors[3];
    memcpy(survivors, vm.workspaces + 1, sizeof(survivors));
    TEST_CHECK(virtual_workspace_destroy_test_start(&vm, 1, 0), VIRTUAL_WORKSPACE_PENDING);
    TEST_CHECK(vm.destroy.target.id == 2, true);
    TEST_CHECK(buf_len(vm.workspaces), 4);
    virtual_workspace_manager_destroy_continue(&vm, vm.destroy.token);
    TEST_CHECK(buf_len(vm.workspaces), 4); /* First empty observation cannot release. */
    virtual_workspace_destroy_test_drain(&vm);
    TEST_CHECK(virtual_workspace_destroy_test.result, VIRTUAL_WORKSPACE_SUCCESS);
    TEST_CHECK(memcmp(vm.workspaces, survivors, sizeof(survivors)), 0);
    TEST_CHECK(virtual_workspace_test_state.focused_sid == 0, true);
    TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_SUCCESS);
    TEST_CHECK(vm.workspaces[3].sid == 700, true);
    TEST_CHECK(vm.workspaces[3].id == 5, true);
    TEST_CHECK(vm.workspaces[3].index, 5);
    buf_free(vm.workspaces);
});

TEST_FUNC(virtual_workspace_destroy_adjacent_and_visible,
{
    struct virtual_workspace_manager vm = {0};
    TEST_CHECK(virtual_workspace_destroy_test_begin(&vm), true);
    /* Interleave display mappings and use sparse indices with distinct IDs. */
    struct virtual_workspace a = vm.workspaces[1];
    vm.workspaces[1] = vm.workspaces[2];
    vm.workspaces[2] = a;
    for (int i = 0; i < 4; ++i) { vm.workspaces[i].index = i * 3 + 2; vm.workspaces[i].id += 100; }
    TEST_CHECK(virtual_workspace_destroy_test_start(&vm, 8, 0), VIRTUAL_WORKSPACE_PENDING);
    TEST_CHECK(vm.destroy.target.index, 2);
    virtual_workspace_destroy_test_drain(&vm);
    TEST_CHECK(virtual_workspace_destroy_test.result, VIRTUAL_WORKSPACE_SUCCESS);
    TEST_CHECK(virtual_workspace_test_state.active_sid == 700, true);
    TEST_CHECK(virtual_workspace_test_state.visible_sid[0] == 700, true);
    TEST_CHECK(vm.workspaces[1].index, 5);
    TEST_CHECK(vm.workspaces[2].index, 11);
    buf_free(vm.workspaces);
});

TEST_FUNC(virtual_workspace_destroy_preflight,
{
    struct virtual_workspace_manager vm = {0};
    TEST_CHECK(virtual_workspace_destroy_test_begin(&vm), true);
    TEST_CHECK(virtual_workspace_destroy_test_start(&vm, 99, 0), VIRTUAL_WORKSPACE_INVALID_INDEX);
    TEST_CHECK(virtual_workspace_destroy_test_start(&vm, 1, 99), VIRTUAL_WORKSPACE_INVALID_TARGET);
    TEST_CHECK(virtual_workspace_destroy_test_start(&vm, 1, 1), VIRTUAL_WORKSPACE_INVALID_TARGET);
    virtual_workspace_test_state.non_user_sid = 300;
    TEST_CHECK(virtual_workspace_destroy_test_start(&vm, 1, 2), VIRTUAL_WORKSPACE_INVALID_TARGET);
    TEST_CHECK(virtual_workspace_destroy_test_start(&vm, 1, 0), VIRTUAL_WORKSPACE_LOCAL_TARGET_REQUIRED);
    virtual_workspace_test_state.non_user_sid = 0;
    virtual_workspace_destroy_test_add(10, 700);
    virtual_workspace_destroy_test.windows[0].unsupported = true;
    TEST_CHECK(virtual_workspace_destroy_test_start(&vm, 1, 0), VIRTUAL_WORKSPACE_UNSAFE_WINDOWS);
    TEST_CHECK(virtual_workspace_destroy_test.windows[0].moves, 0);
    TEST_CHECK(buf_len(vm.workspaces), 4);
    virtual_workspace_destroy_test.windows[0].unsupported = false;
    virtual_workspace_destroy_test.unknown = true;
    TEST_CHECK(virtual_workspace_destroy_test_start(&vm, 1, 0), VIRTUAL_WORKSPACE_UNSAFE_WINDOWS);
    virtual_workspace_destroy_test.unknown = false;
    virtual_workspace_test_state.missing_spaces = true;
    TEST_CHECK(virtual_workspace_destroy_test_start(&vm, 1, 0), VIRTUAL_WORKSPACE_STATE_UNAVAILABLE);
    virtual_workspace_test_state.missing_spaces = false;
    buf__hdr(vm.workspaces)->len = 1;
    TEST_CHECK(virtual_workspace_destroy_test_start(&vm, 1, 0), VIRTUAL_WORKSPACE_LAST);
    buf_free(vm.workspaces);
});

TEST_FUNC(virtual_workspace_destroy_cross_display,
{
    struct virtual_workspace_manager vm = {0};
    TEST_CHECK(virtual_workspace_destroy_test_begin(&vm), true);
    /* Source 500 is visible on the unfocused display. Local 400 replaces it. */
    virtual_workspace_destroy_test_add(10, 500);
    TEST_CHECK(virtual_workspace_destroy_test_start(&vm, 3, 1), VIRTUAL_WORKSPACE_PENDING);
    TEST_CHECK(vm.destroy.replacement.sid == 400, true);
    virtual_workspace_destroy_test_drain(&vm);
    TEST_CHECK(virtual_workspace_destroy_test.result, VIRTUAL_WORKSPACE_SUCCESS);
    TEST_CHECK(virtual_workspace_destroy_test.windows[0].sid == 700, true);
    TEST_CHECK(virtual_workspace_test_state.visible_sid[1] == 400, true);
    TEST_CHECK(virtual_workspace_test_state.active_sid == 300, true);
    TEST_CHECK(virtual_workspace_destroy_test.restores, 1);
    /* Removing the last local replacement must reject before any move. */
    TEST_CHECK(virtual_workspace_destroy_test_start(&vm, 4, 1), VIRTUAL_WORKSPACE_LOCAL_REPLACEMENT_REQUIRED);
    TEST_CHECK(virtual_workspace_destroy_test_start(&vm, 4, 0), VIRTUAL_WORKSPACE_LOCAL_TARGET_REQUIRED);
    TEST_CHECK(virtual_workspace_destroy_test.windows[0].moves, 1);
    buf_free(vm.workspaces);
});

TEST_FUNC(virtual_workspace_destroy_partial_retry,
{
    struct virtual_workspace_manager vm = {0};
    TEST_CHECK(virtual_workspace_destroy_test_begin(&vm), true);
    virtual_workspace_destroy_test_add(10, 700);
    virtual_workspace_destroy_test_add(11, 700);
    virtual_workspace_destroy_test.fail_wid = 11;
    TEST_CHECK(virtual_workspace_destroy_test_start(&vm, 1, 2), VIRTUAL_WORKSPACE_PENDING);
    virtual_workspace_destroy_test_drain(&vm);
    TEST_CHECK(virtual_workspace_destroy_test.result, VIRTUAL_WORKSPACE_MOVE_FAILED);
    TEST_CHECK(buf_len(vm.workspaces), 4);
    TEST_CHECK(virtual_workspace_destroy_test.moved, 1);
    TEST_CHECK(virtual_workspace_destroy_test.windows[0].sid == 300, true);
    TEST_CHECK(virtual_workspace_destroy_test.windows[1].sid == 700, true);
    TEST_CHECK(virtual_workspace_destroy_test.settles >= 2, true);
    virtual_workspace_destroy_test.fail_wid = 0;
    TEST_CHECK(virtual_workspace_destroy_test_start(&vm, 1, 2), VIRTUAL_WORKSPACE_PENDING);
    virtual_workspace_destroy_test_drain(&vm);
    TEST_CHECK(virtual_workspace_destroy_test.result, VIRTUAL_WORKSPACE_SUCCESS);
    TEST_CHECK(virtual_workspace_destroy_test.windows[0].moves, 1);
    TEST_CHECK(virtual_workspace_destroy_test.windows[1].moves, 2);
    TEST_CHECK(buf_len(vm.workspaces), 3);
    buf_free(vm.workspaces);
});

TEST_FUNC(virtual_workspace_destroy_races_and_tokens,
{
    for (int change = 0; change < 6; ++change) {
        struct virtual_workspace_manager vm = {0};
        TEST_CHECK(virtual_workspace_destroy_test_begin(&vm), true);
        virtual_workspace_destroy_test_add(10, 700);
        TEST_CHECK(virtual_workspace_destroy_test_start(&vm, 1, 2), VIRTUAL_WORKSPACE_PENDING);
        uint64_t token = vm.destroy.token;
        TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_BUSY);
        TEST_CHECK(virtual_workspace_manager_focus(&vm, 3), VIRTUAL_WORKSPACE_BUSY);
        TEST_CHECK(virtual_workspace_destroy_test_start(&vm, 3, 4), VIRTUAL_WORKSPACE_BUSY);
        virtual_workspace_manager_destroy_continue(&vm, token + 1);
        TEST_CHECK(virtual_workspace_destroy_test.windows[0].moves, 0);
        switch (change) {
        case 0: virtual_workspace_destroy_test.windows[0].sid = 0; break; /* Closed. */
        case 1: virtual_workspace_destroy_test.windows[0].sid = 500; break; /* Left independently. */
        case 2: ++virtual_workspace_destroy_test.windows[0].pid; break; /* Reused ID. */
        case 3: virtual_workspace_test_state.display_count = 1; break;
        case 4: virtual_workspace_test_state.spaces[0][0] = 999; break;
        case 5: virtual_workspace_destroy_test.windows[0].unsupported = true; break;
        }
        virtual_workspace_destroy_test_drain(&vm);
        TEST_CHECK(virtual_workspace_destroy_test.windows[0].moves, 0);
        TEST_CHECK(buf_len(vm.workspaces), change < 2 ? 3 : 4);
        TEST_CHECK(virtual_workspace_destroy_test.result == VIRTUAL_WORKSPACE_SUCCESS, change < 2);
        virtual_workspace_manager_destroy_continue(&vm, token);
        TEST_CHECK(virtual_workspace_destroy_test.completed, 1);
        buf_free(vm.workspaces);
    }
});

TEST_FUNC(virtual_workspace_destroy_arrivals_and_handoff_failure,
{
    for (int change = 0; change < 6; ++change) {
        struct virtual_workspace_manager vm = {0};
        TEST_CHECK(virtual_workspace_destroy_test_begin(&vm), true);
        TEST_CHECK(virtual_workspace_destroy_test_start(&vm, 2, 1), VIRTUAL_WORKSPACE_PENDING);
        if (change == 0) virtual_workspace_destroy_test_add(20, 300);
        if (change == 1) virtual_workspace_destroy_test.unknown = true;
        if (change == 2) virtual_workspace_destroy_test.focus_visibility = false;
        if (change == 3) virtual_workspace_test_state.focus_changes_space = false;
        if (change == 4) virtual_workspace_test_state.focus_result = SPACE_OP_ERROR_IN_MISSION_CONTROL;
        if (change == 5) {
            /* An arrival between the two final empty observations blocks release. */
            virtual_workspace_manager_destroy_continue(&vm, vm.destroy.token);
            virtual_workspace_manager_destroy_continue(&vm, vm.destroy.token);
            virtual_workspace_destroy_test_add(20, 300);
        }
        virtual_workspace_destroy_test_drain(&vm);
        TEST_CHECK(buf_len(vm.workspaces), 4);
        TEST_CHECK(virtual_workspace_destroy_test.result != VIRTUAL_WORKSPACE_SUCCESS, true);
        buf_free(vm.workspaces);
    }
});

TEST_FUNC(virtual_workspace_destroy_settle_failure,
{
    struct virtual_workspace_manager vm = {0};
    TEST_CHECK(virtual_workspace_destroy_test_begin(&vm), true);
    virtual_workspace_destroy_test_add(10, 700);
    virtual_workspace_destroy_test.fail_settle = true;
    TEST_CHECK(virtual_workspace_destroy_test_start(&vm, 1, 2), VIRTUAL_WORKSPACE_PENDING);
    virtual_workspace_destroy_test_drain(&vm);
    TEST_CHECK(virtual_workspace_destroy_test.result, VIRTUAL_WORKSPACE_MOVE_FAILED);
    TEST_CHECK(buf_len(vm.workspaces), 4);
    TEST_CHECK(virtual_workspace_destroy_test.moved, 1);
    buf_free(vm.workspaces);
});

TEST_FUNC(virtual_workspace_destroy_window_classes,
{
    struct application application = { .is_hidden = true };
    struct window window = ((struct window) { .application = &application, .is_root = true, .role = kAXWindowRole, .subrole = kAXStandardWindowSubrole });
    TEST_CHECK(virtual_workspace_window_can_evacuate(&window, 0), true);
    window_set_flag(&window, WINDOW_FLOAT);
    window_set_flag(&window, WINDOW_MINIMIZE);
    TEST_CHECK(virtual_workspace_classify_window(&window, 0, 0), VIRTUAL_WORKSPACE_OCCUPIED);
    TEST_CHECK(virtual_workspace_window_can_evacuate(&window, 0), true);
    TEST_CHECK(virtual_workspace_window_can_evacuate(&window, 20), false);
    window_set_flag(&window, WINDOW_STICKY);
    TEST_CHECK(virtual_workspace_window_can_evacuate(&window, 0), false);
    window_clear_flag(&window, WINDOW_STICKY);
    window_set_flag(&window, WINDOW_FULLSCREEN);
    TEST_CHECK(virtual_workspace_window_can_evacuate(&window, 0), false);
    window_clear_flag(&window, WINDOW_FULLSCREEN);
    window.subrole = kAXDialogSubrole;
    TEST_CHECK(virtual_workspace_window_can_evacuate(&window, 0), true);
    window.is_root = false;
    TEST_CHECK(virtual_workspace_window_can_evacuate(&window, 0), false);
    window.is_root = true;
    window.subrole = kAXUnknownSubrole;
    TEST_CHECK(virtual_workspace_window_can_evacuate(&window, 0), false);
    TEST_CHECK(virtual_workspace_classify_window(&window, 0, 0), VIRTUAL_WORKSPACE_UNKNOWN);
});

TEST_FUNC(virtual_workspace_destroy_focused_cross_display,
{
    struct virtual_workspace_manager vm = {0};
    TEST_CHECK(virtual_workspace_destroy_test_begin(&vm), true);
    virtual_workspace_destroy_test_add(10, 300);
    TEST_CHECK(virtual_workspace_destroy_test_start(&vm, 2, 3), VIRTUAL_WORKSPACE_PENDING);
    virtual_workspace_destroy_test_drain(&vm);
    TEST_CHECK(virtual_workspace_destroy_test.result, VIRTUAL_WORKSPACE_SUCCESS);
    TEST_CHECK(virtual_workspace_test_state.visible_sid[0] == 700, true);
    TEST_CHECK(virtual_workspace_test_state.visible_sid[1] == 500, true);
    TEST_CHECK(virtual_workspace_test_state.active_sid == 500, true);
    TEST_CHECK(virtual_workspace_destroy_test.windows[0].sid == 500, true);
    TEST_CHECK(virtual_workspace_destroy_test.restores, 0);
    buf_free(vm.workspaces);
});

TEST_FUNC(virtual_workspace_destroy_late_failures,
{
    for (int change = 0; change < 3; ++change) {
        struct virtual_workspace_manager vm = {0};
        TEST_CHECK(virtual_workspace_destroy_test_begin(&vm), true);
        virtual_workspace_destroy_test_add(10, 700);
        if (change == 0) virtual_workspace_destroy_test.fail_wid = 10;
        if (change == 2) virtual_workspace_destroy_test.fail_settle_at = 2;
        TEST_CHECK(virtual_workspace_destroy_test_start(&vm, 1, 2), VIRTUAL_WORKSPACE_PENDING);
        virtual_workspace_manager_destroy_continue(&vm, vm.destroy.token);
        TEST_CHECK(buf_len(vm.workspaces), 4);
        if (change == 0) {
            virtual_workspace_manager_destroy_continue(&vm, vm.destroy.token);
            TEST_CHECK(buf_len(vm.workspaces), 4); /* Dispatch alone is insufficient. */
            virtual_workspace_destroy_test.windows[0].sid = 300; /* Native completion. */
        }
        if (change == 1) virtual_workspace_test_state.display_count = 1;
        virtual_workspace_destroy_test_drain(&vm);
        TEST_CHECK(virtual_workspace_destroy_test.windows[0].moves, 1);
        TEST_CHECK(virtual_workspace_destroy_test.moved, 1);
        TEST_CHECK(virtual_workspace_destroy_test.result == VIRTUAL_WORKSPACE_SUCCESS, change == 0);
        TEST_CHECK(buf_len(vm.workspaces), change == 0 ? 3 : 4);
        buf_free(vm.workspaces);
    }
});
