#ifndef VIRTUAL_WORKSPACE_MANAGER_H
#define VIRTUAL_WORKSPACE_MANAGER_H

struct virtual_workspace
{
    uint64_t id;
    uint64_t sid;
    uint32_t did;
    int index;
};

enum virtual_workspace_occupancy
{
    VIRTUAL_WORKSPACE_EMPTY,
    VIRTUAL_WORKSPACE_OCCUPIED,
    VIRTUAL_WORKSPACE_UNKNOWN
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
};

struct virtual_workspace_manager
{
    struct virtual_workspace *workspaces;
    const struct virtual_workspace_source *source;
    uint64_t next_id;
    bool did_begin;
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
    VIRTUAL_WORKSPACE_LIMIT
};

bool virtual_workspace_manager_begin(struct virtual_workspace_manager *vm, const struct virtual_workspace_source *source);
struct virtual_workspace *virtual_workspace_manager_find_index(struct virtual_workspace_manager *vm, int index);
struct virtual_workspace *virtual_workspace_manager_find_space(struct virtual_workspace_manager *vm, uint64_t sid);
enum virtual_workspace_error virtual_workspace_manager_focus(struct virtual_workspace_manager *vm, int index);
enum virtual_workspace_error virtual_workspace_manager_create(struct virtual_workspace_manager *vm);

#endif
