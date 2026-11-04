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
    uint32_t (*space_display)(uint64_t sid);
    enum virtual_workspace_occupancy (*occupancy)(uint64_t sid);
    enum space_op_error (*focus_space)(uint64_t sid);
};

struct virtual_workspace_manager
{
    struct virtual_workspace *workspaces;
    const struct virtual_workspace_source *source;
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
    VIRTUAL_WORKSPACE_FOCUS_FAILED
};

bool virtual_workspace_manager_begin(struct virtual_workspace_manager *vm, const struct virtual_workspace_source *source);
struct virtual_workspace *virtual_workspace_manager_find_index(struct virtual_workspace_manager *vm, int index);
struct virtual_workspace *virtual_workspace_manager_find_space(struct virtual_workspace_manager *vm, uint64_t sid);
enum virtual_workspace_error virtual_workspace_manager_focus(struct virtual_workspace_manager *vm, int index);

#endif
