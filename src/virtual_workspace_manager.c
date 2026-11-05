#include <limits.h>

extern struct window_manager g_window_manager;
extern struct space_manager g_space_manager;
extern struct event_loop g_event_loop;
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

static bool virtual_workspace_window_can_evacuate(struct window *window, uint32_t parent)
{
    return window && window->is_root && !parent && window_is_real(window) &&
           !window_check_flag(window, WINDOW_STICKY) && !window_check_flag(window, WINDOW_FULLSCREEN);
}

static bool virtual_workspace_native_ax_identity(struct virtual_workspace_window *identity, struct window *window)
{
    if (ax_window_id(window->ref) != identity->wid) return false;
    CFTypeRef role = NULL;
    AXError error = AXUIElementCopyAttributeValue(window->ref, kAXRoleAttribute, &role);
    bool valid = error == kAXErrorSuccess && role && window_role(window) && CFEqual(role, window_role(window));
    if (role) CFRelease(role);
    return valid;
}

static enum virtual_workspace_window_state virtual_workspace_native_window_state(struct virtual_workspace_window *identity, uint64_t src, uint64_t dst)
{
    struct window *window = window_manager_find_window(&g_window_manager, identity->wid);
    if (!window || window->ref != identity->ref || window->application->pid != identity->pid ||
        !__atomic_load_n(&window->id_ptr, __ATOMIC_RELAXED)) return VIRTUAL_WORKSPACE_WINDOW_LEFT;
    if (!virtual_workspace_native_ax_identity(identity, window)) return VIRTUAL_WORKSPACE_WINDOW_UNKNOWN;

    int cid = 0;
    pid_t pid = 0;
    if (SLSGetWindowOwner(g_connection, identity->wid, &cid) != kCGErrorSuccess ||
        SLSConnectionGetPID(cid, &pid) != kCGErrorSuccess) return VIRTUAL_WORKSPACE_WINDOW_UNKNOWN;
    if (pid != identity->pid) return VIRTUAL_WORKSPACE_WINDOW_LEFT;

    int count = 0;
    uint64_t *spaces = window_space_list(identity->wid, &count);
    if (!spaces || count != 1) return VIRTUAL_WORKSPACE_WINDOW_UNKNOWN;
    if (spaces[0] == dst) return VIRTUAL_WORKSPACE_WINDOW_TARGET;
    if (spaces[0] != src) return VIRTUAL_WORKSPACE_WINDOW_LEFT;

    /* Children and all-Spaces/fullscreen windows have no safe independent move. */
    CFArrayRef window_list = cfarray_of_cfnumbers(&identity->wid, sizeof(uint32_t), 1, kCFNumberSInt32Type);
    if (!window_list) return VIRTUAL_WORKSPACE_WINDOW_UNKNOWN;
    CFTypeRef query = SLSWindowQueryWindows(g_connection, window_list, 1);
    CFTypeRef iterator = query ? SLSWindowQueryResultCopyWindows(query) : NULL;
    bool supported = iterator && SLSWindowIteratorGetCount(iterator) == 1 && SLSWindowIteratorAdvance(iterator) &&
                     virtual_workspace_window_can_evacuate(window, SLSWindowIteratorGetParentID(iterator));
    if (iterator) CFRelease(iterator);
    if (query) CFRelease(query);
    CFRelease(window_list);
    if (!supported) return VIRTUAL_WORKSPACE_WINDOW_UNKNOWN;
    CFTypeRef fullscreen = NULL;
    AXError error = AXUIElementCopyAttributeValue(window->ref, kAXFullscreenAttribute, &fullscreen);
    supported = error == kAXErrorAttributeUnsupported ||
                (error == kAXErrorSuccess && fullscreen && CFEqual(fullscreen, kCFBooleanFalse));
    if (fullscreen) CFRelease(fullscreen);
    if (!supported) return VIRTUAL_WORKSPACE_WINDOW_UNKNOWN;
    return VIRTUAL_WORKSPACE_WINDOW_SOURCE;
}

static bool virtual_workspace_native_can_destroy(uint32_t src_did, uint32_t dst_did)
{
    return !mission_control_is_active() && !display_manager_display_is_animating(src_did) &&
           !display_manager_display_is_animating(dst_did);
}

static enum virtual_workspace_occupancy virtual_workspace_native_scan(uint64_t sid, struct virtual_workspace_window **identities)
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
            if (result != VIRTUAL_WORKSPACE_UNKNOWN) result = occupancy;
            if (identities) {
                struct virtual_workspace_window identity = { .wid = wid, .pid = window->application->pid, .ref = window->ref };
                if (SLSWindowIteratorGetParentID(iterator) || virtual_workspace_native_window_state(&identity, sid, 0) != VIRTUAL_WORKSPACE_WINDOW_SOURCE) {
                    result = VIRTUAL_WORKSPACE_UNKNOWN;
                } else {
                    CFRetain(identity.ref);
                    buf_push(*identities, identity);
                }
            }
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

static enum virtual_workspace_occupancy virtual_workspace_native_occupancy(uint64_t sid)
{
    return virtual_workspace_native_scan(sid, NULL);
}

static bool virtual_workspace_native_collect_windows(uint64_t sid, struct virtual_workspace_window **windows)
{
    if (SLSPerformAsynchronousBridgedWindowManagementOperation && !objc_getClass("SLSBridgedMoveWindowsToManagedSpaceOperation")) return false;
    return virtual_workspace_native_scan(sid, windows) != VIRTUAL_WORKSPACE_UNKNOWN;
}

static void virtual_workspace_native_move_window(struct virtual_workspace_window *identity, uint64_t sid)
{
    struct window *window = window_manager_find_window(&g_window_manager, identity->wid);
    window_manager_send_window_to_space(&g_space_manager, &g_window_manager, window, sid, false);
}

static bool virtual_workspace_native_settle_window(struct virtual_workspace_window *identity)
{
    struct window *window = window_manager_find_window(&g_window_manager, identity->wid);
    if (!window || window->ref != identity->ref || window->application->pid != identity->pid ||
        !__atomic_load_n(&window->id_ptr, __ATOMIC_RELAXED)) return true;
    int cid = 0;
    pid_t pid = 0;
    if (SLSGetWindowOwner(g_connection, identity->wid, &cid) != kCGErrorSuccess ||
        SLSConnectionGetPID(cid, &pid) != kCGErrorSuccess) return false;
    if (pid != identity->pid) return true;
    if (!virtual_workspace_native_ax_identity(identity, window)) return false;
    int count = 0;
    uint64_t *spaces = window_space_list(window->id, &count);
    struct view *view = window_manager_find_managed_window(&g_window_manager, window);
    bool known = spaces && count == 1 && space_display_id(spaces[0]) && space_is_user(spaces[0]);
    bool manage = known && window_manager_should_manage_window(window);
    if (view && (!manage || view->sid != spaces[0])) {
        space_manager_untile_window(view, window);
        window_manager_remove_managed_window(&g_window_manager, window->id);
        window_manager_purify_window(&g_window_manager, window);
        view = NULL;
    }
    if (manage && !view) {
        view = space_manager_tile_window_on_space(&g_space_manager, window, spaces[0]);
        window_manager_add_managed_window(&g_window_manager, window, view);
    }
    return known;
}

static void virtual_workspace_native_capture_focus(struct virtual_workspace_window *identity)
{
    struct window *window = window_manager_focused_window(&g_window_manager);
    if (!window) return;
    int count = 0;
    uint64_t *spaces = window_space_list(window->id, &count);
    bool member = false;
    uint64_t active = space_manager_active_space();
    for (int i = 0; spaces && i < count; ++i) if (spaces[i] == active) member = true;
    /* AX may remember an off-Space window when global focus is an empty desktop. */
    if (!member) return;
    *identity = (struct virtual_workspace_window) { .wid = window->id, .pid = window->application->pid, .ref = CFRetain(window->ref) };
}

static bool virtual_workspace_native_restore_focus(struct virtual_workspace_window *identity, bool request)
{
    if (!identity->wid) return true;
    struct window *window = window_manager_find_window(&g_window_manager, identity->wid);
    if (!window || window->ref != identity->ref || window->application->pid != identity->pid ||
        !__atomic_load_n(&window->id_ptr, __ATOMIC_RELAXED)) return true;
    int cid = 0;
    pid_t pid = 0;
    if (SLSGetWindowOwner(g_connection, identity->wid, &cid) != kCGErrorSuccess ||
        SLSConnectionGetPID(cid, &pid) != kCGErrorSuccess || pid != identity->pid ||
        !virtual_workspace_native_ax_identity(identity, window)) return false;
    if (window_manager_focused_window(&g_window_manager) == window) return true;
    /* A focused sticky window is allowed here; it is never an evacuation source. */
    int count = 0;
    uint64_t *spaces = window_space_list(window->id, &count);
    bool member = false;
    for (int i = 0; spaces && i < count; ++i) if (spaces[i] == space_manager_active_space()) member = true;
    if (!member) return false;
    if (request) window_manager_focus_window_with_raise(&window->application->psn, window->id, window->ref);
    return false;
}

static void virtual_workspace_native_schedule_destroy(uint64_t token)
{
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.05 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        event_loop_post(&g_event_loop, VSPACE_DESTROY_CONTINUE, (void *)(uintptr_t)token, 0);
    });
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
    .focus_space = space_manager_focus_space,
    .can_destroy = virtual_workspace_native_can_destroy,
    .collect_windows = virtual_workspace_native_collect_windows,
    .window_state = virtual_workspace_native_window_state,
    .move_window = virtual_workspace_native_move_window,
    .settle_window = virtual_workspace_native_settle_window,
    .capture_focus = virtual_workspace_native_capture_focus,
    .restore_focus = virtual_workspace_native_restore_focus,
    .schedule_destroy = virtual_workspace_native_schedule_destroy
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
    if (vm->destroy.token) return VIRTUAL_WORKSPACE_BUSY;
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
    if (vm->destroy.token) return VIRTUAL_WORKSPACE_BUSY;
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

static void virtual_workspace_destroy_dispose(struct virtual_workspace_destroy *operation)
{
    for (int i = 0; i < buf_len(operation->windows); ++i) {
        if (operation->windows[i].ref) CFRelease(operation->windows[i].ref);
    }
    if (operation->focus_window.ref) CFRelease(operation->focus_window.ref);
    buf_free(operation->windows);
    buf_free(operation->topology);
    *operation = (struct virtual_workspace_destroy) {0};
}

static bool virtual_workspace_manager_read_topology(struct virtual_workspace_manager *vm, struct virtual_workspace_native_space **topology)
{
    const struct virtual_workspace_source *source = vm->source;
    int display_count = 0;
    uint32_t *displays = source->display_list(&display_count);
    if (!displays || display_count <= 0) return false;
    for (int i = 0; i < display_count; ++i) {
        if (!displays[i]) return false;
        for (int j = 0; j < i; ++j) if (displays[j] == displays[i]) return false;
        int space_count = 0;
        uint64_t *spaces = source->space_list(displays[i], &space_count);
        if (!spaces || space_count <= 0) return false;
        uint64_t visible = source->visible_space(displays[i]);
        bool found_visible = false;
        for (int j = 0; j < space_count; ++j) {
            if (!spaces[j] || source->space_display(spaces[j]) != displays[i]) return false;
            for (int k = 0; k < buf_len(*topology); ++k) if ((*topology)[k].sid == spaces[j]) return false;
            buf_push(*topology, ((struct virtual_workspace_native_space) { .sid = spaces[j], .did = displays[i] }));
            if (visible == spaces[j]) found_visible = true;
        }
        if (!found_visible) return false;
    }
    return true;
}

static bool virtual_workspace_manager_valid_mapping(struct virtual_workspace_manager *vm, struct virtual_workspace *workspace, struct virtual_workspace_native_space *topology)
{
    struct virtual_workspace *live = virtual_workspace_manager_find_space(vm, workspace->sid);
    if (!live || live->id != workspace->id || live->did != workspace->did || live->index != workspace->index ||
        !vm->source->is_user_space(workspace->sid)) return false;
    for (int i = 0; i < buf_len(topology); ++i) {
        if (topology[i].sid == workspace->sid && topology[i].did == workspace->did) return true;
    }
    return false;
}

struct virtual_workspace_refresh_snapshot
{
    struct virtual_workspace_native_space *topology;
    struct virtual_workspace_native_space *visible;
    uint64_t focused_sid;
};

static void virtual_workspace_refresh_snapshot_dispose(struct virtual_workspace_refresh_snapshot *snapshot)
{
    buf_free(snapshot->topology);
    buf_free(snapshot->visible);
}

static bool virtual_workspace_refresh_read_snapshot(struct virtual_workspace_manager *vm, struct virtual_workspace_refresh_snapshot *snapshot)
{
    const struct virtual_workspace_source *source = vm->source;
    int display_count = 0;
    uint32_t *displays = source->display_list(&display_count);
    if (!displays || display_count <= 0) return false;
    for (int i = 0; i < display_count; ++i) {
        if (!displays[i]) return false;
        for (int j = 0; j < i; ++j) if (displays[j] == displays[i]) return false;
        int space_count = 0;
        uint64_t *spaces = source->space_list(displays[i], &space_count);
        if (!spaces || space_count <= 0) return false;
        uint64_t visible = source->visible_space(displays[i]);
        bool found_visible = false;
        for (int j = 0; j < space_count; ++j) {
            if (!spaces[j]) return false;
            for (int k = 0; k < buf_len(snapshot->topology); ++k) if (snapshot->topology[k].sid == spaces[j]) return false;
            buf_push(snapshot->topology, ((struct virtual_workspace_native_space) { .sid = spaces[j], .did = displays[i] }));
            if (spaces[j] == visible) found_visible = true;
        }
        if (!found_visible) return false;
        buf_push(snapshot->visible, ((struct virtual_workspace_native_space) { .sid = visible, .did = displays[i] }));
    }
    snapshot->focused_sid = source->active_space();
    for (int i = 0; i < buf_len(snapshot->visible); ++i) {
        if (snapshot->focused_sid == snapshot->visible[i].sid) return true;
    }
    return false;
}

static bool virtual_workspace_refresh_same_topology(struct virtual_workspace_refresh_snapshot *a, struct virtual_workspace_refresh_snapshot *b)
{
    if (buf_len(a->topology) != buf_len(b->topology)) return false;
    for (int i = 0; i < buf_len(a->topology); ++i) {
        if (a->topology[i].sid != b->topology[i].sid || a->topology[i].did != b->topology[i].did) return false;
    }
    return true;
}

static enum virtual_workspace_retention virtual_workspace_refresh_protection(struct virtual_workspace_manager *vm, struct virtual_workspace *workspace, struct virtual_workspace_refresh_snapshot *snapshot)
{
    bool connected = false;
    for (int i = 0; i < buf_len(snapshot->visible); ++i) {
        if (snapshot->visible[i].did == workspace->did) connected = true;
    }
    if (!connected) return VIRTUAL_WORKSPACE_RETAIN_DISPLAY;
    /* Enumeration failure aborts refresh; uncertain individual identities retain. */
    if (!virtual_workspace_manager_valid_mapping(vm, workspace, snapshot->topology) ||
        vm->source->space_display(workspace->sid) != workspace->did) return VIRTUAL_WORKSPACE_RETAIN_MAPPING;
    if (workspace->sid == snapshot->focused_sid) return VIRTUAL_WORKSPACE_RETAIN_FOCUSED;
    for (int i = 0; i < buf_len(snapshot->visible); ++i) {
        if (workspace->sid == snapshot->visible[i].sid) return VIRTUAL_WORKSPACE_RETAIN_VISIBLE;
    }
    return VIRTUAL_WORKSPACE_RELEASE;
}

static enum virtual_workspace_retention virtual_workspace_refresh_classify(struct virtual_workspace_manager *vm, struct virtual_workspace *workspace, struct virtual_workspace_refresh_snapshot *snapshot)
{
    enum virtual_workspace_retention retention = virtual_workspace_refresh_protection(vm, workspace, snapshot);
    if (retention != VIRTUAL_WORKSPACE_RELEASE) return retention;
    enum virtual_workspace_occupancy occupancy = vm->source->occupancy(workspace->sid);
    if (occupancy == VIRTUAL_WORKSPACE_EMPTY) return VIRTUAL_WORKSPACE_RELEASE;
    return occupancy == VIRTUAL_WORKSPACE_OCCUPIED ? VIRTUAL_WORKSPACE_RETAIN_OCCUPIED : VIRTUAL_WORKSPACE_RETAIN_UNKNOWN;
}

/* One daemon-loop publication. M2-D can persist this candidate before the swap. */
static void virtual_workspace_manager_publish_refresh(struct virtual_workspace_manager *vm, struct virtual_workspace *candidate)
{
    struct virtual_workspace *previous = vm->workspaces;
    vm->workspaces = candidate;
    buf_free(previous);
}

enum virtual_workspace_error virtual_workspace_manager_refresh(struct virtual_workspace_manager *vm, struct virtual_workspace_refresh_entry **entries)
{
    *entries = NULL;
    if (!vm->did_begin) return VIRTUAL_WORKSPACE_NOT_INITIALIZED;
    /* Destroy retains stable source/target/replacement indices until completion. */
    if (vm->destroy.token) return VIRTUAL_WORKSPACE_BUSY;
    struct virtual_workspace_refresh_snapshot before = {0}, confirmed = {0}, final = {0}, publication = {0};
    struct virtual_workspace_refresh_entry *pending = NULL;
    struct virtual_workspace *candidate = NULL;
    enum virtual_workspace_error result = VIRTUAL_WORKSPACE_STATE_UNAVAILABLE;
    if (!virtual_workspace_refresh_read_snapshot(vm, &before)) goto out;

    /* The live array is the authoritative global order, including interleaving. */
    for (int i = 0; i < buf_len(vm->workspaces); ++i) {
        struct virtual_workspace workspace = vm->workspaces[i];
        buf_push(pending, ((struct virtual_workspace_refresh_entry) {
            .workspace = workspace,
            .retention = virtual_workspace_refresh_classify(vm, &workspace, &before)
        }));
    }
    if (!virtual_workspace_refresh_read_snapshot(vm, &confirmed)) goto out;
    result = VIRTUAL_WORKSPACE_STATE_CHANGED;
    if (!virtual_workspace_refresh_same_topology(&before, &confirmed)) goto out;
    for (int i = 0; i < buf_len(pending); ++i) {
        if (pending[i].retention != VIRTUAL_WORKSPACE_RELEASE) continue;
        pending[i].retention = virtual_workspace_refresh_classify(vm, &pending[i].workspace, &confirmed);
    }
    /* Bracket occupancy queries with complete visibility/topology snapshots. */
    result = VIRTUAL_WORKSPACE_STATE_UNAVAILABLE;
    if (!virtual_workspace_refresh_read_snapshot(vm, &final)) goto out;
    result = VIRTUAL_WORKSPACE_STATE_CHANGED;
    if (!virtual_workspace_refresh_same_topology(&confirmed, &final)) goto out;
    for (int i = 0; i < buf_len(pending); ++i) {
        if (pending[i].retention == VIRTUAL_WORKSPACE_RELEASE) {
            pending[i].retention = virtual_workspace_refresh_classify(vm, &pending[i].workspace, &final);
        }
    }
    result = VIRTUAL_WORKSPACE_STATE_UNAVAILABLE;
    if (!virtual_workspace_refresh_read_snapshot(vm, &publication)) goto out;
    result = VIRTUAL_WORKSPACE_STATE_CHANGED;
    if (!virtual_workspace_refresh_same_topology(&final, &publication)) goto out;
    int retained = 0;
    for (int i = 0; i < buf_len(pending); ++i) {
        if (pending[i].retention == VIRTUAL_WORKSPACE_RELEASE) {
            pending[i].retention = virtual_workspace_refresh_protection(vm, &pending[i].workspace, &publication);
        }
        if (pending[i].retention != VIRTUAL_WORKSPACE_RELEASE) ++retained;
    }
    if (!retained && buf_len(pending)) pending[0].retention = VIRTUAL_WORKSPACE_RETAIN_LAST;
    for (int i = 0; i < buf_len(pending); ++i) {
        if (pending[i].retention == VIRTUAL_WORKSPACE_RELEASE) continue;
        struct virtual_workspace workspace = pending[i].workspace;
        pending[i].new_index = workspace.index = buf_len(candidate) + 1;
        buf_push(candidate, workspace);
    }
    if (!buf_len(candidate)) goto out;
    virtual_workspace_manager_publish_refresh(vm, candidate);
    candidate = NULL;
    *entries = pending;
    pending = NULL;
    result = VIRTUAL_WORKSPACE_SUCCESS;

out:
    virtual_workspace_refresh_snapshot_dispose(&before);
    virtual_workspace_refresh_snapshot_dispose(&confirmed);
    virtual_workspace_refresh_snapshot_dispose(&final);
    virtual_workspace_refresh_snapshot_dispose(&publication);
    buf_free(candidate);
    buf_free(pending);
    return result;
}

static struct virtual_workspace virtual_workspace_manager_local_neighbor(struct virtual_workspace_manager *vm, struct virtual_workspace *workspace, struct virtual_workspace_native_space *topology)
{
    struct virtual_workspace *before = NULL;
    struct virtual_workspace *after = NULL;
    for (int i = 0; i < buf_len(vm->workspaces); ++i) {
        struct virtual_workspace *candidate = &vm->workspaces[i];
        if (candidate->did != workspace->did || candidate->id == workspace->id ||
            !virtual_workspace_manager_valid_mapping(vm, candidate, topology)) continue;
        if (candidate->index < workspace->index && (!before || candidate->index > before->index)) before = candidate;
        if (candidate->index > workspace->index && (!after || candidate->index < after->index)) after = candidate;
    }
    return before ? *before : after ? *after : (struct virtual_workspace) {0};
}

static bool virtual_workspace_manager_destroy_state_valid(struct virtual_workspace_manager *vm)
{
    struct virtual_workspace_destroy *operation = &vm->destroy;
    struct virtual_workspace_native_space *topology = NULL;
    bool valid = virtual_workspace_manager_read_topology(vm, &topology) && buf_len(topology) == buf_len(operation->topology);
    for (int i = 0; valid && i < buf_len(topology); ++i) {
        if (topology[i].sid != operation->topology[i].sid || topology[i].did != operation->topology[i].did) valid = false;
    }
    valid = valid && virtual_workspace_manager_valid_mapping(vm, &operation->source, topology) &&
                    virtual_workspace_manager_valid_mapping(vm, &operation->target, topology);
    if (operation->replacement.id) valid = valid && virtual_workspace_manager_valid_mapping(vm, &operation->replacement, topology);
    buf_free(topology);
    return valid;
}

static void virtual_workspace_manager_destroy_finish(struct virtual_workspace_manager *vm, enum virtual_workspace_error result)
{
    struct virtual_workspace_destroy *operation = &vm->destroy;
    for (int i = 0; i < buf_len(operation->windows); ++i) {
        struct virtual_workspace_window *window = &operation->windows[i];
        if (!window->sent) continue;
        window->moved = vm->source->window_state(window, operation->source.sid, operation->target.sid) == VIRTUAL_WORKSPACE_WINDOW_TARGET;
        if (result != VIRTUAL_WORKSPACE_SUCCESS) vm->source->settle_window(window);
    }
    if (operation->complete) operation->complete(operation, result);
    virtual_workspace_destroy_dispose(operation);
}

/* M2-D can persist candidate metadata here before publishing mapping removal. */
static void virtual_workspace_manager_release(struct virtual_workspace_manager *vm, struct virtual_workspace *workspace)
{
    for (int i = 0; i < buf_len(vm->workspaces); ++i) {
        if (vm->workspaces[i].id != workspace->id) continue;
        memmove(vm->workspaces + i, vm->workspaces + i + 1, (buf_len(vm->workspaces) - i - 1) * sizeof(*vm->workspaces));
        --buf__hdr(vm->workspaces)->len;
        return;
    }
}

enum virtual_workspace_error virtual_workspace_manager_destroy(struct virtual_workspace_manager *vm, int index, int target_index, void *context, void (*complete)(struct virtual_workspace_destroy *, enum virtual_workspace_error))
{
    if (!vm->did_begin) return VIRTUAL_WORKSPACE_NOT_INITIALIZED;
    if (vm->destroy.token) return VIRTUAL_WORKSPACE_BUSY;
    struct virtual_workspace *workspace = virtual_workspace_manager_find_index(vm, index);
    if (!workspace) return VIRTUAL_WORKSPACE_INVALID_INDEX;
    if (buf_len(vm->workspaces) <= 1) return VIRTUAL_WORKSPACE_LAST;
    struct virtual_workspace_destroy operation = { .source = *workspace, .context = context, .complete = complete };
    enum virtual_workspace_error result = VIRTUAL_WORKSPACE_STATE_UNAVAILABLE;
    if (!virtual_workspace_manager_read_topology(vm, &operation.topology)) goto err;
    result = VIRTUAL_WORKSPACE_SPACE_UNAVAILABLE;
    if (!virtual_workspace_manager_valid_mapping(vm, &operation.source, operation.topology)) goto err;
    if (target_index) {
        workspace = virtual_workspace_manager_find_index(vm, target_index);
        result = VIRTUAL_WORKSPACE_INVALID_TARGET;
        if (!workspace || workspace->id == operation.source.id || !virtual_workspace_manager_valid_mapping(vm, workspace, operation.topology)) goto err;
        operation.target = *workspace;
    } else {
        operation.target = virtual_workspace_manager_local_neighbor(vm, &operation.source, operation.topology);
        result = VIRTUAL_WORKSPACE_LOCAL_TARGET_REQUIRED;
        if (!operation.target.id) goto err;
    }
    operation.replacement = operation.source.did == operation.target.did ? operation.target :
                            virtual_workspace_manager_local_neighbor(vm, &operation.source, operation.topology);
    operation.handoff_required = vm->source->visible_space(operation.source.did) == operation.source.sid;
    result = VIRTUAL_WORKSPACE_LOCAL_REPLACEMENT_REQUIRED;
    if (operation.handoff_required && !operation.replacement.id) goto err;
    operation.original_focus = vm->source->active_space();
    bool found_focus = false;
    for (int i = 0; i < buf_len(operation.topology); ++i) {
        if (operation.topology[i].sid == operation.original_focus) found_focus = true;
    }
    result = VIRTUAL_WORKSPACE_STATE_UNAVAILABLE;
    if (!found_focus) goto err;
    operation.final_focus = operation.original_focus == operation.source.sid ? operation.target.sid : operation.original_focus;
    if (!vm->source->can_destroy || !vm->source->can_destroy(operation.source.did, operation.target.did)) goto err;
    result = VIRTUAL_WORKSPACE_UNSAFE_WINDOWS;
    if (!vm->source->collect_windows || !vm->source->window_state || !vm->source->move_window ||
        !vm->source->settle_window || !vm->source->capture_focus || !vm->source->restore_focus || !vm->source->schedule_destroy ||
        !vm->source->collect_windows(operation.source.sid, &operation.windows)) goto err;
    for (int i = 0; i < buf_len(operation.windows); ++i) {
        if (vm->source->window_state(&operation.windows[i], operation.source.sid, operation.target.sid) == VIRTUAL_WORKSPACE_WINDOW_UNKNOWN) goto err;
    }
    vm->source->capture_focus(&operation.focus_window);
    result = VIRTUAL_WORKSPACE_LIMIT;
    if (vm->next_operation == UINT64_MAX) goto err;
    operation.token = ++vm->next_operation;
    vm->destroy = operation;
    vm->source->schedule_destroy(operation.token);
    return VIRTUAL_WORKSPACE_PENDING;
err:
    virtual_workspace_destroy_dispose(&operation);
    return result;
}

void virtual_workspace_manager_destroy_continue(struct virtual_workspace_manager *vm, uint64_t token)
{
    struct virtual_workspace_destroy *operation = &vm->destroy;
    if (!token || token != operation->token) return;
    enum virtual_workspace_error result = VIRTUAL_WORKSPACE_STATE_CHANGED;
    if (!virtual_workspace_manager_destroy_state_valid(vm)) goto err;
    if (vm->source->visible_space(operation->source.did) == operation->source.sid) operation->handoff_required = true;
    result = VIRTUAL_WORKSPACE_LOCAL_REPLACEMENT_REQUIRED;
    if (operation->handoff_required && !operation->replacement.id) goto err;

    if (operation->cursor < buf_len(operation->windows)) {
        struct virtual_workspace_window *window = &operation->windows[operation->cursor];
        enum virtual_workspace_window_state state = vm->source->window_state(window, operation->source.sid, operation->target.sid);
        result = VIRTUAL_WORKSPACE_UNSAFE_WINDOWS;
        if (state == VIRTUAL_WORKSPACE_WINDOW_UNKNOWN) goto err;
        if (state != VIRTUAL_WORKSPACE_WINDOW_SOURCE) {
            window->moved = window->sent && state == VIRTUAL_WORKSPACE_WINDOW_TARGET;
            result = VIRTUAL_WORKSPACE_MOVE_FAILED;
            if (window->sent && !vm->source->settle_window(window)) goto err;
            ++operation->cursor;
            operation->observations = 0;
        } else if (!window->sent) {
            result = VIRTUAL_WORKSPACE_STATE_CHANGED;
            if (!vm->source->can_destroy(operation->source.did, operation->target.did)) goto err;
            vm->source->move_window(window, operation->target.sid);
            window->sent = true;
            operation->observations = 0;
        } else {
            result = VIRTUAL_WORKSPACE_MOVE_FAILED;
            if (++operation->observations >= 100) goto err;
        }
        goto defer;
    }

    /* A new arrival, an untracked window, or incomplete iteration retains the map. */
    result = VIRTUAL_WORKSPACE_UNSAFE_WINDOWS;
    if (vm->source->occupancy(operation->source.sid) != VIRTUAL_WORKSPACE_EMPTY) goto err;
    if (operation->phase == 0) operation->phase = 1;
    if (operation->phase == 1) {
        result = VIRTUAL_WORKSPACE_LOCAL_REPLACEMENT_REQUIRED;
        if (operation->handoff_required && !operation->replacement.id) goto err;
        if (operation->handoff_required && vm->source->visible_space(operation->source.did) != operation->replacement.sid) {
            uint64_t sid = operation->replacement.sid;
            result = VIRTUAL_WORKSPACE_VISIBILITY_FAILED;
            if (!operation->requested_focus) {
                enum space_op_error error = vm->source->focus_space(sid);
                if (error != SPACE_OP_ERROR_SUCCESS && error != SPACE_OP_ERROR_SAME_SPACE) goto err;
                operation->requested_focus = sid;
            }
            if (++operation->observations >= 100) goto err;
            goto defer;
        }
        operation->phase = 2;
        operation->observations = 0;
        operation->requested_focus = 0;
    }
    if (operation->phase == 2) {
        result = VIRTUAL_WORKSPACE_VISIBILITY_FAILED;
        if (vm->source->active_space() != operation->final_focus) {
            if (!operation->requested_focus) {
                enum space_op_error error = vm->source->focus_space(operation->final_focus);
                if (error != SPACE_OP_ERROR_SUCCESS && error != SPACE_OP_ERROR_SAME_SPACE) goto err;
                operation->requested_focus = operation->final_focus;
            }
            if (++operation->observations >= 100) goto err;
            goto defer;
        }
        operation->phase = 3;
        operation->observations = 0;
    }
    if (operation->phase == 3 && operation->original_focus != operation->source.sid) {
        bool restored = vm->source->restore_focus(&operation->focus_window, !operation->restore_requested);
        operation->restore_requested = true;
        result = VIRTUAL_WORKSPACE_VISIBILITY_FAILED;
        if (!restored) {
            if (++operation->observations >= 100) goto err;
            goto defer;
        }
    }
    operation->phase = 4;
    result = VIRTUAL_WORKSPACE_VISIBILITY_FAILED;
    if (vm->source->active_space() != operation->final_focus ||
        vm->source->visible_space(operation->source.did) == operation->source.sid ||
        (operation->handoff_required && vm->source->visible_space(operation->source.did) != operation->replacement.sid)) goto err;
    /* Two fresh empty observations with an event-loop turn between them. */
    if (++operation->empty_observations < 2) goto defer;
    result = VIRTUAL_WORKSPACE_MOVE_FAILED;
    for (int i = 0; i < buf_len(operation->windows); ++i) {
        struct virtual_workspace_window *window = &operation->windows[i];
        enum virtual_workspace_window_state state = vm->source->window_state(window, operation->source.sid, operation->target.sid);
        if (state == VIRTUAL_WORKSPACE_WINDOW_SOURCE || state == VIRTUAL_WORKSPACE_WINDOW_UNKNOWN ||
            (window->sent && !vm->source->settle_window(window))) goto err;
    }
    /* Settle can touch native layouts; requery immediately before publication. */
    result = VIRTUAL_WORKSPACE_STATE_CHANGED;
    if (!virtual_workspace_manager_destroy_state_valid(vm) ||
        vm->source->occupancy(operation->source.sid) != VIRTUAL_WORKSPACE_EMPTY ||
        vm->source->visible_space(operation->source.did) == operation->source.sid ||
        (operation->handoff_required && vm->source->visible_space(operation->source.did) != operation->replacement.sid) ||
        vm->source->active_space() != operation->final_focus ||
        vm->source->visible_space(vm->source->space_display(operation->final_focus)) != operation->final_focus ||
        (operation->original_focus != operation->source.sid && !vm->source->restore_focus(&operation->focus_window, false))) goto err;
    virtual_workspace_manager_release(vm, &operation->source);
    virtual_workspace_manager_destroy_finish(vm, VIRTUAL_WORKSPACE_SUCCESS);
    return;
defer:
    vm->source->schedule_destroy(operation->token);
    return;
err:
    virtual_workspace_manager_destroy_finish(vm, result);
}
