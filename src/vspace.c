#include <limits.h>

extern struct virtual_workspace_manager g_virtual_workspace_manager;

#define DOMAIN_VSPACE           "vspace"
#define COMMAND_VSPACE_PING     "--ping"
#define COMMAND_VSPACE_LIST     "--list"
#define COMMAND_VSPACE_CURRENT  "--current"
#define COMMAND_VSPACE_FOCUS    "--focus"
#define COMMAND_VSPACE_CREATE   "--create"

static bool vspace_parse_index(struct token token, int *index)
{
    *index = 0;
    if (!token_is_valid(token)) return false;

    for (int i = 0; i < token.length; ++i) {
        char c = token.text[i];
        if (c < '0' || c > '9') return false;
        int digit = c - '0';
        if (*index > (INT_MAX - digit) / 10) return false;
        *index = *index * 10 + digit;
    }
    return *index > 0;
}

static void vspace_serialize(FILE *rsp, struct virtual_workspace *workspace, uint64_t active_sid)
{
    fprintf(rsp, "{\"index\":%d,\"id\":%llu,\"space-id\":%llu,\"display-id\":%u,\"has-focus\":%s}",
            workspace->index, (unsigned long long)workspace->id, (unsigned long long)workspace->sid,
            workspace->did, json_bool(workspace->sid == active_sid));
}

static void handle_domain_vspace(FILE *rsp, struct token domain, char *message)
{
    TIME_FUNCTION;

    struct token command = get_token(&message);
    bool ping = token_equals(command, COMMAND_VSPACE_PING);
    bool list = token_equals(command, COMMAND_VSPACE_LIST);
    bool current = token_equals(command, COMMAND_VSPACE_CURRENT);
    bool focus = token_equals(command, COMMAND_VSPACE_FOCUS);
    bool create = token_equals(command, COMMAND_VSPACE_CREATE);
    if (!(ping || list || current || focus || create)) {
        daemon_fail(rsp, "unknown command '%.*s' for domain '%.*s'\n", command.length, command.text, domain.length, domain.text);
        return;
    }

    int index = 0;
    struct token argument = get_token(&message);
    if (focus) {
        if (!vspace_parse_index(argument, &index)) {
            daemon_fail(rsp, "value '%.*s' is not a valid virtual workspace index.\n", argument.length, argument.text);
            return;
        }
        argument = get_token(&message);
    }
    if (token_is_valid(argument)) {
        daemon_fail(rsp, "unexpected argument '%.*s' for command '%.*s' for domain '%.*s'\n", argument.length, argument.text, command.length, command.text, domain.length, domain.text);
        return;
    }

    if (ping) {
        fprintf(rsp, "{\"status\":\"ok\",\"component\":\"vspace\"}\n");
        return;
    }

    struct virtual_workspace_manager *vm = &g_virtual_workspace_manager;
    if (!vm->did_begin) {
        daemon_fail(rsp, "virtual workspace mapping is not initialized.\n");
        return;
    }

    if (create) {
        enum virtual_workspace_error result = virtual_workspace_manager_create(vm);
        if (result == VIRTUAL_WORKSPACE_DISPLAY_UNAVAILABLE) {
            daemon_fail(rsp, "cannot create virtual workspace because the current display could not be resolved.\n");
        } else if (result == VIRTUAL_WORKSPACE_STATE_UNAVAILABLE) {
            daemon_fail(rsp, "cannot create virtual workspace because current native space state is unavailable.\n");
        } else if (result == VIRTUAL_WORKSPACE_NO_SPARE) {
            daemon_fail(rsp, "no eligible spare native desktop space on the current display.\n");
        } else if (result == VIRTUAL_WORKSPACE_STATE_CHANGED) {
            daemon_fail(rsp, "cannot create virtual workspace because native state changed; retry the command.\n");
        } else if (result == VIRTUAL_WORKSPACE_LIMIT) {
            daemon_fail(rsp, "cannot create virtual workspace because virtual IDs or indices are exhausted.\n");
        } else if (result != VIRTUAL_WORKSPACE_SUCCESS) {
            daemon_fail(rsp, "could not create virtual workspace.\n");
        }
        return;
    }

    if (focus) {
        enum virtual_workspace_error result = virtual_workspace_manager_focus(vm, index);
        if (result == VIRTUAL_WORKSPACE_INVALID_INDEX) {
            daemon_fail(rsp, "could not locate virtual workspace with index '%d'.\n", index);
        } else if (result == VIRTUAL_WORKSPACE_SPACE_UNAVAILABLE) {
            daemon_fail(rsp, "cannot focus virtual workspace because its native space or display is unavailable.\n");
        } else if (result == VIRTUAL_WORKSPACE_SAME_SPACE) {
            daemon_fail(rsp, "cannot focus an already focused virtual workspace.\n");
        } else if (result == VIRTUAL_WORKSPACE_DISPLAY_IS_ANIMATING) {
            daemon_fail(rsp, "cannot focus virtual workspace because the display is in the middle of an animation.\n");
        } else if (result == VIRTUAL_WORKSPACE_IN_MISSION_CONTROL) {
            daemon_fail(rsp, "cannot focus virtual workspace because mission-control is active.\n");
        } else if (result == VIRTUAL_WORKSPACE_SCRIPTING_ADDITION) {
            daemon_fail(rsp, "cannot focus virtual workspace due to an error with the scripting-addition.\n");
        } else if (result != VIRTUAL_WORKSPACE_SUCCESS) {
            daemon_fail(rsp, "could not focus virtual workspace with index '%d'.\n", index);
        }
        return;
    }

    uint64_t active_sid = vm->source->active_space();
    if (list) {
        fprintf(rsp, "[");
        for (int i = 0; i < buf_len(vm->workspaces); ++i) {
            if (i) fprintf(rsp, ",");
            vspace_serialize(rsp, &vm->workspaces[i], active_sid);
        }
        fprintf(rsp, "]\n");
    } else {
        struct virtual_workspace *workspace = virtual_workspace_manager_find_space(vm, active_sid);
        if (!workspace) {
            daemon_fail(rsp, "current native space is not mapped to a virtual workspace.\n");
            return;
        }
        vspace_serialize(rsp, workspace, active_sid);
        fprintf(rsp, "\n");
    }
}
