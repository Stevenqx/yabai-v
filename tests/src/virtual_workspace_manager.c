struct virtual_workspace_test_state
{
    uint32_t displays[2];
    uint64_t spaces[2][16];
    int space_count[2];
    int display_count;
    uint32_t active_did;
    uint64_t visible_sid[2];
    uint64_t non_user_sid;
    uint64_t occupied_sid;
    uint64_t unknown_sid;
    uint64_t reassociated_sid;
    int space_queries;
    int change_on_query;
    int change;
    uint64_t active_sid;
    uint64_t focused_sid;
    enum space_op_error focus_result;
    bool focus_changes_space;
    bool missing_display;
    bool missing_spaces;
};

static struct virtual_workspace_test_state virtual_workspace_test_state;

static void virtual_workspace_test_reset(void)
{
    virtual_workspace_test_state = (struct virtual_workspace_test_state) {
        .displays = { 90, 12 },
        .spaces = { { 700, 80, 300 }, { 500, 400 } },
        .space_count = { 3, 2 },
        .display_count = 2,
        .active_did = 90,
        .visible_sid = { 300, 500 },
        .active_sid = 300,
        .focus_result = SPACE_OP_ERROR_SUCCESS,
        .focus_changes_space = true
    };
}

static uint32_t *virtual_workspace_test_displays(int *count)
{
    *count = virtual_workspace_test_state.display_count;
    return virtual_workspace_test_state.missing_display ? NULL : virtual_workspace_test_state.displays;
}

static uint64_t *virtual_workspace_test_spaces(uint32_t did, int *count)
{
    struct virtual_workspace_test_state *state = &virtual_workspace_test_state;
    if (++state->space_queries == state->change_on_query) {
        switch (state->change) {
        case 1: state->visible_sid[0] = 80; break;
        case 2: state->occupied_sid = 80; break;
        case 3: state->unknown_sid = 80; break;
        case 4: state->non_user_sid = 80; break;
        case 5: state->spaces[0][1] = 999; break;
        case 6: state->active_did = 12; break;
        case 7: state->reassociated_sid = 80; break;
        case 8: state->missing_spaces = true; break;
        case 9: state->visible_sid[0] = 0; break;
        }
    }
    for (int i = 0; i < 2; ++i) {
        if (virtual_workspace_test_state.displays[i] == did) {
            *count = virtual_workspace_test_state.space_count[i];
            return virtual_workspace_test_state.missing_spaces ? NULL : virtual_workspace_test_state.spaces[i];
        }
    }
    return NULL;
}

static uint64_t virtual_workspace_test_active_space(void)
{
    return virtual_workspace_test_state.active_sid;
}

static uint32_t virtual_workspace_test_active_display(void)
{
    return virtual_workspace_test_state.active_did;
}

static uint64_t virtual_workspace_test_visible_space(uint32_t did)
{
    for (int i = 0; i < virtual_workspace_test_state.display_count; ++i) {
        if (virtual_workspace_test_state.displays[i] == did) return virtual_workspace_test_state.visible_sid[i];
    }
    return 0;
}

static bool virtual_workspace_test_is_user_space(uint64_t sid)
{
    return sid != virtual_workspace_test_state.non_user_sid;
}

static uint32_t virtual_workspace_test_space_display(uint64_t sid)
{
    if (sid == virtual_workspace_test_state.reassociated_sid) return 12;
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < virtual_workspace_test_state.space_count[i]; ++j) {
            if (virtual_workspace_test_state.spaces[i][j] == sid) return virtual_workspace_test_state.displays[i];
        }
    }
    return 0;
}

static enum virtual_workspace_occupancy virtual_workspace_test_occupancy(uint64_t sid)
{
    if (sid == virtual_workspace_test_state.occupied_sid) return VIRTUAL_WORKSPACE_OCCUPIED;
    if (sid == virtual_workspace_test_state.unknown_sid) return VIRTUAL_WORKSPACE_UNKNOWN;
    if (sid == 700 || sid == 500) return VIRTUAL_WORKSPACE_OCCUPIED;
    if (sid == 400) return VIRTUAL_WORKSPACE_UNKNOWN;
    return VIRTUAL_WORKSPACE_EMPTY;
}

static enum space_op_error virtual_workspace_test_focus(uint64_t sid)
{
    virtual_workspace_test_state.focused_sid = sid;
    if (virtual_workspace_test_state.focus_changes_space && virtual_workspace_test_state.focus_result == SPACE_OP_ERROR_SUCCESS) {
        virtual_workspace_test_state.active_sid = sid;
    }
    return virtual_workspace_test_state.focus_result;
}

static const struct virtual_workspace_source virtual_workspace_test_source = {
    .display_list = virtual_workspace_test_displays,
    .space_list = virtual_workspace_test_spaces,
    .active_space = virtual_workspace_test_active_space,
    .active_display = virtual_workspace_test_active_display,
    .visible_space = virtual_workspace_test_visible_space,
    .is_user_space = virtual_workspace_test_is_user_space,
    .space_display = virtual_workspace_test_space_display,
    .occupancy = virtual_workspace_test_occupancy,
    .focus_space = virtual_workspace_test_focus
};

static const uint64_t virtual_workspace_test_order[] = { 700, 300, 500, 400 };

static enum virtual_workspace_occupancy virtual_workspace_test_empty(uint64_t sid)
{
    (void) sid;
    return VIRTUAL_WORKSPACE_EMPTY;
}

TEST_FUNC(virtual_workspace_only_empty_active,
{
    virtual_workspace_test_reset();
    struct virtual_workspace_source source = virtual_workspace_test_source;
    source.occupancy = virtual_workspace_test_empty;
    struct virtual_workspace_manager vm = {0};
    TEST_CHECK(virtual_workspace_manager_begin(&vm, &source), true);
    TEST_CHECK(buf_len(vm.workspaces), 1);
    TEST_CHECK(vm.workspaces[0].sid == 300, true);
    TEST_CHECK(vm.workspaces[0].did, 90);
    TEST_CHECK(vm.workspaces[0].index, 1);
    buf_free(vm.workspaces);
});

TEST_FUNC(virtual_workspace_mapping_and_order,
{
    virtual_workspace_test_reset();
    struct virtual_workspace_manager vm = {0};
    TEST_CHECK(virtual_workspace_manager_begin(&vm, &virtual_workspace_test_source), true);
    TEST_CHECK(buf_len(vm.workspaces), 4);
    for (int i = 0; i < buf_len(vm.workspaces); ++i) {
        TEST_CHECK(vm.workspaces[i].sid == virtual_workspace_test_order[i], true);
        TEST_CHECK(vm.workspaces[i].id == (uint64_t)i + 1, true);
        TEST_CHECK(vm.workspaces[i].index, i + 1);
        TEST_CHECK(vm.workspaces[i].did, i < 2 ? 90 : 12);
    }
    TEST_CHECK(virtual_workspace_manager_find_space(&vm, 80) == NULL, true);
    TEST_CHECK(virtual_workspace_manager_find_space(&vm, 300) != NULL, true);
    TEST_CHECK(virtual_workspace_manager_find_space(&vm, 400) != NULL, true);
    buf_free(vm.workspaces);
});

TEST_FUNC(virtual_workspace_lookup_and_session_stability,
{
    virtual_workspace_test_reset();
    struct virtual_workspace_manager vm = {0};
    TEST_CHECK(virtual_workspace_manager_begin(&vm, &virtual_workspace_test_source), true);
    TEST_CHECK(virtual_workspace_manager_find_index(&vm, -1) == NULL, true);
    TEST_CHECK(virtual_workspace_manager_find_index(&vm, 0) == NULL, true);
    TEST_CHECK(virtual_workspace_manager_find_index(&vm, 5) == NULL, true);
    TEST_CHECK(virtual_workspace_manager_find_index(&vm, INT_MAX) == NULL, true);
    TEST_CHECK(virtual_workspace_manager_find_space(&vm, 0) == NULL, true);
    TEST_CHECK(virtual_workspace_manager_find_space(&vm, 999) == NULL, true);
    TEST_CHECK(virtual_workspace_manager_find_index(&vm, 3) == virtual_workspace_manager_find_space(&vm, 500), true);
    virtual_workspace_test_state.active_sid = 80;
    virtual_workspace_test_state.spaces[0][0] = 999;
    TEST_CHECK(virtual_workspace_manager_begin(&vm, &virtual_workspace_test_source), true);
    TEST_CHECK(buf_len(vm.workspaces), 4);
    TEST_CHECK(vm.workspaces[0].sid == 700, true);
    TEST_CHECK(vm.workspaces[0].id == 1, true);
    TEST_CHECK(virtual_workspace_manager_find_space(&vm, 80) == NULL, true);
    buf_free(vm.workspaces);
});

TEST_FUNC(virtual_workspace_duplicate_prevention,
{
    virtual_workspace_test_reset();
    virtual_workspace_test_state.spaces[0][3] = 700;
    virtual_workspace_test_state.space_count[0] = 4;
    struct virtual_workspace_manager vm = {0};
    TEST_CHECK(virtual_workspace_manager_begin(&vm, &virtual_workspace_test_source), true);
    TEST_CHECK(buf_len(vm.workspaces), 4);
    buf_free(vm.workspaces);
    vm = (struct virtual_workspace_manager) {0};
    virtual_workspace_test_state.spaces[1][1] = 700;
    TEST_CHECK(virtual_workspace_manager_begin(&vm, &virtual_workspace_test_source), false);
    TEST_CHECK(vm.workspaces == NULL, true);
    TEST_CHECK(vm.did_begin, false);
});

TEST_FUNC(virtual_workspace_incomplete_enumeration,
{
    virtual_workspace_test_reset();
    struct virtual_workspace_manager vm = {0};
    virtual_workspace_test_state.missing_display = true;
    TEST_CHECK(virtual_workspace_manager_begin(&vm, &virtual_workspace_test_source), false);
    virtual_workspace_test_state.missing_display = false;
    virtual_workspace_test_state.missing_spaces = true;
    TEST_CHECK(virtual_workspace_manager_begin(&vm, &virtual_workspace_test_source), false);
    virtual_workspace_test_state.missing_spaces = false;
    virtual_workspace_test_state.active_sid = 999;
    TEST_CHECK(virtual_workspace_manager_begin(&vm, &virtual_workspace_test_source), false);
    virtual_workspace_test_state.active_sid = 0;
    TEST_CHECK(virtual_workspace_manager_begin(&vm, &virtual_workspace_test_source), false);
    TEST_CHECK(vm.workspaces == NULL, true);
    TEST_CHECK(vm.did_begin, false);
});

TEST_FUNC(virtual_workspace_window_occupancy,
{
    struct window window = {0};
    window.role = kAXWindowRole;
    window.subrole = kAXStandardWindowSubrole;
    /* No BSP node is required, in either BSP or float layout. */
    enum view_type original_layout = g_space_manager.layout;
    g_space_manager.layout = VIEW_BSP;
    TEST_CHECK(virtual_workspace_classify_window(&window, 0, 0), VIRTUAL_WORKSPACE_OCCUPIED);
    g_space_manager.layout = VIEW_FLOAT;
    window_set_flag(&window, WINDOW_FLOAT);
    TEST_CHECK(virtual_workspace_classify_window(&window, 0, 0), VIRTUAL_WORKSPACE_OCCUPIED);
    window_set_flag(&window, WINDOW_MINIMIZE);
    TEST_CHECK(virtual_workspace_classify_window(&window, 0, 0), VIRTUAL_WORKSPACE_OCCUPIED);
    window.subrole = kAXFloatingWindowSubrole;
    TEST_CHECK(virtual_workspace_classify_window(&window, 3, 0), VIRTUAL_WORKSPACE_OCCUPIED);
    window.subrole = kAXDialogSubrole;
    TEST_CHECK(virtual_workspace_classify_window(&window, 8, 0), VIRTUAL_WORKSPACE_OCCUPIED);
    window.subrole = NULL;
    TEST_CHECK(virtual_workspace_classify_window(&window, 0, 0), VIRTUAL_WORKSPACE_UNKNOWN);
    window.subrole = kAXUnknownSubrole;
    TEST_CHECK(virtual_workspace_classify_window(&window, 0, 0), VIRTUAL_WORKSPACE_UNKNOWN);
    window.subrole = CFSTR("CustomWindowSubrole");
    TEST_CHECK(virtual_workspace_classify_window(&window, 0, 0), VIRTUAL_WORKSPACE_UNKNOWN);
    window.role = CFSTR("CustomWindowRole");
    TEST_CHECK(virtual_workspace_classify_window(&window, 0, 0), VIRTUAL_WORKSPACE_UNKNOWN);
    window.role = kAXMenuRole;
    window.subrole = kAXStandardWindowSubrole;
    TEST_CHECK(virtual_workspace_classify_window(&window, 0, 0), VIRTUAL_WORKSPACE_EMPTY);
    TEST_CHECK(virtual_workspace_classify_window(NULL, 0, 0x1), VIRTUAL_WORKSPACE_UNKNOWN);
    TEST_CHECK(virtual_workspace_classify_window(NULL, 3, 0x1), VIRTUAL_WORKSPACE_UNKNOWN);
    TEST_CHECK(virtual_workspace_classify_window(NULL, 8, 0x1), VIRTUAL_WORKSPACE_UNKNOWN);
    TEST_CHECK(virtual_workspace_classify_window(NULL, -2147483623, 0x1), VIRTUAL_WORKSPACE_EMPTY);
    TEST_CHECK(virtual_workspace_classify_window(NULL, -1, 0x1), VIRTUAL_WORKSPACE_UNKNOWN);
    TEST_CHECK(virtual_workspace_classify_window(NULL, 20, 0x1), VIRTUAL_WORKSPACE_UNKNOWN);
    TEST_CHECK(virtual_workspace_classify_window(NULL, 25, 0x816000000d0440), VIRTUAL_WORKSPACE_EMPTY);
    TEST_CHECK(virtual_workspace_classify_window(NULL, 24, 0x8140000d000408), VIRTUAL_WORKSPACE_EMPTY);
    TEST_CHECK(virtual_workspace_classify_window(NULL, 0, 0x80000002), VIRTUAL_WORKSPACE_UNKNOWN);
    TEST_CHECK(virtual_workspace_classify_window(NULL, 0, 0x2), VIRTUAL_WORKSPACE_EMPTY);
    g_space_manager.layout = original_layout;
});

TEST_FUNC(virtual_workspace_focus_native_id,
{
    virtual_workspace_test_reset();
    struct virtual_workspace_manager vm = {0};
    TEST_CHECK(virtual_workspace_manager_focus(&vm, 1), VIRTUAL_WORKSPACE_NOT_INITIALIZED);
    TEST_CHECK(virtual_workspace_manager_begin(&vm, &virtual_workspace_test_source), true);
    TEST_CHECK(virtual_workspace_manager_focus(&vm, 0), VIRTUAL_WORKSPACE_INVALID_INDEX);
    TEST_CHECK(virtual_workspace_manager_focus(&vm, 5), VIRTUAL_WORKSPACE_INVALID_INDEX);
    TEST_CHECK(virtual_workspace_test_state.focused_sid == 0, true);
    TEST_CHECK(virtual_workspace_manager_focus(&vm, 3), VIRTUAL_WORKSPACE_SUCCESS);
    TEST_CHECK(virtual_workspace_test_state.focused_sid == 500, true);
    TEST_CHECK(virtual_workspace_test_state.active_sid == 500, true);
    virtual_workspace_test_state.spaces[1][0] = 999;
    virtual_workspace_test_state.focused_sid = 0;
    TEST_CHECK(virtual_workspace_manager_focus(&vm, 3), VIRTUAL_WORKSPACE_SPACE_UNAVAILABLE);
    TEST_CHECK(virtual_workspace_test_state.focused_sid == 0, true);
    virtual_workspace_test_state.reassociated_sid = 700;
    TEST_CHECK(virtual_workspace_manager_focus(&vm, 1), VIRTUAL_WORKSPACE_SPACE_UNAVAILABLE);
    TEST_CHECK(virtual_workspace_test_state.focused_sid == 0, true);
    buf_free(vm.workspaces);
});

TEST_FUNC(virtual_workspace_focus_failures,
{
    virtual_workspace_test_reset();
    struct virtual_workspace_manager vm = {0};
    TEST_CHECK(virtual_workspace_manager_begin(&vm, &virtual_workspace_test_source), true);
    virtual_workspace_test_state.focus_result = SPACE_OP_ERROR_SAME_SPACE;
    TEST_CHECK(virtual_workspace_manager_focus(&vm, 1), VIRTUAL_WORKSPACE_SAME_SPACE);
    virtual_workspace_test_state.focus_result = SPACE_OP_ERROR_DISPLAY_IS_ANIMATING;
    TEST_CHECK(virtual_workspace_manager_focus(&vm, 1), VIRTUAL_WORKSPACE_DISPLAY_IS_ANIMATING);
    virtual_workspace_test_state.focus_result = SPACE_OP_ERROR_IN_MISSION_CONTROL;
    TEST_CHECK(virtual_workspace_manager_focus(&vm, 1), VIRTUAL_WORKSPACE_IN_MISSION_CONTROL);
    virtual_workspace_test_state.focus_result = SPACE_OP_ERROR_SCRIPTING_ADDITION;
    TEST_CHECK(virtual_workspace_manager_focus(&vm, 1), VIRTUAL_WORKSPACE_SCRIPTING_ADDITION);
    virtual_workspace_test_state.focus_result = SPACE_OP_ERROR_SUCCESS;
    virtual_workspace_test_state.focus_changes_space = false;
    /* Native success is dispatch success, even before macOS changes focus. */
    TEST_CHECK(virtual_workspace_manager_focus(&vm, 1), VIRTUAL_WORKSPACE_SUCCESS);
    TEST_CHECK(virtual_workspace_test_state.active_sid == 300, true);
    buf_free(vm.workspaces);
});

TEST_FUNC(virtual_workspace_create_one_display,
{
    virtual_workspace_test_reset();
    virtual_workspace_test_state.display_count = 1;
    struct virtual_workspace_source source = virtual_workspace_test_source;
    source.occupancy = virtual_workspace_test_empty;
    struct virtual_workspace_manager vm = {0};
    TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_NOT_INITIALIZED);
    TEST_CHECK(virtual_workspace_manager_begin(&vm, &source), true);
    /* Identity and index are independent; append after the highest index. */
    vm.workspaces[0].id = 900;
    vm.workspaces[0].index = 9;
    vm.next_id = 901;
    TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_SUCCESS);
    TEST_CHECK(vm.workspaces[1].sid == 700, true);
    TEST_CHECK(vm.workspaces[1].id == 901, true);
    TEST_CHECK(vm.workspaces[1].index, 10);
    TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_SUCCESS);
    TEST_CHECK(vm.workspaces[2].sid == 80, true);
    TEST_CHECK(vm.workspaces[2].id == 902, true);
    TEST_CHECK(vm.workspaces[2].index, 11);
    struct virtual_workspace before[3];
    memcpy(before, vm.workspaces, sizeof(before));
    for (int i = 0; i < 3; ++i) {
        TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_NO_SPARE);
        TEST_CHECK(buf_len(vm.workspaces), 3);
        TEST_CHECK(memcmp(before, vm.workspaces, sizeof(before)), 0);
        TEST_CHECK(vm.next_id == 903, true);
    }
    TEST_CHECK(vm.workspaces[0].id == 900, true);
    TEST_CHECK(vm.workspaces[0].index, 9);
    TEST_CHECK(virtual_workspace_test_state.active_sid == 300, true);
    TEST_CHECK(virtual_workspace_test_state.focused_sid == 0, true);
    TEST_CHECK(virtual_workspace_test_state.visible_sid[0] == 300, true);
    buf_free(vm.workspaces);
});

TEST_FUNC(virtual_workspace_create_two_displays,
{
    virtual_workspace_test_reset();
    struct virtual_workspace_source source = virtual_workspace_test_source;
    source.occupancy = virtual_workspace_test_empty;
    struct virtual_workspace_manager vm = {0};
    TEST_CHECK(virtual_workspace_manager_begin(&vm, &source), true);
    TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_SUCCESS);
    TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_SUCCESS);
    TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_NO_SPARE);
    TEST_CHECK(buf_len(vm.workspaces), 3);
    TEST_CHECK(virtual_workspace_manager_find_space(&vm, 400) == NULL, true);
    virtual_workspace_test_state.active_did = 12;
    TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_SUCCESS);
    /* The visible, unmapped 500 is skipped on the second display. */
    TEST_CHECK(vm.workspaces[3].sid == 400, true);
    TEST_CHECK(vm.workspaces[3].did, 12);
    TEST_CHECK(vm.workspaces[3].index, 4);
    TEST_CHECK(vm.workspaces[3].id == 4, true);
    TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_NO_SPARE);
    TEST_CHECK(virtual_workspace_manager_begin(&vm, &source), true);
    TEST_CHECK(buf_len(vm.workspaces), 4);
    /* An empty allocation stays mapped and is addressable across displays. */
    TEST_CHECK(virtual_workspace_manager_focus(&vm, 4), VIRTUAL_WORKSPACE_SUCCESS);
    TEST_CHECK(virtual_workspace_test_state.focused_sid == 400, true);
    buf_free(vm.workspaces);
});

static enum virtual_workspace_occupancy virtual_workspace_test_create_occupancy(uint64_t sid)
{
    if (sid == 81) {
        struct window window = { .role = kAXWindowRole, .subrole = kAXFloatingWindowSubrole };
        window_set_flag(&window, WINDOW_FLOAT);
        return virtual_workspace_classify_window(&window, 3, 0);
    }
    /* Represents an unavailable/incomplete native query or uncertain AX role. */
    if (sid == 82) return VIRTUAL_WORKSPACE_UNKNOWN;
    return VIRTUAL_WORKSPACE_EMPTY;
}

TEST_FUNC(virtual_workspace_create_unsafe_candidates,
{
    virtual_workspace_test_reset();
    memcpy(virtual_workspace_test_state.spaces[0], ((uint64_t[]) { 300, 80, 81, 82, 84, 83 }), 6 * sizeof(uint64_t));
    virtual_workspace_test_state.space_count[0] = 6;
    virtual_workspace_test_state.active_sid = 500;
    struct virtual_workspace_source source = virtual_workspace_test_source;
    source.occupancy = virtual_workspace_test_empty;
    struct virtual_workspace_manager vm = {0};
    TEST_CHECK(virtual_workspace_manager_begin(&vm, &source), true);
    virtual_workspace_test_state.active_sid = 300;
    virtual_workspace_test_state.non_user_sid = 80;
    source.occupancy = virtual_workspace_test_create_occupancy;
    TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_SUCCESS);
    TEST_CHECK(vm.workspaces[1].sid == 84, true);
    TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_SUCCESS);
    TEST_CHECK(vm.workspaces[2].sid == 83, true);
    TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_NO_SPARE);
    TEST_CHECK(buf_len(vm.workspaces), 3);
    TEST_CHECK(virtual_workspace_manager_find_space(&vm, 300) == NULL, true);
    TEST_CHECK(virtual_workspace_manager_find_space(&vm, 80) == NULL, true);
    TEST_CHECK(virtual_workspace_manager_find_space(&vm, 81) == NULL, true);
    TEST_CHECK(virtual_workspace_manager_find_space(&vm, 82) == NULL, true);
    buf_free(vm.workspaces);
});

TEST_FUNC(virtual_workspace_create_unavailable_state,
{
    virtual_workspace_test_reset();
    struct virtual_workspace_manager vm = {0};
    TEST_CHECK(virtual_workspace_manager_begin(&vm, &virtual_workspace_test_source), true);
    struct virtual_workspace before[4];
    memcpy(before, vm.workspaces, sizeof(before));
    virtual_workspace_test_state.active_did = 0;
    TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_DISPLAY_UNAVAILABLE);
    virtual_workspace_test_state.active_did = 999;
    TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_DISPLAY_UNAVAILABLE);
    virtual_workspace_test_state.active_did = 90;
    virtual_workspace_test_state.missing_display = true;
    TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_DISPLAY_UNAVAILABLE);
    virtual_workspace_test_state.missing_display = false;
    virtual_workspace_test_state.missing_spaces = true;
    TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_STATE_UNAVAILABLE);
    virtual_workspace_test_state.missing_spaces = false;
    virtual_workspace_test_state.visible_sid[0] = 0;
    TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_STATE_UNAVAILABLE);
    virtual_workspace_test_state.visible_sid[0] = 999;
    TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_STATE_UNAVAILABLE);
    TEST_CHECK(buf_len(vm.workspaces), 4);
    TEST_CHECK(memcmp(before, vm.workspaces, sizeof(before)), 0);
    TEST_CHECK(vm.next_id == 5, true);
    buf_free(vm.workspaces);
});

TEST_FUNC(virtual_workspace_create_revalidation,
{
    /* Visibility, occupied/unknown windows, type, topology, focus, ownership,
       enumeration failure, and unavailable visibility can change after selection. */
    for (int change = 1; change <= 9; ++change) {
        virtual_workspace_test_reset();
        struct virtual_workspace_manager vm = {0};
        TEST_CHECK(virtual_workspace_manager_begin(&vm, &virtual_workspace_test_source), true);
        struct virtual_workspace before[4];
        memcpy(before, vm.workspaces, sizeof(before));
        virtual_workspace_test_state.space_queries = 0;
        virtual_workspace_test_state.change_on_query = 2;
        virtual_workspace_test_state.change = change;
        TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_STATE_CHANGED);
        TEST_CHECK(buf_len(vm.workspaces), 4);
        TEST_CHECK(memcmp(before, vm.workspaces, sizeof(before)), 0);
        TEST_CHECK(vm.next_id == 5, true);
        buf_free(vm.workspaces);
    }
});

TEST_FUNC(virtual_workspace_create_limits,
{
    virtual_workspace_test_reset();
    struct virtual_workspace_manager vm = {0};
    TEST_CHECK(virtual_workspace_manager_begin(&vm, &virtual_workspace_test_source), true);
    vm.workspaces[0].index = INT_MAX;
    TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_LIMIT);
    TEST_CHECK(vm.next_id == 5, true);
    vm.workspaces[0].index = 1;
    vm.next_id = 0;
    TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_LIMIT);
    TEST_CHECK(buf_len(vm.workspaces), 4);
    vm.next_id = UINT64_MAX;
    TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_SUCCESS);
    TEST_CHECK(vm.workspaces[4].id == UINT64_MAX, true);
    TEST_CHECK(vm.next_id == 0, true);
    buf_free(vm.workspaces);
});
