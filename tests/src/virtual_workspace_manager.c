struct virtual_workspace_test_state
{
    uint32_t displays[2];
    uint64_t spaces[2][4];
    int space_count[2];
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
        .active_sid = 300,
        .focus_result = SPACE_OP_ERROR_SUCCESS,
        .focus_changes_space = true
    };
}

static uint32_t *virtual_workspace_test_displays(int *count)
{
    *count = 2;
    return virtual_workspace_test_state.missing_display ? NULL : virtual_workspace_test_state.displays;
}

static uint64_t *virtual_workspace_test_spaces(uint32_t did, int *count)
{
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

static uint32_t virtual_workspace_test_space_display(uint64_t sid)
{
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < virtual_workspace_test_state.space_count[i]; ++j) {
            if (virtual_workspace_test_state.spaces[i][j] == sid) return virtual_workspace_test_state.displays[i];
        }
    }
    return 0;
}

static enum virtual_workspace_occupancy virtual_workspace_test_occupancy(uint64_t sid)
{
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
    .space_display = virtual_workspace_test_space_display,
    .occupancy = virtual_workspace_test_occupancy,
    .focus_space = virtual_workspace_test_focus
};

static const uint64_t virtual_workspace_test_order[] = { 700, 300, 500, 400 };

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
    TEST_CHECK(virtual_workspace_classify_window(&window, 0, 3), VIRTUAL_WORKSPACE_OCCUPIED);
    window.subrole = kAXDialogSubrole;
    TEST_CHECK(virtual_workspace_classify_window(&window, 0, 8), VIRTUAL_WORKSPACE_OCCUPIED);
    window.subrole = NULL;
    TEST_CHECK(virtual_workspace_classify_window(&window, 0, 0), VIRTUAL_WORKSPACE_UNKNOWN);
    window.subrole = kAXUnknownSubrole;
    TEST_CHECK(virtual_workspace_classify_window(&window, 0, 0), VIRTUAL_WORKSPACE_EMPTY);
    TEST_CHECK(virtual_workspace_classify_window(NULL, 0, 0), VIRTUAL_WORKSPACE_UNKNOWN);
    TEST_CHECK(virtual_workspace_classify_window(NULL, 0, 3), VIRTUAL_WORKSPACE_UNKNOWN);
    TEST_CHECK(virtual_workspace_classify_window(NULL, 0, 8), VIRTUAL_WORKSPACE_UNKNOWN);
    TEST_CHECK(virtual_workspace_classify_window(NULL, 0, -2147483623), VIRTUAL_WORKSPACE_EMPTY);
    TEST_CHECK(virtual_workspace_classify_window(NULL, 123, 0), VIRTUAL_WORKSPACE_EMPTY);
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
    TEST_CHECK(virtual_workspace_manager_focus(&vm, 1), VIRTUAL_WORKSPACE_FOCUS_FAILED);
    virtual_workspace_test_state.focus_result = SPACE_OP_ERROR_SUCCESS;
    virtual_workspace_test_state.focus_changes_space = false;
    TEST_CHECK(virtual_workspace_manager_focus(&vm, 1), VIRTUAL_WORKSPACE_FOCUS_FAILED);
    TEST_CHECK(virtual_workspace_test_state.active_sid == 300, true);
    buf_free(vm.workspaces);
});
