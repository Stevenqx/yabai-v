extern struct window_manager g_window_manager;
extern int g_connection;

static enum virtual_workspace_occupancy virtual_workspace_classify_window(struct window *window, int level, uint64_t tags)
{
    if (window) {
        if (!window_role(window) || !window_subrole(window) || window_is_unknown(window)) return VIRTUAL_WORKSPACE_UNKNOWN;
        return window_is_real(window) ? VIRTUAL_WORKSPACE_OCCUPIED : VIRTUAL_WORKSPACE_EMPTY;
    }

    /* Exclude desktop and native nonordinary windows, as space_window_list does. */
    if (level <= CGWindowLevelForKey(kCGDesktopIconWindowLevelKey)) return VIRTUAL_WORKSPACE_EMPTY;
    if (!((tags & 0x1) || ((tags & 0x2) && (tags & 0x80000000)))) return VIRTUAL_WORKSPACE_EMPTY;
    return VIRTUAL_WORKSPACE_UNKNOWN;
}

static enum virtual_workspace_occupancy virtual_workspace_native_occupancy(uint64_t sid)
{
    enum virtual_workspace_occupancy result = VIRTUAL_WORKSPACE_UNKNOWN;
    uint64_t set_tags = 0;
    uint64_t clear_tags = 0;
    CFTypeRef query = NULL;
    CFTypeRef iterator = NULL;
    CFArrayRef spaces = cfarray_of_cfnumbers(&sid, sizeof(sid), 1, kCFNumberSInt64Type);
    if (!spaces) return result;

    /* Include minimized/hidden windows, independently of tiling and AX tracking. */
    CFArrayRef windows = SLSCopyWindowsWithOptionsAndTags(g_connection, 0, spaces, 0x7, &set_tags, &clear_tags);
    if (!windows) goto out;

    int count = CFArrayGetCount(windows);
    if (!count) {
        result = VIRTUAL_WORKSPACE_EMPTY;
        goto out;
    }

    query = SLSWindowQueryWindows(g_connection, windows, count);
    if (!query) goto out;
    iterator = SLSWindowQueryResultCopyWindows(query);
    if (!iterator) goto out;

    result = VIRTUAL_WORKSPACE_EMPTY;
    int seen = 0;
    while (SLSWindowIteratorAdvance(iterator)) {
        ++seen;
        uint32_t wid = SLSWindowIteratorGetWindowID(iterator);
        struct window *window = window_manager_find_window(&g_window_manager, wid);
        enum virtual_workspace_occupancy occupancy = virtual_workspace_classify_window(window, SLSWindowIteratorGetLevel(iterator), SLSWindowIteratorGetTags(iterator));
        if (occupancy == VIRTUAL_WORKSPACE_OCCUPIED) {
            result = occupancy;
            goto out;
        }
        if (occupancy == VIRTUAL_WORKSPACE_UNKNOWN) result = occupancy;
    }
    if (seen != count) result = VIRTUAL_WORKSPACE_UNKNOWN;

out:
    if (iterator) CFRelease(iterator);
    if (query) CFRelease(query);
    if (windows) CFRelease(windows);
    CFRelease(spaces);
    return result;
}

static const struct virtual_workspace_source virtual_workspace_native_source = {
    .display_list = display_manager_active_display_list,
    .space_list = display_space_list,
    .active_space = space_manager_active_space,
    .space_display = space_display_id,
    .occupancy = virtual_workspace_native_occupancy,
    .focus_space = space_manager_focus_space
};

struct virtual_workspace *virtual_workspace_manager_find_index(struct virtual_workspace_manager *vm, int index)
{
    if (index <= 0) return NULL;
    for (int i = 0; i < buf_len(vm->workspaces); ++i) {
        if (vm->workspaces[i].index == index) return &vm->workspaces[i];
    }
    return NULL;
}

struct virtual_workspace *virtual_workspace_manager_find_space(struct virtual_workspace_manager *vm, uint64_t sid)
{
    for (int i = 0; i < buf_len(vm->workspaces); ++i) {
        if (vm->workspaces[i].sid == sid) return &vm->workspaces[i];
    }
    return NULL;
}

bool virtual_workspace_manager_begin(struct virtual_workspace_manager *vm, const struct virtual_workspace_source *source)
{
    if (vm->did_begin) return true;
    if (!source) source = &virtual_workspace_native_source;

    struct virtual_workspace_manager pending = { .source = source };
    uint64_t active_sid = source->active_space();
    int display_count = 0;
    uint32_t *displays = source->display_list(&display_count);
    if (!active_sid || !displays || display_count <= 0) goto err;

    uint64_t next_id = 1;
    for (int i = 0; i < display_count; ++i) {
        int space_count = 0;
        uint64_t *spaces = source->space_list(displays[i], &space_count);
        if (!displays[i] || !spaces || space_count <= 0) goto err;

        for (int j = 0; j < space_count; ++j) {
            if (!spaces[j]) goto err;
            struct virtual_workspace *existing = virtual_workspace_manager_find_space(&pending, spaces[j]);
            if (existing) {
                if (existing->did != displays[i]) goto err;
                continue;
            }
            if (spaces[j] != active_sid && source->occupancy(spaces[j]) == VIRTUAL_WORKSPACE_EMPTY) continue;

            buf_push(pending.workspaces, ((struct virtual_workspace) {
                .id = next_id++,
                .sid = spaces[j],
                .did = displays[i],
                .index = buf_len(pending.workspaces) + 1
            }));
        }
    }

    /* Do not expose a partial snapshot if native enumeration lost the current Space. */
    if (!virtual_workspace_manager_find_space(&pending, active_sid)) goto err;
    pending.did_begin = true;
    *vm = pending;
    return true;

err:
    buf_free(pending.workspaces);
    return false;
}

enum virtual_workspace_error virtual_workspace_manager_focus(struct virtual_workspace_manager *vm, int index)
{
    if (!vm->did_begin) return VIRTUAL_WORKSPACE_NOT_INITIALIZED;
    struct virtual_workspace *workspace = virtual_workspace_manager_find_index(vm, index);
    if (!workspace) return VIRTUAL_WORKSPACE_INVALID_INDEX;
    if (vm->source->space_display(workspace->sid) != workspace->did) return VIRTUAL_WORKSPACE_SPACE_UNAVAILABLE;

    enum space_op_error result = vm->source->focus_space(workspace->sid);
    if (result == SPACE_OP_ERROR_SAME_SPACE) return VIRTUAL_WORKSPACE_SAME_SPACE;
    if (result == SPACE_OP_ERROR_DISPLAY_IS_ANIMATING) return VIRTUAL_WORKSPACE_DISPLAY_IS_ANIMATING;
    if (result == SPACE_OP_ERROR_IN_MISSION_CONTROL) return VIRTUAL_WORKSPACE_IN_MISSION_CONTROL;
    if (result == SPACE_OP_ERROR_SCRIPTING_ADDITION) return VIRTUAL_WORKSPACE_SCRIPTING_ADDITION;
    if (result != SPACE_OP_ERROR_SUCCESS) return VIRTUAL_WORKSPACE_FOCUS_FAILED;
    return VIRTUAL_WORKSPACE_SUCCESS;
}
