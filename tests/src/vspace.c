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
    "[{\"index\":1,\"id\":1,\"space-id\":700,\"display-id\":90,\"has-focus\":false,\"availability\":\"available\",\"persistence\":\"session-only\"},"
    "{\"index\":2,\"id\":2,\"space-id\":300,\"display-id\":90,\"has-focus\":true,\"availability\":\"available\",\"persistence\":\"session-only\"},"
    "{\"index\":3,\"id\":3,\"space-id\":500,\"display-id\":12,\"has-focus\":false,\"availability\":\"available\",\"persistence\":\"session-only\"},"
    "{\"index\":4,\"id\":4,\"space-id\":400,\"display-id\":12,\"has-focus\":false,\"availability\":\"available\",\"persistence\":\"session-only\"}]\n";

TEST_FUNC(vspace_list_and_current,
{
    TEST_CHECK(vspace_test_begin(), true);
    TEST_CHECK(vspace_test_response("vspace\0--list\0", vspace_test_list), true);
    TEST_CHECK(vspace_test_response("vspace\0--current\0", "{\"index\":2,\"id\":2,\"space-id\":300,\"display-id\":90,\"has-focus\":true,\"availability\":\"available\",\"persistence\":\"session-only\"}\n"), true);
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
    TEST_CHECK(vspace_test_response("vspace\0--create\0", FAILURE_MESSAGE "virtual workspace mapping is not initialized.\n"), true);
    TEST_CHECK(vspace_test_response("vspace\0--refresh\0", FAILURE_MESSAGE "virtual workspace mapping is not initialized.\n"), true);
    TEST_CHECK(vspace_test_response("vspace\0--destroy\0" "1\0", FAILURE_MESSAGE "virtual workspace mapping is not initialized.\n"), true);
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
    TEST_CHECK(vspace_test_response("vspace\0--create\0extra\0", FAILURE_MESSAGE "unexpected argument 'extra' for command '--create' for domain 'vspace'\n"), true);
    TEST_CHECK(vspace_test_response("vspace\0--refresh\0extra\0", FAILURE_MESSAGE "unexpected argument 'extra' for command '--refresh' for domain 'vspace'\n"), true);
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
    TEST_CHECK(vspace_test_response("vspace\0--current\0", "{\"index\":3,\"id\":3,\"space-id\":500,\"display-id\":12,\"has-focus\":true,\"availability\":\"available\",\"persistence\":\"session-only\"}\n"), true);
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
    TEST_CHECK(vspace_test_response("vspace\0--focus\0" "1\0", FAILURE_MESSAGE "cannot focus virtual workspace due to an error with the scripting-addition.\n"), true);
    virtual_workspace_test_state.focus_result = SPACE_OP_ERROR_SUCCESS;
    virtual_workspace_test_state.focus_changes_space = false;
    TEST_CHECK(vspace_test_response("vspace\0--focus\0" "1\0", ""), true);
    TEST_CHECK(virtual_workspace_test_state.active_sid == 300, true);
    virtual_workspace_test_state.spaces[0][0] = 999;
    TEST_CHECK(vspace_test_response("vspace\0--focus\0" "1\0", FAILURE_MESSAGE "cannot focus virtual workspace because its native space or display is unavailable or unresolved.\n"), true);
    vspace_test_end();
});

TEST_FUNC(vspace_64_bit_ids,
{
    TEST_CHECK(vspace_test_begin(), true);
    g_virtual_workspace_manager.workspaces[0].id = UINT64_MAX;
    g_virtual_workspace_manager.workspaces[0].sid = UINT64_MAX - 1;
    virtual_workspace_test_state.spaces[0][0] = UINT64_MAX - 1;
    virtual_workspace_test_state.active_sid = UINT64_MAX - 1;
    TEST_CHECK(vspace_test_response("vspace\0--current\0", "{\"index\":1,\"id\":18446744073709551615,\"space-id\":18446744073709551614,\"display-id\":90,\"has-focus\":true,\"availability\":\"available\",\"persistence\":\"session-only\"}\n"), true);
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

TEST_FUNC(vspace_create,
{
    TEST_CHECK(vspace_test_begin(), true);
    TEST_CHECK(vspace_test_response("vspace\0--create\0", ""), true);
    TEST_CHECK(buf_len(g_virtual_workspace_manager.workspaces), 5);
    TEST_CHECK(vspace_test_response("vspace\0--list\0", "[{\"index\":1,\"id\":1,\"space-id\":700,\"display-id\":90,\"has-focus\":false,\"availability\":\"available\",\"persistence\":\"session-only\"},{\"index\":2,\"id\":2,\"space-id\":300,\"display-id\":90,\"has-focus\":true,\"availability\":\"available\",\"persistence\":\"session-only\"},{\"index\":3,\"id\":3,\"space-id\":500,\"display-id\":12,\"has-focus\":false,\"availability\":\"available\",\"persistence\":\"session-only\"},{\"index\":4,\"id\":4,\"space-id\":400,\"display-id\":12,\"has-focus\":false,\"availability\":\"available\",\"persistence\":\"session-only\"},{\"index\":5,\"id\":5,\"space-id\":80,\"display-id\":90,\"has-focus\":false,\"availability\":\"available\",\"persistence\":\"session-only\"}]\n"), true);
    TEST_CHECK(vspace_test_response("vspace\0--current\0", "{\"index\":2,\"id\":2,\"space-id\":300,\"display-id\":90,\"has-focus\":true,\"availability\":\"available\",\"persistence\":\"session-only\"}\n"), true);
    TEST_CHECK(vspace_test_response("vspace\0--create\0", FAILURE_MESSAGE "no eligible spare native desktop space on the current display.\n"), true);
    TEST_CHECK(vspace_test_response("vspace\0--focus\0" "5\0", ""), true);
    TEST_CHECK(virtual_workspace_test_state.focused_sid == 80, true);
    TEST_CHECK(vspace_test_response("vspace\0--current\0", "{\"index\":5,\"id\":5,\"space-id\":80,\"display-id\":90,\"has-focus\":true,\"availability\":\"available\",\"persistence\":\"session-only\"}\n"), true);
    virtual_workspace_test_state.active_did = 0;
    TEST_CHECK(vspace_test_response("vspace\0--create\0", FAILURE_MESSAGE "cannot create virtual workspace because the current display could not be resolved.\n"), true);
    virtual_workspace_test_state.active_did = 90;
    virtual_workspace_test_state.visible_sid[0] = 0;
    TEST_CHECK(vspace_test_response("vspace\0--create\0", FAILURE_MESSAGE "cannot create virtual workspace because current native space state is unavailable.\n"), true);
    TEST_CHECK(buf_len(g_virtual_workspace_manager.workspaces), 5);
    vspace_test_end();
});

TEST_FUNC(vspace_create_revalidation_errors,
{
    TEST_CHECK(vspace_test_begin(), true);
    virtual_workspace_test_state.space_queries = 0;
    virtual_workspace_test_state.change_on_query = 2;
    virtual_workspace_test_state.change = 2;
    TEST_CHECK(vspace_test_response("vspace\0--create\0", FAILURE_MESSAGE "cannot create virtual workspace because native state changed; retry the command.\n"), true);
    TEST_CHECK(buf_len(g_virtual_workspace_manager.workspaces), 4);
    virtual_workspace_test_state.occupied_sid = 0;
    g_virtual_workspace_manager.workspaces[0].index = INT_MAX;
    TEST_CHECK(vspace_test_response("vspace\0--create\0", FAILURE_MESSAGE "cannot create virtual workspace because virtual IDs or indices are exhausted.\n"), true);
    TEST_CHECK(buf_len(g_virtual_workspace_manager.workspaces), 4);
    TEST_CHECK(g_virtual_workspace_manager.next_id == 5, true);
    vspace_test_end();
});

TEST_FUNC(vspace_destroy_parsing,
{
    TEST_CHECK(vspace_test_begin(), true);
    for (int i = 0; i < array_count(vspace_test_invalid_indices); ++i) {
        char message[128] = "vspace\0--destroy\0";
        char expected[256];
        snprintf(message + sizeof("vspace\0--destroy\0") - 1, 100, "%s", vspace_test_invalid_indices[i]);
        snprintf(expected, sizeof(expected), FAILURE_MESSAGE "value '%s' is not a valid virtual workspace index.\n", vspace_test_invalid_indices[i]);
        TEST_CHECK(vspace_test_response(message, expected), true);
    }
    TEST_CHECK(vspace_test_response("vspace\0--destroy\0" "1\0--to\0", FAILURE_MESSAGE "value '' is not a valid virtual workspace index.\n"), true);
    TEST_CHECK(vspace_test_response("vspace\0--destroy\0" "1\0--to\0-2\0", FAILURE_MESSAGE "value '-2' is not a valid virtual workspace index.\n"), true);
    TEST_CHECK(vspace_test_response("vspace\0--destroy\0" "1\0extra\0", FAILURE_MESSAGE "unexpected argument 'extra' for command '--destroy' for domain 'vspace'\n"), true);
    TEST_CHECK(vspace_test_response("vspace\0--destroy\0" "1\0--to\0" "2\0extra\0", FAILURE_MESSAGE "unexpected argument 'extra' for command '--destroy' for domain 'vspace'\n"), true);
    TEST_CHECK(buf_len(g_virtual_workspace_manager.workspaces), 4);
    vspace_test_end();
});

TEST_FUNC(vspace_destroy_deferred_ipc,
{
    for (int fail = 0; fail < 2; ++fail) {
        TEST_CHECK(virtual_workspace_destroy_test_begin(&g_virtual_workspace_manager), true);
        virtual_workspace_destroy_test_add(10, 700);
        virtual_workspace_destroy_test_add(11, 700);
        if (fail) virtual_workspace_destroy_test.fail_wid = 11;
        int sockets[2];
        TEST_CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);
        char message[] = "vspace\0--destroy\0" "0001\0--to\0" "0002\0";
        int length = sizeof(message);
        ssize_t written = write(sockets[0], &length, sizeof(length));
        TEST_CHECK(written == sizeof(length), true);
        written = write(sockets[0], message, length);
        TEST_CHECK(written == length, true);
        /* DAEMON_MESSAGE uses temporary storage, exactly as the real event loop. */
        g_temp_storage.memory = malloc(4096);
        g_temp_storage.size = 4096;
        EVENT_HANDLER_DAEMON_MESSAGE(NULL, sockets[1]);
        TEST_CHECK(g_virtual_workspace_manager.destroy.token != 0, true);
        ts_reset(); /* The operation must retain no command/temporary pointers. */
        struct pollfd descriptor = ((struct pollfd) { .fd = sockets[0], .events = POLLIN });
        TEST_CHECK(poll(&descriptor, 1, 0), 0);
        TEST_CHECK(vspace_test_response("vspace\0--ping\0", "{\"status\":\"ok\",\"component\":\"vspace\"}\n"), true);
        TEST_CHECK(vspace_test_response("vspace\0--create\0", FAILURE_MESSAGE "another virtual workspace lifecycle operation is pending; retry after completion.\n"), true);
        TEST_CHECK(vspace_test_response("vspace\0--refresh\0", FAILURE_MESSAGE "another virtual workspace lifecycle operation is pending; retry after completion.\n"), true);
        TEST_CHECK(vspace_test_response("vspace\0--focus\0" "3\0", FAILURE_MESSAGE "another virtual workspace lifecycle operation is pending; retry after completion.\n"), true);
        virtual_workspace_destroy_test_drain(&g_virtual_workspace_manager);
        char response[1024] = {0};
        ssize_t received = read(sockets[0], response, sizeof(response)-1);
        if (fail) {
            TEST_CHECK(received > 0 && response[0] == FAILURE_MESSAGE[0], true);
            TEST_CHECK(strstr(response, "source 1 (id 1, space 700) to target 2 (id 2, space 300)") != NULL, true);
            TEST_CHECK(strstr(response, "moved windows: [10]; source mapping retained.") != NULL, true);
        } else {
            TEST_CHECK(received == 0, true);
        }
        TEST_CHECK(buf_len(g_virtual_workspace_manager.workspaces), fail ? 4 : 3);
        close(sockets[0]);
        free(g_temp_storage.memory);
        memset(&g_temp_storage, 0, sizeof(g_temp_storage));
        vspace_test_end();
    }
});

TEST_FUNC(vspace_refresh_result_and_errors,
{
    virtual_workspace_refresh_test_begin(&g_virtual_workspace_manager);
    TEST_CHECK(vspace_test_response("vspace\0--refresh\0", "{\"released\":[{\"index\":2,\"id\":202,\"space-id\":400,\"display-id\":12},{\"index\":3,\"id\":303,\"space-id\":80,\"display-id\":90}],\"reindexed\":[{\"id\":404,\"old-index\":4,\"index\":2},{\"id\":505,\"old-index\":5,\"index\":3},{\"id\":606,\"old-index\":6,\"index\":4}],\"retained\":[{\"id\":101,\"index\":1,\"reason\":\"focused\"},{\"id\":404,\"index\":2,\"reason\":\"visible\"},{\"id\":505,\"index\":3,\"reason\":\"occupied\"},{\"id\":606,\"index\":4,\"reason\":\"occupied\"}]}\n"), true);
    TEST_CHECK(vspace_test_response("vspace\0--current\0", "{\"index\":1,\"id\":101,\"space-id\":300,\"display-id\":90,\"has-focus\":true,\"availability\":\"available\",\"persistence\":\"session-only\"}\n"), true);
    virtual_workspace_test_state.missing_spaces = true;
    TEST_CHECK(vspace_test_response("vspace\0--refresh\0", FAILURE_MESSAGE "cannot refresh virtual workspaces because current native topology, visibility, or focus is unavailable; mapping unchanged.\n"), true);
    virtual_workspace_test_state.missing_spaces = false;
    virtual_workspace_test_state.space_queries = 0;
    virtual_workspace_refresh_test_query = 3;
    virtual_workspace_refresh_test_change = 7;
    TEST_CHECK(vspace_test_response("vspace\0--refresh\0", FAILURE_MESSAGE "cannot refresh virtual workspaces because native topology or mapping changed; retry after checking current state; mapping unchanged.\n"), true);
    TEST_CHECK(buf_len(g_virtual_workspace_manager.workspaces), 4);
    vspace_test_end();

    TEST_CHECK(vspace_test_begin(), true);
    TEST_CHECK(vspace_test_response("vspace\0--refresh\0", "{\"released\":[],\"reindexed\":[],\"retained\":[{\"id\":1,\"index\":1,\"reason\":\"occupied\"},{\"id\":2,\"index\":2,\"reason\":\"focused\"},{\"id\":3,\"index\":3,\"reason\":\"visible\"},{\"id\":4,\"index\":4,\"reason\":\"occupancy-unknown\"}]}\n"), true);
    TEST_CHECK(vspace_test_response("vspace\0--list\0", vspace_test_list), true);
    vspace_test_end();
});
