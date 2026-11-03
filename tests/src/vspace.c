static bool vspace_test_response(char *message, char *expected)
{
    char *response = NULL;
    size_t length = 0;
    FILE *rsp = open_memstream(&response, &length);
    if (!rsp) return false;

    handle_message(rsp, message);
    int status = fclose(rsp);
    bool result = status == 0 && string_equals(response, expected);
    if (!result) fprintf(stderr, "expected: %sreceived: %s", expected, response ? response : "(null)\n");
    free(response);
    return result;
}

TEST_FUNC(vspace_ping,
{
    bool matches = vspace_test_response("vspace\0--ping\0", "{\"status\":\"ok\",\"component\":\"vspace\"}\n");
    TEST_CHECK(matches, true);
});

TEST_FUNC(vspace_missing_command,
{
    bool matches = vspace_test_response("vspace\0", FAILURE_MESSAGE "unknown command '' for domain 'vspace'\n");
    TEST_CHECK(matches, true);
});

TEST_FUNC(vspace_unknown_command,
{
    bool matches = vspace_test_response("vspace\0--unknown\0", FAILURE_MESSAGE "unknown command '--unknown' for domain 'vspace'\n");
    TEST_CHECK(matches, true);
});

TEST_FUNC(vspace_extra_argument,
{
    bool matches = vspace_test_response("vspace\0--ping\0extra\0", FAILURE_MESSAGE "unexpected argument 'extra' for command '--ping' for domain 'vspace'\n");
    TEST_CHECK(matches, true);
});

static bool vspace_test_begin(void)
{
    virtual_workspace_test_reset();
    return virtual_workspace_manager_begin(&g_virtual_workspace_manager, &virtual_workspace_test_source);
}

static void vspace_test_end(void)
{
    buf_free(g_virtual_workspace_manager.workspaces);
    g_virtual_workspace_manager = (struct virtual_workspace_manager) {0};
}

static char *vspace_test_list =
    "[{\"index\":1,\"id\":1,\"space-id\":700,\"display-id\":90,\"has-focus\":false},"
    "{\"index\":2,\"id\":2,\"space-id\":300,\"display-id\":90,\"has-focus\":true},"
    "{\"index\":3,\"id\":3,\"space-id\":500,\"display-id\":12,\"has-focus\":false},"
    "{\"index\":4,\"id\":4,\"space-id\":400,\"display-id\":12,\"has-focus\":false}]\n";

TEST_FUNC(vspace_list_and_current,
{
    TEST_CHECK(vspace_test_begin(), true);
    TEST_CHECK(vspace_test_response("vspace\0--list\0", vspace_test_list), true);
    TEST_CHECK(vspace_test_response("vspace\0--current\0", "{\"index\":2,\"id\":2,\"space-id\":300,\"display-id\":90,\"has-focus\":true}\n"), true);
    virtual_workspace_test_state.active_sid = 80;
    TEST_CHECK(vspace_test_response("vspace\0--current\0", FAILURE_MESSAGE "current native space is not mapped to a virtual workspace.\n"), true);
    virtual_workspace_test_state.active_sid = 0;
    TEST_CHECK(vspace_test_response("vspace\0--current\0", FAILURE_MESSAGE "current native space is not mapped to a virtual workspace.\n"), true);
    vspace_test_end();
});

TEST_FUNC(vspace_uninitialized,
{
    TEST_CHECK(vspace_test_response("vspace\0--list\0", FAILURE_MESSAGE "virtual workspace mapping is not initialized.\n"), true);
    TEST_CHECK(vspace_test_response("vspace\0--current\0", FAILURE_MESSAGE "virtual workspace mapping is not initialized.\n"), true);
    TEST_CHECK(vspace_test_response("vspace\0--focus\0" "1\0", FAILURE_MESSAGE "virtual workspace mapping is not initialized.\n"), true);
    TEST_CHECK(vspace_test_response("vspace\0--ping\0", "{\"status\":\"ok\",\"component\":\"vspace\"}\n"), true);
});

static char *vspace_test_invalid_indices[] = {
    "", "0", "-1", "+1", "1.0", "1e0", "0x1", "prev", "1x", " 1", "1 ",
    "2147483648", "9999999999999999999999999999999999999999"
};

TEST_FUNC(vspace_invalid_indices,
{
    TEST_CHECK(vspace_test_begin(), true);
    for (int i = 0; i < array_count(vspace_test_invalid_indices); ++i) {
        char message[128] = "vspace\0--focus\0";
        char expected[256];
        size_t offset = sizeof("vspace\0--focus\0") - 1;
        snprintf(message + offset, sizeof(message) - offset, "%s", vspace_test_invalid_indices[i]);
        snprintf(expected, sizeof(expected), FAILURE_MESSAGE "value '%s' is not a valid virtual workspace index.\n", vspace_test_invalid_indices[i]);
        TEST_CHECK(vspace_test_response(message, expected), true);
    }
    TEST_CHECK(vspace_test_response("vspace\0--focus\0" "2147483647\0", FAILURE_MESSAGE "could not locate virtual workspace with index '2147483647'.\n"), true);
    TEST_CHECK(vspace_test_response("vspace\0--focus\0" "5\0", FAILURE_MESSAGE "could not locate virtual workspace with index '5'.\n"), true);
    TEST_CHECK(virtual_workspace_test_state.focused_sid == 0, true);
    vspace_test_end();
});

TEST_FUNC(vspace_reject_extra_arguments,
{
    TEST_CHECK(vspace_test_begin(), true);
    TEST_CHECK(vspace_test_response("vspace\0--list\0extra\0", FAILURE_MESSAGE "unexpected argument 'extra' for command '--list' for domain 'vspace'\n"), true);
    TEST_CHECK(vspace_test_response("vspace\0--current\0extra\0", FAILURE_MESSAGE "unexpected argument 'extra' for command '--current' for domain 'vspace'\n"), true);
    TEST_CHECK(vspace_test_response("vspace\0--focus\0" "1\0extra\0", FAILURE_MESSAGE "unexpected argument 'extra' for command '--focus' for domain 'vspace'\n"), true);
    TEST_CHECK(vspace_test_response("vspace\0--list\0--focus\0" "1\0", FAILURE_MESSAGE "unexpected argument '--focus' for command '--list' for domain 'vspace'\n"), true);
    TEST_CHECK(virtual_workspace_test_state.focused_sid == 0, true);
    vspace_test_end();
});

TEST_FUNC(vspace_focus_and_current,
{
    TEST_CHECK(vspace_test_begin(), true);
    TEST_CHECK(vspace_test_response("vspace\0--focus\0" "0003\0", ""), true);
    TEST_CHECK(virtual_workspace_test_state.focused_sid == 500, true);
    TEST_CHECK(vspace_test_response("vspace\0--current\0", "{\"index\":3,\"id\":3,\"space-id\":500,\"display-id\":12,\"has-focus\":true}\n"), true);
    TEST_CHECK(buf_len(g_virtual_workspace_manager.workspaces), 4);
    vspace_test_end();
});

TEST_FUNC(vspace_focus_errors,
{
    TEST_CHECK(vspace_test_begin(), true);
    virtual_workspace_test_state.focus_result = SPACE_OP_ERROR_SAME_SPACE;
    TEST_CHECK(vspace_test_response("vspace\0--focus\0" "1\0", FAILURE_MESSAGE "cannot focus an already focused virtual workspace.\n"), true);
    virtual_workspace_test_state.focus_result = SPACE_OP_ERROR_DISPLAY_IS_ANIMATING;
    TEST_CHECK(vspace_test_response("vspace\0--focus\0" "1\0", FAILURE_MESSAGE "cannot focus virtual workspace because the display is in the middle of an animation.\n"), true);
    virtual_workspace_test_state.focus_result = SPACE_OP_ERROR_IN_MISSION_CONTROL;
    TEST_CHECK(vspace_test_response("vspace\0--focus\0" "1\0", FAILURE_MESSAGE "cannot focus virtual workspace because mission-control is active.\n"), true);
    virtual_workspace_test_state.focus_result = SPACE_OP_ERROR_SCRIPTING_ADDITION;
    TEST_CHECK(vspace_test_response("vspace\0--focus\0" "1\0", FAILURE_MESSAGE "could not focus virtual workspace with index '1'.\n"), true);
    virtual_workspace_test_state.spaces[0][0] = 999;
    TEST_CHECK(vspace_test_response("vspace\0--focus\0" "1\0", FAILURE_MESSAGE "cannot focus virtual workspace because its native space or display is unavailable.\n"), true);
    vspace_test_end();
});

TEST_FUNC(vspace_64_bit_ids,
{
    TEST_CHECK(vspace_test_begin(), true);
    g_virtual_workspace_manager.workspaces[0].id = UINT64_MAX;
    g_virtual_workspace_manager.workspaces[0].sid = UINT64_MAX - 1;
    virtual_workspace_test_state.active_sid = UINT64_MAX - 1;
    TEST_CHECK(vspace_test_response("vspace\0--current\0", "{\"index\":1,\"id\":18446744073709551615,\"space-id\":18446744073709551614,\"display-id\":90,\"has-focus\":true}\n"), true);
    vspace_test_end();
});

TEST_FUNC(vspace_preserve_existing_dispatch,
{
    enum view_type original_layout = g_space_manager.layout;
    g_space_manager.layout = VIEW_BSP;
    TEST_CHECK(vspace_test_response("config\0layout\0", "bsp\n"), true);
    g_space_manager.layout = VIEW_STACK;
    TEST_CHECK(vspace_test_response("config\0layout\0", "stack\n"), true);
    g_space_manager.layout = VIEW_FLOAT;
    TEST_CHECK(vspace_test_response("config\0layout\0", "float\n"), true);
    TEST_CHECK(vspace_test_response("query\0--current\0", FAILURE_MESSAGE "unknown command '--current' for domain 'query'\n"), true);
    TEST_CHECK(vspace_test_response("vspace-extra\0--list\0", FAILURE_MESSAGE "unknown domain 'vspace-extra'\n"), true);
    g_space_manager.layout = original_layout;
});
