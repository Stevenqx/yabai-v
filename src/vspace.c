#include <limits.h>

extern struct virtual_workspace_manager g_virtual_workspace_manager;

#define DOMAIN_VSPACE           "vspace"
#define COMMAND_VSPACE_PING     "--ping"
#define COMMAND_VSPACE_LIST     "--list"
#define COMMAND_VSPACE_CURRENT  "--current"
#define COMMAND_VSPACE_FOCUS    "--focus"
#define COMMAND_VSPACE_CREATE   "--create"
#define COMMAND_VSPACE_DESTROY  "--destroy"
#define COMMAND_VSPACE_REFRESH  "--refresh"

bool vspace_response_is_deferred(FILE *rsp)
{
    struct virtual_workspace_destroy *operation = &g_virtual_workspace_manager.destroy;
    return operation->token && operation->context == rsp;
}

static const char *vspace_destroy_error(enum virtual_workspace_error result)
{
    switch (result) {
    case VIRTUAL_WORKSPACE_INVALID_INDEX: return "could not locate source virtual workspace";
    case VIRTUAL_WORKSPACE_BUSY: return "another virtual workspace lifecycle operation is pending; retry after completion";
    case VIRTUAL_WORKSPACE_LAST: return "cannot destroy the last virtual workspace";
    case VIRTUAL_WORKSPACE_INVALID_TARGET: return "target must be a different surviving workspace with a valid native desktop mapping";
    case VIRTUAL_WORKSPACE_LOCAL_TARGET_REQUIRED: return "no valid destination on the source display; specify --to TARGET";
    case VIRTUAL_WORKSPACE_LOCAL_REPLACEMENT_REQUIRED: return "a visible source requires a surviving mapped desktop on its display, even with --to";
    case VIRTUAL_WORKSPACE_SPACE_UNAVAILABLE: return "source native desktop or display is unavailable";
    case VIRTUAL_WORKSPACE_STATE_UNAVAILABLE: return "current native topology, visibility, or focus is unavailable";
    case VIRTUAL_WORKSPACE_STATE_CHANGED: return "native topology or mapping changed; retry after checking current state";
    case VIRTUAL_WORKSPACE_UNSAFE_WINDOWS: return "source occupancy is unsupported, unknown, or still occupied (including new arrivals)";
    case VIRTUAL_WORKSPACE_MOVE_FAILED: return "window evacuation or layout reconciliation could not be verified";
    case VIRTUAL_WORKSPACE_VISIBILITY_FAILED: return "native visibility or focus handoff could not be verified";
    case VIRTUAL_WORKSPACE_LIMIT: return "operation identities are exhausted";
    default: return "destroy could not be completed";
    }
}

static void vspace_destroy_complete(struct virtual_workspace_destroy *operation, enum virtual_workspace_error result)
{
    FILE *rsp = operation->context;
    if (result != VIRTUAL_WORKSPACE_SUCCESS) {
        daemon_fail(rsp, "destroy source %d (id %llu, space %llu) to target %d (id %llu, space %llu) failed: %s",
                    operation->source.index, (unsigned long long)operation->source.id, (unsigned long long)operation->source.sid,
                    operation->target.index, (unsigned long long)operation->target.id, (unsigned long long)operation->target.sid,
                    vspace_destroy_error(result));
        if (operation->cursor < buf_len(operation->windows)) fprintf(rsp, "; pending window: %u", operation->windows[operation->cursor].wid);
        fprintf(rsp, "; moved windows: [");
        bool separator = false;
        for (int i = 0; i < buf_len(operation->windows); ++i) {
            if (!operation->windows[i].moved) continue;
            fprintf(rsp, "%s%u", separator ? "," : "", operation->windows[i].wid);
            separator = true;
        }
        fprintf(rsp, "]; source mapping retained.\n");
    }
    fflush(rsp);
    fclose(rsp);
}

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

static const char *vspace_retention_reason(enum virtual_workspace_retention retention)
{
    switch (retention) {
    case VIRTUAL_WORKSPACE_RETAIN_FOCUSED: return "focused";
    case VIRTUAL_WORKSPACE_RETAIN_VISIBLE: return "visible";
    case VIRTUAL_WORKSPACE_RETAIN_OCCUPIED: return "occupied";
    case VIRTUAL_WORKSPACE_RETAIN_UNKNOWN: return "occupancy-unknown";
    case VIRTUAL_WORKSPACE_RETAIN_DISPLAY: return "display-unavailable";
    case VIRTUAL_WORKSPACE_RETAIN_MAPPING: return "mapping-unavailable";
    case VIRTUAL_WORKSPACE_RETAIN_LAST: return "last-workspace";
    default: return "unknown";
    }
}

static void vspace_refresh_serialize(FILE *rsp, struct virtual_workspace_refresh_entry *entries)
{
    bool separator = false;
    fprintf(rsp, "{\"released\":[");
    for (int i = 0; i < buf_len(entries); ++i) {
        struct virtual_workspace *workspace = &entries[i].workspace;
        if (entries[i].retention != VIRTUAL_WORKSPACE_RELEASE) continue;
        fprintf(rsp, "%s{\"index\":%d,\"id\":%llu,\"space-id\":%llu,\"display-id\":%u}",
                separator ? "," : "", workspace->index, (unsigned long long)workspace->id,
                (unsigned long long)workspace->sid, workspace->did);
        separator = true;
    }
    fprintf(rsp, "],\"reindexed\":[");
    separator = false;
    for (int i = 0; i < buf_len(entries); ++i) {
        struct virtual_workspace_refresh_entry *entry = &entries[i];
        if (!entry->new_index || entry->new_index == entry->workspace.index) continue;
        fprintf(rsp, "%s{\"id\":%llu,\"old-index\":%d,\"index\":%d}", separator ? "," : "",
                (unsigned long long)entry->workspace.id, entry->workspace.index, entry->new_index);
        separator = true;
    }
    fprintf(rsp, "],\"retained\":[");
    separator = false;
    for (int i = 0; i < buf_len(entries); ++i) {
        struct virtual_workspace_refresh_entry *entry = &entries[i];
        if (entry->retention == VIRTUAL_WORKSPACE_RELEASE) continue;
        fprintf(rsp, "%s{\"id\":%llu,\"index\":%d,\"reason\":\"%s\"}", separator ? "," : "",
                (unsigned long long)entry->workspace.id, entry->new_index, vspace_retention_reason(entry->retention));
        separator = true;
    }
    fprintf(rsp, "]}\n");
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
    bool destroy = token_equals(command, COMMAND_VSPACE_DESTROY);
    bool refresh = token_equals(command, COMMAND_VSPACE_REFRESH);
    if (!(ping || list || current || focus || create || destroy || refresh)) {
        daemon_fail(rsp, "unknown command '%.*s' for domain '%.*s'\n", command.length, command.text, domain.length, domain.text);
        return;
    }

    int index = 0;
    int target_index = 0;
    struct token argument = get_token(&message);
    if (focus || destroy) {
        if (!vspace_parse_index(argument, &index)) {
            daemon_fail(rsp, "value '%.*s' is not a valid virtual workspace index.\n", argument.length, argument.text);
            return;
        }
        argument = get_token(&message);
    }
    if (destroy && token_equals(argument, "--to")) {
        argument = get_token(&message);
        if (!vspace_parse_index(argument, &target_index)) {
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
        if (result == VIRTUAL_WORKSPACE_BUSY) {
            daemon_fail(rsp, "%s.\n", vspace_destroy_error(result));
        } else if (result == VIRTUAL_WORKSPACE_DISPLAY_UNAVAILABLE) {
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

    if (destroy) {
        enum virtual_workspace_error result = virtual_workspace_manager_destroy(vm, index, target_index, rsp, vspace_destroy_complete);
        if (result != VIRTUAL_WORKSPACE_PENDING) {
            daemon_fail(rsp, "cannot destroy virtual workspace with index '%d': %s.\n", index, vspace_destroy_error(result));
        }
        return;
    }

    if (refresh) {
        struct virtual_workspace_refresh_entry *entries = NULL;
        enum virtual_workspace_error result = virtual_workspace_manager_refresh(vm, &entries);
        if (result == VIRTUAL_WORKSPACE_BUSY) {
            daemon_fail(rsp, "%s.\n", vspace_destroy_error(result));
        } else if (result == VIRTUAL_WORKSPACE_STATE_UNAVAILABLE) {
            daemon_fail(rsp, "cannot refresh virtual workspaces because current native topology, visibility, or focus is unavailable; mapping unchanged.\n");
        } else if (result != VIRTUAL_WORKSPACE_SUCCESS) {
            daemon_fail(rsp, "cannot refresh virtual workspaces because native topology or mapping changed; retry after checking current state; mapping unchanged.\n");
        } else {
            vspace_refresh_serialize(rsp, entries);
        }
        buf_free(entries);
        return;
    }

    if (focus) {
        enum virtual_workspace_error result = virtual_workspace_manager_focus(vm, index);
        if (result == VIRTUAL_WORKSPACE_BUSY) {
            daemon_fail(rsp, "%s.\n", vspace_destroy_error(result));
        } else if (result == VIRTUAL_WORKSPACE_INVALID_INDEX) {
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
