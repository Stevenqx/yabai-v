static int virtual_workspace_refresh_test_change;
static int virtual_workspace_refresh_test_query;
static int virtual_workspace_refresh_test_scans;
static int virtual_workspace_refresh_test_change_scan;
static enum virtual_workspace_occupancy virtual_workspace_refresh_test_occupancy(uint64_t sid)
{
    if (sid == 80 && ++virtual_workspace_refresh_test_scans == virtual_workspace_refresh_test_change_scan) {
        if (virtual_workspace_refresh_test_change == 10) virtual_workspace_test_state.occupied_sid = 80;
        if (virtual_workspace_refresh_test_change == 11) virtual_workspace_test_state.unknown_sid = 80;
        if (virtual_workspace_refresh_test_change == 12) virtual_workspace_test_state.visible_sid[1] = 400;
        if (virtual_workspace_refresh_test_change == 13) virtual_workspace_test_state.reassociated_sid = 80;
        if (virtual_workspace_refresh_test_change == 14) virtual_workspace_test_state.missing_spaces = true;
        if (virtual_workspace_refresh_test_change == 15) virtual_workspace_test_state.spaces[0][1] = 999;
    }
    if (sid == virtual_workspace_test_state.unknown_sid) return VIRTUAL_WORKSPACE_UNKNOWN;
    if (sid == 700 || sid == 600 || sid == virtual_workspace_test_state.occupied_sid) return VIRTUAL_WORKSPACE_OCCUPIED;
    return VIRTUAL_WORKSPACE_EMPTY;
}

static uint64_t *virtual_workspace_refresh_test_spaces(uint32_t did, int *count)
{
    if (virtual_workspace_test_state.space_queries + 1 == virtual_workspace_refresh_test_query) {
        switch (virtual_workspace_refresh_test_change) {
        case 1: virtual_workspace_test_state.occupied_sid = 80; break;
        case 2: virtual_workspace_test_state.unknown_sid = 80; break;
        case 3: virtual_workspace_test_state.visible_sid[1] = 400; break;
        case 4: virtual_workspace_test_state.visible_sid[0] = virtual_workspace_test_state.active_sid = 80; break;
        case 5: virtual_workspace_test_state.non_user_sid = 80; break;
        case 6: virtual_workspace_test_state.reassociated_sid = 80; break;
        case 7: virtual_workspace_test_state.spaces[0][1] = 999; break;
        case 8: virtual_workspace_test_state.missing_spaces = true; break;
        case 9: virtual_workspace_test_state.display_count = 1; break;
        }
    }
    return virtual_workspace_test_spaces(did, count);
}

static const struct virtual_workspace_source virtual_workspace_refresh_test_source = {
    .display_list = virtual_workspace_test_displays,
    .space_list = virtual_workspace_refresh_test_spaces,
    .active_space = virtual_workspace_test_active_space,
    .active_display = virtual_workspace_test_active_display,
    .visible_space = virtual_workspace_test_visible_space,
    .is_user_space = virtual_workspace_test_is_user_space,
    .space_display = virtual_workspace_test_space_display,
    .occupancy = virtual_workspace_refresh_test_occupancy,
    .focus_space = virtual_workspace_test_focus
};

static void virtual_workspace_refresh_test_begin(struct virtual_workspace_manager *vm)
{
    virtual_workspace_test_reset();
    virtual_workspace_refresh_test_change = virtual_workspace_refresh_test_query = virtual_workspace_refresh_test_scans = 0;
    virtual_workspace_refresh_test_change_scan = 2;
    memcpy(virtual_workspace_test_state.spaces[0], ((uint64_t[]) { 300, 80, 700 }), 3 * sizeof(uint64_t));
    memcpy(virtual_workspace_test_state.spaces[1], ((uint64_t[]) { 400, 500, 600 }), 3 * sizeof(uint64_t));
    virtual_workspace_test_state.space_count[1] = 3;
    *vm = (struct virtual_workspace_manager) { .source = &virtual_workspace_refresh_test_source, .next_id = 607, .did_begin = true };
    uint64_t order[] = { 300, 400, 80, 500, 700, 600 };
    for (int i = 0; i < array_count(order); ++i) {
        buf_push(vm->workspaces, ((struct virtual_workspace) { .id = (uint64_t)(i + 1) * 101, .sid = order[i], .did = i % 2 ? 12 : 90, .index = i + 1 }));
    }
}

TEST_FUNC(virtual_workspace_refresh_interleaved_and_idempotent,
{
    struct virtual_workspace_manager vm;
    virtual_workspace_refresh_test_begin(&vm);
    struct virtual_workspace original[6];
    memcpy(original, vm.workspaces, sizeof(original));
    struct virtual_workspace_refresh_entry *entries = NULL;
    TEST_CHECK(virtual_workspace_manager_refresh(&vm, &entries), VIRTUAL_WORKSPACE_SUCCESS);
    int *survivors = ((int[]) { 0, 3, 4, 5 });
    TEST_CHECK(buf_len(vm.workspaces), 4);
    TEST_CHECK(entries[1].retention, VIRTUAL_WORKSPACE_RELEASE);
    TEST_CHECK(entries[2].retention, VIRTUAL_WORKSPACE_RELEASE);
    TEST_CHECK(entries[0].retention, VIRTUAL_WORKSPACE_RETAIN_FOCUSED);
    TEST_CHECK(entries[3].retention, VIRTUAL_WORKSPACE_RETAIN_VISIBLE);
    for (int i = 0; i < 4; ++i) {
        struct virtual_workspace expected = original[survivors[i]];
        expected.index = i + 1;
        TEST_CHECK(memcmp(&expected, &vm.workspaces[i], sizeof(expected)), 0);
    }
    TEST_CHECK(vm.next_id == 607, true);
    TEST_CHECK(virtual_workspace_test_state.focused_sid == 0, true);
    TEST_CHECK(virtual_workspace_test_state.active_sid == 300, true);
    TEST_CHECK(virtual_workspace_test_state.visible_sid[1] == 500, true);
    buf_free(entries);
    struct virtual_workspace stable[4];
    memcpy(stable, vm.workspaces, sizeof(stable));
    TEST_CHECK(virtual_workspace_manager_refresh(&vm, &entries), VIRTUAL_WORKSPACE_SUCCESS);
    TEST_CHECK(buf_len(vm.workspaces), 4);
    TEST_CHECK(memcmp(stable, vm.workspaces, sizeof(stable)), 0);
    for (int i = 0; i < buf_len(entries); ++i) TEST_CHECK(entries[i].new_index, entries[i].workspace.index);
    buf_free(entries);
    /* Released metadata is no proof of emptiness: allocation still revalidates. */
    virtual_workspace_test_state.occupied_sid = 80;
    TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_NO_SPARE);
    virtual_workspace_test_state.occupied_sid = 0;
    TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_SUCCESS);
    TEST_CHECK(vm.workspaces[4].id == 607 && vm.workspaces[4].sid == 80, true);
    TEST_CHECK(vm.workspaces[4].index, 5);
    TEST_CHECK(virtual_workspace_manager_focus(&vm, 2), VIRTUAL_WORKSPACE_SUCCESS);
    TEST_CHECK(virtual_workspace_test_state.focused_sid == 500, true);
    buf_free(vm.workspaces);
});

TEST_FUNC(virtual_workspace_refresh_gaps_and_last,
{
    for (int only = 0; only < 2; ++only) {
        struct virtual_workspace_manager vm;
        virtual_workspace_refresh_test_begin(&vm);
        struct virtual_workspace_source source = virtual_workspace_refresh_test_source;
        source.occupancy = virtual_workspace_test_empty;
        vm.source = &source;
        for (int i = 0; i < buf_len(vm.workspaces); ++i) vm.workspaces[i].index *= 3;
        if (only) {
            /* All visible native desktops may be unmapped. Keep the first ID. */
            vm.workspaces[0] = vm.workspaces[2];
            buf__hdr(vm.workspaces)->len = 1;
        }
        struct virtual_workspace_refresh_entry *entries = NULL;
        TEST_CHECK(virtual_workspace_manager_refresh(&vm, &entries), VIRTUAL_WORKSPACE_SUCCESS);
        TEST_CHECK(buf_len(vm.workspaces), only ? 1 : 2);
        TEST_CHECK(vm.workspaces[0].index, 1);
        TEST_CHECK(vm.workspaces[0].id == (only ? 303 : 101), true);
        if (only) {
            TEST_CHECK(entries[0].retention, VIRTUAL_WORKSPACE_RETAIN_LAST);
        } else {
            TEST_CHECK(vm.workspaces[1].index, 2);
            TEST_CHECK(vm.workspaces[1].id == 404, true);
        }
        buf_free(entries);
        buf_free(vm.workspaces);
    }
});

TEST_FUNC(virtual_workspace_refresh_uncertain_mappings,
{
    for (int change = 0; change < 5; ++change) {
        struct virtual_workspace_manager vm;
        virtual_workspace_refresh_test_begin(&vm);
        if (change == 0) virtual_workspace_test_state.unknown_sid = 80;
        if (change == 1) virtual_workspace_test_state.non_user_sid = 80;
        if (change == 2) virtual_workspace_test_state.reassociated_sid = 80;
        if (change == 3) virtual_workspace_test_state.spaces[0][1] = 999;
        if (change == 4) virtual_workspace_test_state.display_count = 1;
        struct virtual_workspace_refresh_entry *entries = NULL;
        TEST_CHECK(virtual_workspace_manager_refresh(&vm, &entries), VIRTUAL_WORKSPACE_SUCCESS);
        if (change < 4) {
            TEST_CHECK(virtual_workspace_manager_find_space(&vm, 80) != NULL, true);
            TEST_CHECK(entries[2].retention, change ? VIRTUAL_WORKSPACE_RETAIN_MAPPING : VIRTUAL_WORKSPACE_RETAIN_UNKNOWN);
        } else {
            TEST_CHECK(entries[1].retention, VIRTUAL_WORKSPACE_RETAIN_DISPLAY);
            TEST_CHECK(entries[3].retention, VIRTUAL_WORKSPACE_RETAIN_DISPLAY);
            TEST_CHECK(entries[5].retention, VIRTUAL_WORKSPACE_RETAIN_DISPLAY);
        }
        buf_free(entries);
        buf_free(vm.workspaces);
    }
});

TEST_FUNC(virtual_workspace_refresh_unavailable_snapshot,
{
    for (int change = 0; change < 9; ++change) {
        struct virtual_workspace_manager vm;
        virtual_workspace_refresh_test_begin(&vm);
        struct virtual_workspace before[6];
        memcpy(before, vm.workspaces, sizeof(before));
        struct virtual_workspace *original = vm.workspaces;
        if (change == 0) virtual_workspace_test_state.missing_display = true;
        if (change == 1) virtual_workspace_test_state.missing_spaces = true;
        if (change == 2) virtual_workspace_test_state.visible_sid[1] = 0;
        if (change == 3) virtual_workspace_test_state.active_sid = 0;
        if (change == 4) virtual_workspace_test_state.active_sid = 80;
        if (change == 5) virtual_workspace_test_state.displays[1] = 90;
        if (change == 6) virtual_workspace_test_state.spaces[1][2] = 80;
        if (change == 7) virtual_workspace_test_state.spaces[0][1] = 0;
        if (change == 8) virtual_workspace_test_state.space_count[1] = 0;
        struct virtual_workspace_refresh_entry *entries = NULL;
        TEST_CHECK(virtual_workspace_manager_refresh(&vm, &entries), VIRTUAL_WORKSPACE_STATE_UNAVAILABLE);
        TEST_CHECK(entries == NULL && vm.workspaces == original, true);
        TEST_CHECK(buf_len(vm.workspaces), 6);
        TEST_CHECK(memcmp(before, vm.workspaces, sizeof(before)), 0);
        TEST_CHECK(vm.next_id == 607, true);
        buf_free(vm.workspaces);
    }
});

TEST_FUNC(virtual_workspace_refresh_revalidation,
{
    for (int query = 3; query <= 5; query += 2) {
        for (int change = 1; change <= 9; ++change) {
            struct virtual_workspace_manager vm;
            virtual_workspace_refresh_test_begin(&vm);
            struct virtual_workspace before[6];
            memcpy(before, vm.workspaces, sizeof(before));
            virtual_workspace_refresh_test_change = change;
            virtual_workspace_refresh_test_query = query;
            struct virtual_workspace_refresh_entry *entries = NULL;
            enum virtual_workspace_error status = virtual_workspace_manager_refresh(&vm, &entries);
            if (change >= 7) {
                TEST_CHECK(status, change >= 8 ? VIRTUAL_WORKSPACE_STATE_UNAVAILABLE : VIRTUAL_WORKSPACE_STATE_CHANGED);
                TEST_CHECK(memcmp(before, vm.workspaces, sizeof(before)), 0);
                TEST_CHECK(entries == NULL, true);
            } else {
                TEST_CHECK(status, VIRTUAL_WORKSPACE_SUCCESS);
                uint64_t protected_sid = change == 3 ? 400 : 80;
                TEST_CHECK(virtual_workspace_manager_find_space(&vm, protected_sid) != NULL, true);
                for (int i = 0; i < buf_len(vm.workspaces); ++i) TEST_CHECK(vm.workspaces[i].index, i + 1);
            }
            buf_free(entries);
            buf_free(vm.workspaces);
        }
    }
});

TEST_FUNC(virtual_workspace_refresh_late_arrivals,
{
    for (int change = 10; change <= 15; ++change) {
        struct virtual_workspace_manager vm;
        virtual_workspace_refresh_test_begin(&vm);
        virtual_workspace_refresh_test_change = change;
        virtual_workspace_refresh_test_change_scan = 3;
        struct virtual_workspace_refresh_entry *entries = NULL;
        enum virtual_workspace_error status = virtual_workspace_manager_refresh(&vm, &entries);
        if (change >= 14) {
            TEST_CHECK(status, change == 14 ? VIRTUAL_WORKSPACE_STATE_UNAVAILABLE : VIRTUAL_WORKSPACE_STATE_CHANGED);
            TEST_CHECK(entries == NULL, true);
            TEST_CHECK(buf_len(vm.workspaces), 6);
        } else {
            TEST_CHECK(status, VIRTUAL_WORKSPACE_SUCCESS);
            uint64_t protected_sid = change == 12 ? 400 : 80;
            TEST_CHECK(virtual_workspace_manager_find_space(&vm, protected_sid) != NULL, true);
            TEST_CHECK(entries[change == 12 ? 1 : 2].retention != VIRTUAL_WORKSPACE_RELEASE, true);
        }
        buf_free(entries);
        buf_free(vm.workspaces);
    }
});

TEST_FUNC(virtual_workspace_refresh_pending_destroy,
{
    struct virtual_workspace_manager vm = {0};
    struct virtual_workspace_refresh_entry *entries = NULL;
    TEST_CHECK(virtual_workspace_manager_refresh(&vm, &entries), VIRTUAL_WORKSPACE_NOT_INITIALIZED);
    TEST_CHECK(virtual_workspace_destroy_test_begin(&vm), true);
    virtual_workspace_destroy_test_add(10, 700);
    TEST_CHECK(virtual_workspace_manager_destroy(&vm, 1, 2, NULL, virtual_workspace_destroy_test_complete), VIRTUAL_WORKSPACE_PENDING);
    uint64_t token = vm.destroy.token;
    struct virtual_workspace *original = vm.workspaces;
    TEST_CHECK(virtual_workspace_manager_refresh(&vm, &entries), VIRTUAL_WORKSPACE_BUSY);
    TEST_CHECK(entries == NULL && vm.workspaces == original && vm.destroy.token == token, true);
    TEST_CHECK(vm.destroy.source.id == 1 && vm.destroy.target.id == 2, true);
    virtual_workspace_destroy_test_drain(&vm);
    TEST_CHECK(virtual_workspace_destroy_test.result, VIRTUAL_WORKSPACE_SUCCESS);
    buf_free(vm.workspaces);
});

static struct virtual_workspace_manager *virtual_workspace_refresh_test_live;
static struct window virtual_workspace_refresh_test_window;
static bool virtual_workspace_refresh_test_atomic;

static enum virtual_workspace_occupancy virtual_workspace_refresh_test_classified(uint64_t sid)
{
    struct virtual_workspace_manager *vm = virtual_workspace_refresh_test_live;
    /* Native callbacks/read queries must see the original complete generation. */
    virtual_workspace_refresh_test_atomic &= buf_len(vm->workspaces) == 3 &&
        vm->workspaces[0].id == 101 && vm->workspaces[1].index == 3 && vm->workspaces[2].index == 5;
    if (sid == 80) return virtual_workspace_classify_window(&virtual_workspace_refresh_test_window, 0, 0);
    return VIRTUAL_WORKSPACE_EMPTY;
}

TEST_FUNC(virtual_workspace_refresh_live_windows_and_publication,
{
    for (int kind = 0; kind < 6; ++kind) {
        struct virtual_workspace_manager vm;
        virtual_workspace_refresh_test_begin(&vm);
        virtual_workspace_test_state.display_count = 1;
        vm.workspaces[1] = vm.workspaces[2];
        vm.workspaces[2] = vm.workspaces[4];
        buf__hdr(vm.workspaces)->len = 3;
        struct application application = {0};
        application.is_hidden = kind == 2;
        virtual_workspace_refresh_test_window = ((struct window) { .application = &application, .role = kAXWindowRole, .subrole = kAXStandardWindowSubrole });
        if (kind == 0) window_set_flag(&virtual_workspace_refresh_test_window, WINDOW_FLOAT);
        if (kind == 1) window_set_flag(&virtual_workspace_refresh_test_window, WINDOW_MINIMIZE);
        if (kind == 3) virtual_workspace_refresh_test_window.subrole = kAXUnknownSubrole;
        if (kind == 4) virtual_workspace_refresh_test_window.subrole = NULL;
        if (kind == 5) window_set_flag(&virtual_workspace_refresh_test_window, WINDOW_STICKY);
        struct virtual_workspace_source source = virtual_workspace_refresh_test_source;
        source.occupancy = virtual_workspace_refresh_test_classified;
        vm.source = &source;
        virtual_workspace_refresh_test_live = &vm;
        virtual_workspace_refresh_test_atomic = true;
        struct virtual_workspace_refresh_entry *entries = NULL;
        TEST_CHECK(virtual_workspace_manager_refresh(&vm, &entries), VIRTUAL_WORKSPACE_SUCCESS);
        TEST_CHECK(virtual_workspace_refresh_test_atomic, true);
        TEST_CHECK(buf_len(vm.workspaces), 2);
        TEST_CHECK(vm.workspaces[1].id == 303 && vm.workspaces[1].index == 2, true);
        TEST_CHECK(entries[1].retention, kind == 3 || kind == 4 ? VIRTUAL_WORKSPACE_RETAIN_UNKNOWN : VIRTUAL_WORKSPACE_RETAIN_OCCUPIED);
        buf_free(entries);
        buf_free(vm.workspaces);
    }
    virtual_workspace_refresh_test_live = NULL;
});
