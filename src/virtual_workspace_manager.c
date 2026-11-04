#include <limits.h>

extern struct window_manager g_window_manager;
extern int g_connection;

static enum virtual_workspace_occupancy virtual_workspace_classify_window(struct window *window, int level, uint64_t tags)
{
    if (window) {
        CFStringRef role = window_role(window);
        if (!role || !window_subrole(window) || window_is_unknown(window)) return VIRTUAL_WORKSPACE_UNKNOWN;
        if (window_is_real(window)) return VIRTUAL_WORKSPACE_OCCUPIED;
        if (CFEqual(role, kAXMenuRole) || CFEqual(role, kAXMenuBarRole)) return VIRTUAL_WORKSPACE_EMPTY;
        return VIRTUAL_WORKSPACE_UNKNOWN;
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

static uint32_t virtual_workspace_native_active_display(void)
{
    /* Use yabai's focused-display lookup, but fail instead of asserting on NULL. */
    CFStringRef uuid = display_manager_active_display_uuid();
    if (!uuid) return 0;
    uint32_t did = display_id(uuid);
    CFRelease(uuid);
    return did;
}

static const struct virtual_workspace_source virtual_workspace_native_source = {
    .display_list = display_manager_active_display_list,
    .space_list = display_space_list,
    .active_space = space_manager_active_space,
    .active_display = virtual_workspace_native_active_display,
    .visible_space = display_space_id,
    .is_user_space = space_is_user,
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
    pending.next_id = next_id;
    *vm = pending;
    return true;

err:
    buf_free(pending.workspaces);
    return false;
}

static enum virtual_workspace_error virtual_workspace_manager_find_spare(struct virtual_workspace_manager *vm, uint32_t did, uint64_t *sid)
{
    const struct virtual_workspace_source *source = vm->source;
    int display_count = 0;
    uint32_t *displays = source->display_list(&display_count);
    if (!displays || display_count <= 0) return VIRTUAL_WORKSPACE_DISPLAY_UNAVAILABLE;
    bool connected = false;
    for (int i = 0; i < display_count; ++i) {
        if (displays[i] == did) connected = true;
    }
    if (!connected) return VIRTUAL_WORKSPACE_DISPLAY_UNAVAILABLE;

    int space_count = 0;
    uint64_t *spaces = source->space_list(did, &space_count);
    if (!spaces || space_count <= 0) return VIRTUAL_WORKSPACE_STATE_UNAVAILABLE;
    uint64_t visible_sid = source->visible_space(did);
    bool visible_found = false;
    for (int i = 0; i < space_count; ++i) {
        if (!spaces[i] || source->space_display(spaces[i]) != did) return VIRTUAL_WORKSPACE_STATE_UNAVAILABLE;
        if (spaces[i] == visible_sid) visible_found = true;
    }
    if (!visible_sid || !visible_found) return VIRTUAL_WORKSPACE_STATE_UNAVAILABLE;

    for (int i = 0; i < space_count; ++i) {
        if (spaces[i] == visible_sid || virtual_workspace_manager_find_space(vm, spaces[i])) continue;
        if (!source->is_user_space(spaces[i])) continue;
        if (source->occupancy(spaces[i]) != VIRTUAL_WORKSPACE_EMPTY) continue;
        *sid = spaces[i];
        return VIRTUAL_WORKSPACE_SUCCESS;
    }
    return VIRTUAL_WORKSPACE_NO_SPARE;
}

enum virtual_workspace_error virtual_workspace_manager_create(struct virtual_workspace_manager *vm)
{
    if (!vm->did_begin) return VIRTUAL_WORKSPACE_NOT_INITIALIZED;
    uint32_t did = vm->source->active_display();
    if (!did) return VIRTUAL_WORKSPACE_DISPLAY_UNAVAILABLE;

    uint64_t sid = 0;
    enum virtual_workspace_error result = virtual_workspace_manager_find_spare(vm, did, &sid);
    if (result != VIRTUAL_WORKSPACE_SUCCESS) return result;

    int max_index = 0;
    for (int i = 0; i < buf_len(vm->workspaces); ++i) {
        if (vm->workspaces[i].index > max_index) max_index = vm->workspaces[i].index;
    }
    if (!vm->next_id || max_index == INT_MAX) return VIRTUAL_WORKSPACE_LIMIT;

    /* Commands and events share the daemon loop. Requery macOS before mutation. */
    uint64_t confirmed_sid = 0;
    result = virtual_workspace_manager_find_spare(vm, did, &confirmed_sid);
    if (result != VIRTUAL_WORKSPACE_SUCCESS || confirmed_sid != sid || vm->source->active_display() != did) {
        return VIRTUAL_WORKSPACE_STATE_CHANGED;
    }

    buf_push(vm->workspaces, ((struct virtual_workspace) {
        .id = vm->next_id++,
        .sid = sid,
        .did = did,
        .index = max_index + 1
    }));
    return VIRTUAL_WORKSPACE_SUCCESS;
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
