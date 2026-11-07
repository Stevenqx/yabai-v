#ifndef VIRTUAL_WORKSPACE_MANAGER_H
#define VIRTUAL_WORKSPACE_MANAGER_H

struct virtual_workspace
{
    uint64_t id;
    uint64_t sid;
    uint32_t did;
    int index;
    char space_uuid[37];
    char display_uuid[37];
    bool unresolved;
};

enum virtual_workspace_occupancy
{
    VIRTUAL_WORKSPACE_EMPTY,
    VIRTUAL_WORKSPACE_OCCUPIED,
    VIRTUAL_WORKSPACE_UNKNOWN
};

enum virtual_workspace_error
{
    VIRTUAL_WORKSPACE_SUCCESS,
    VIRTUAL_WORKSPACE_NOT_INITIALIZED,
    VIRTUAL_WORKSPACE_INVALID_INDEX,
    VIRTUAL_WORKSPACE_SPACE_UNAVAILABLE,
    VIRTUAL_WORKSPACE_SAME_SPACE,
    VIRTUAL_WORKSPACE_DISPLAY_IS_ANIMATING,
    VIRTUAL_WORKSPACE_IN_MISSION_CONTROL,
    VIRTUAL_WORKSPACE_SCRIPTING_ADDITION,
    VIRTUAL_WORKSPACE_FOCUS_FAILED,
    VIRTUAL_WORKSPACE_DISPLAY_UNAVAILABLE,
    VIRTUAL_WORKSPACE_STATE_UNAVAILABLE,
    VIRTUAL_WORKSPACE_NO_SPARE,
    VIRTUAL_WORKSPACE_STATE_CHANGED,
    VIRTUAL_WORKSPACE_LIMIT,
    VIRTUAL_WORKSPACE_BUSY,
    VIRTUAL_WORKSPACE_LAST,
    VIRTUAL_WORKSPACE_INVALID_TARGET,
    VIRTUAL_WORKSPACE_LOCAL_TARGET_REQUIRED,
    VIRTUAL_WORKSPACE_LOCAL_REPLACEMENT_REQUIRED,
    VIRTUAL_WORKSPACE_UNSAFE_WINDOWS,
    VIRTUAL_WORKSPACE_MOVE_FAILED,
    VIRTUAL_WORKSPACE_VISIBILITY_FAILED,
    VIRTUAL_WORKSPACE_PENDING,
    VIRTUAL_WORKSPACE_PERSISTENCE_FAILED,
    VIRTUAL_WORKSPACE_DURABILITY_UNCERTAIN
};

struct virtual_workspace_window
{
    uint32_t wid;
    pid_t pid;
    AXUIElementRef ref;
    bool sent;
    bool moved;
};

enum virtual_workspace_window_state
{
    VIRTUAL_WORKSPACE_WINDOW_SOURCE,
    VIRTUAL_WORKSPACE_WINDOW_TARGET,
    VIRTUAL_WORKSPACE_WINDOW_LEFT,
    VIRTUAL_WORKSPACE_WINDOW_UNKNOWN
};

struct virtual_workspace_native_space
{
    uint64_t sid;
    uint32_t did;
};

struct virtual_workspace_destroy;

enum virtual_workspace_retention
{
    VIRTUAL_WORKSPACE_RELEASE,
    VIRTUAL_WORKSPACE_RETAIN_FOCUSED,
    VIRTUAL_WORKSPACE_RETAIN_VISIBLE,
    VIRTUAL_WORKSPACE_RETAIN_OCCUPIED,
    VIRTUAL_WORKSPACE_RETAIN_UNKNOWN,
    VIRTUAL_WORKSPACE_RETAIN_DISPLAY,
    VIRTUAL_WORKSPACE_RETAIN_MAPPING,
    VIRTUAL_WORKSPACE_RETAIN_LAST
};

struct virtual_workspace_refresh_entry
{
    struct virtual_workspace workspace;
    int new_index;
    enum virtual_workspace_retention retention;
};

/* Native queries are separate from mapping policy so it can run headlessly. */
struct virtual_workspace_source
{
    uint32_t *(*display_list)(int *count);
    uint64_t *(*space_list)(uint32_t did, int *count);
    uint64_t (*active_space)(void);
    uint32_t (*active_display)(void);
    uint64_t (*visible_space)(uint32_t did);
    bool (*is_user_space)(uint64_t sid);
    uint32_t (*space_display)(uint64_t sid);
    enum virtual_workspace_occupancy (*occupancy)(uint64_t sid);
    enum space_op_error (*focus_space)(uint64_t sid);
    bool (*can_destroy)(uint32_t src_did, uint32_t dst_did);
    bool (*collect_windows)(uint64_t sid, struct virtual_workspace_window **windows);
    enum virtual_workspace_window_state (*window_state)(struct virtual_workspace_window *window, uint64_t src, uint64_t dst);
    void (*move_window)(struct virtual_workspace_window *window, uint64_t sid);
    bool (*settle_window)(struct virtual_workspace_window *window);
    void (*capture_focus)(struct virtual_workspace_window *window);
    bool (*restore_focus)(struct virtual_workspace_window *window, bool request);
    void (*schedule_destroy)(uint64_t token);
    bool (*identity)(uint64_t sid, uint32_t did, char *space_uuid, char *display_uuid);
};

struct virtual_workspace_destroy
{
    uint64_t token;
    struct virtual_workspace source;
    struct virtual_workspace target;
    struct virtual_workspace replacement;
    struct virtual_workspace_native_space *topology;
    struct virtual_workspace_window *windows;
    struct virtual_workspace_window focus_window;
    uint64_t original_focus;
    uint64_t final_focus;
    uint64_t requested_focus;
    int cursor;
    int phase;
    int observations;
    int empty_observations;
    bool restore_requested;
    bool handoff_required;
    void *context;
    void (*complete)(struct virtual_workspace_destroy *operation, enum virtual_workspace_error result);
};

struct virtual_workspace_store;

struct virtual_workspace_manager
{
    struct virtual_workspace *workspaces;
    const struct virtual_workspace_source *source;
    uint64_t next_id;
    uint64_t next_operation;
    struct virtual_workspace_destroy destroy;
    bool did_begin;
    struct virtual_workspace_store *store;
};

bool virtual_workspace_manager_available(struct virtual_workspace_manager *vm, struct virtual_workspace *workspace);
const char *virtual_workspace_manager_persistence(struct virtual_workspace_manager *vm);
bool virtual_workspace_manager_begin(struct virtual_workspace_manager *vm, const struct virtual_workspace_source *source);
struct virtual_workspace *virtual_workspace_manager_find_index(struct virtual_workspace_manager *vm, int index);
struct virtual_workspace *virtual_workspace_manager_find_space(struct virtual_workspace_manager *vm, uint64_t sid);
enum virtual_workspace_error virtual_workspace_manager_focus(struct virtual_workspace_manager *vm, int index);
enum virtual_workspace_error virtual_workspace_manager_create(struct virtual_workspace_manager *vm);
enum virtual_workspace_error virtual_workspace_manager_refresh(struct virtual_workspace_manager *vm, struct virtual_workspace_refresh_entry **entries);
enum virtual_workspace_error virtual_workspace_manager_destroy(struct virtual_workspace_manager *vm, int index, int target_index, void *context, void (*complete)(struct virtual_workspace_destroy *, enum virtual_workspace_error));
void virtual_workspace_manager_destroy_continue(struct virtual_workspace_manager *vm, uint64_t token);

#endif
