/* Real files and production serialization/commit paths, isolated from user state. */
static struct {
    uint64_t remap_from;
    uint64_t remap_to;
    uint64_t changed;
    uint64_t missing;
    uint64_t duplicate;
    bool duplicate_display;
} virtual_workspace_state_test_identity;

static bool virtual_workspace_state_test_native_identity(uint64_t sid, uint32_t did, char *space, char *display)
{
    if (sid == virtual_workspace_state_test_identity.missing) return false;
    uint64_t stable = sid == virtual_workspace_state_test_identity.remap_to ? virtual_workspace_state_test_identity.remap_from : sid;
    if (sid == virtual_workspace_state_test_identity.changed) stable += 100000;
    if (sid == virtual_workspace_state_test_identity.duplicate) stable = 700;
    snprintf(space, 37, "00000001-0000-0000-0000-%012llX", (unsigned long long)stable);
    int slot = -1;
    for (int i = 0; i < 2; ++i) if (did == virtual_workspace_test_state.displays[i]) slot = i;
    snprintf(display, 37, "00000002-0000-0000-0000-%012X", virtual_workspace_state_test_identity.duplicate_display ? 1 : slot + 1);
    return slot >= 0;
}

static struct virtual_workspace_source virtual_workspace_state_test_source(void)
{
    struct virtual_workspace_source source = virtual_workspace_test_source;
    source.identity = virtual_workspace_state_test_native_identity;
    return source;
}

static void virtual_workspace_state_test_reset(void)
{
    virtual_workspace_test_reset();
    memset(&virtual_workspace_state_test_identity, 0, sizeof(virtual_workspace_state_test_identity));
}

static void virtual_workspace_state_test_close(struct virtual_workspace_manager *vm)
{
    /* Test teardown models process death; no pending operation is persisted. */
    virtual_workspace_destroy_dispose(&vm->destroy);
    buf_free(vm->workspaces);
    virtual_workspace_state_close(vm->store);
    memset(vm, 0, sizeof(*vm));
}

static bool virtual_workspace_state_test_directory(char *path)
{
    strlcpy(path, "/private/tmp/yabai-v-state-test.XXXXXX", PATH_MAX);
    return mkdtemp(path) != NULL;
}

static void virtual_workspace_state_test_remove(const char *path)
{
    int fd = open(path, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    DIR *directory = fd >= 0 ? fdopendir(fd) : NULL;
    if (!directory) return;
    struct dirent *entry;
    while ((entry = readdir(directory))) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
        unlinkat(fd, entry->d_name, 0);
    }
    closedir(directory);
    rmdir(path);
}

static NSData *virtual_workspace_state_test_data(const char *path)
{
    return [NSData dataWithContentsOfFile:[[NSString stringWithUTF8String:path] stringByAppendingPathComponent:@"state.json"]];
}

static bool virtual_workspace_state_test_put(const char *path, NSData *data)
{
    int directory = open(path, O_RDONLY | O_DIRECTORY);
    if (directory < 0) return false;
    int fd = openat(directory, "state.json", O_WRONLY | O_TRUNC | O_CREAT, 0600);
    bool ok = fd >= 0 && virtual_workspace_state_write(fd, [data bytes], [data length]);
    if (fd >= 0) close(fd);
    close(directory);
    return ok;
}

TEST_FUNC(virtual_workspace_state_lossless_and_limits,
{
    @autoreleasepool {
        struct virtual_workspace *records = NULL, *decoded = NULL;
        struct virtual_workspace item = { .id = UINT64_MAX, .sid = UINT64_MAX - 1, .did = UINT32_MAX, .index = INT_MAX };
        buf_push(records, item);
        NSData *data = virtual_workspace_state_encode(records, 0, UINT64_MAX);
        uint64_t next = 1, generation = 0;
        TEST_CHECK(data != nil, true);
        TEST_CHECK(virtual_workspace_state_decode(data, &decoded, &next, &generation), 0);
        TEST_CHECK(next == 0 && generation == UINT64_MAX, true);
        TEST_CHECK(decoded[0].id == UINT64_MAX && decoded[0].sid == UINT64_MAX - 1, true);
        TEST_CHECK(decoded[0].index, INT_MAX);
        buf_free(decoded);
        buf_free(records);
        uint64_t value;
        for (id invalid in @[ @1, @YES, @"01", @"-1", @"+1", @"1e2", @"18446744073709551616", @"1\0x", @"" ]) {
            TEST_CHECK(virtual_workspace_state_number(invalid, &value), false);
        }
    }
});

TEST_FUNC(virtual_workspace_state_restart_lifecycle,
{
    @autoreleasepool {
        virtual_workspace_state_test_reset();
        char path[PATH_MAX];
        TEST_CHECK(virtual_workspace_state_test_directory(path), true);
        struct virtual_workspace_source source = virtual_workspace_state_test_source();
        struct virtual_workspace_manager vm = {0};
        TEST_CHECK(virtual_workspace_manager_begin_state(&vm, &source, path), true);
        TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_SUCCESS);
        TEST_CHECK(buf_last(vm.workspaces).id == 5 && buf_last(vm.workspaces).sid == 80, true);
        uint64_t generation = vm.store->generation;
        TEST_CHECK(virtual_workspace_manager_focus(&vm, 5), VIRTUAL_WORKSPACE_SUCCESS);
        TEST_CHECK(vm.store->generation == generation, true);
        virtual_workspace_test_state.active_sid = 300;
        virtual_workspace_state_test_close(&vm);
        TEST_CHECK(virtual_workspace_manager_begin_state(&vm, &source, path), true);
        TEST_CHECK(buf_len(vm.workspaces), 5);
        TEST_CHECK(vm.workspaces[4].id == 5 && vm.workspaces[4].index == 5 && vm.workspaces[4].sid == 80, true);
        /* Isolated final metadata boundary, already covered with native evacuation below. */
        TEST_CHECK(virtual_workspace_manager_release(&vm, &vm.workspaces[2]), VIRTUAL_WORKSPACE_SUCCESS);
        TEST_CHECK(virtual_workspace_manager_release(&vm, &vm.workspaces[3]), VIRTUAL_WORKSPACE_SUCCESS);
        virtual_workspace_state_test_close(&vm);
        /* Deleted occupied 500 is rediscovered with a fresh ID at the end. */
        TEST_CHECK(virtual_workspace_manager_begin_state(&vm, &source, path), true);
        TEST_CHECK(vm.workspaces[0].id == 1 && vm.workspaces[1].id == 2 && vm.workspaces[2].id == 4, true);
        TEST_CHECK(vm.workspaces[2].index, 4);
        TEST_CHECK(buf_last(vm.workspaces).id == 6 && buf_last(vm.workspaces).sid == 500, true);
        TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_SUCCESS);
        TEST_CHECK(buf_last(vm.workspaces).id == 7 && buf_last(vm.workspaces).index == 6, true);
        struct virtual_workspace_refresh_entry *entries = NULL;
        TEST_CHECK(virtual_workspace_manager_refresh(&vm, &entries), VIRTUAL_WORKSPACE_SUCCESS);
        buf_free(entries);
        virtual_workspace_state_test_close(&vm);
        TEST_CHECK(virtual_workspace_manager_begin_state(&vm, &source, path), true);
        TEST_CHECK(buf_len(vm.workspaces), 4);
        for (int i = 0; i < buf_len(vm.workspaces); ++i) TEST_CHECK(vm.workspaces[i].index, i + 1);
        TEST_CHECK(vm.next_id == 8, true);
        TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_SUCCESS);
        TEST_CHECK(buf_last(vm.workspaces).id == 8, true);
        virtual_workspace_state_test_close(&vm);
        virtual_workspace_state_test_remove(path);
    }
});

TEST_FUNC(virtual_workspace_state_stable_identity_and_order,
{
    @autoreleasepool {
        virtual_workspace_state_test_reset();
        char path[PATH_MAX];
        TEST_CHECK(virtual_workspace_state_test_directory(path), true);
        struct virtual_workspace_source source = virtual_workspace_state_test_source();
        struct virtual_workspace_manager vm = {0};
        TEST_CHECK(virtual_workspace_manager_begin_state(&vm, &source, path), true);
        struct virtual_workspace swap = vm.workspaces[1];
        vm.workspaces[1] = vm.workspaces[2];
        vm.workspaces[2] = swap;
        for (int i = 0; i < 4; ++i) vm.workspaces[i].index = 2 * i + 1;
        TEST_CHECK(virtual_workspace_state_commit(vm.store, vm.workspaces, vm.next_id), 1);
        virtual_workspace_state_test_close(&vm);
        virtual_workspace_test_state.displays[0] = 77;
        virtual_workspace_test_state.active_did = 77;
        virtual_workspace_test_state.spaces[0][2] = 9090;
        virtual_workspace_test_state.visible_sid[0] = virtual_workspace_test_state.active_sid = 9090;
        virtual_workspace_state_test_identity.remap_from = 300;
        virtual_workspace_state_test_identity.remap_to = 9090;
        TEST_CHECK(virtual_workspace_manager_begin_state(&vm, &source, path), true);
        TEST_CHECK(buf_len(vm.workspaces), 4);
        TEST_CHECK(vm.workspaces[0].id == 1 && vm.workspaces[1].id == 3 && vm.workspaces[2].id == 2 && vm.workspaces[3].id == 4, true);
        TEST_CHECK(vm.workspaces[2].sid == 9090 && vm.workspaces[2].did == 77 && vm.workspaces[2].index == 5, true);
        TEST_CHECK(virtual_workspace_manager_focus(&vm, 5), VIRTUAL_WORKSPACE_SUCCESS);
        TEST_CHECK(virtual_workspace_test_state.focused_sid == 9090, true);
        virtual_workspace_state_test_close(&vm);
        virtual_workspace_state_test_remove(path);
    }
});

TEST_FUNC(virtual_workspace_state_disconnect_and_reconnect,
{
    @autoreleasepool {
        virtual_workspace_state_test_reset();
        char path[PATH_MAX];
        TEST_CHECK(virtual_workspace_state_test_directory(path), true);
        struct virtual_workspace_source source = virtual_workspace_state_test_source();
        struct virtual_workspace_manager vm = {0};
        TEST_CHECK(virtual_workspace_manager_begin_state(&vm, &source, path), true);
        virtual_workspace_state_test_close(&vm);
        virtual_workspace_test_state.display_count = 1;
        TEST_CHECK(virtual_workspace_manager_begin_state(&vm, &source, path), true);
        TEST_CHECK(vm.workspaces[2].unresolved && vm.workspaces[3].unresolved, true);
        TEST_CHECK(virtual_workspace_manager_find_space(&vm, 500) == NULL, true);
        TEST_CHECK(virtual_workspace_manager_focus(&vm, 3), VIRTUAL_WORKSPACE_SPACE_UNAVAILABLE);
        TEST_CHECK(virtual_workspace_manager_destroy(&vm, 1, 3, NULL, NULL), VIRTUAL_WORKSPACE_INVALID_TARGET);
        struct virtual_workspace_refresh_entry *entries = NULL;
        TEST_CHECK(virtual_workspace_manager_refresh(&vm, &entries), VIRTUAL_WORKSPACE_SUCCESS);
        TEST_CHECK(entries[2].retention, VIRTUAL_WORKSPACE_RETAIN_MAPPING);
        buf_free(entries);
        TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_SUCCESS);
        TEST_CHECK(buf_last(vm.workspaces).id == 5, true);
        virtual_workspace_state_test_close(&vm);
        virtual_workspace_test_state.display_count = 2;
        virtual_workspace_test_state.displays[1] = 66;
        TEST_CHECK(virtual_workspace_manager_begin_state(&vm, &source, path), true);
        TEST_CHECK(vm.workspaces[2].id == 3 && vm.workspaces[2].did == 66 && !vm.workspaces[2].unresolved, true);
        TEST_CHECK(buf_len(vm.workspaces), 5);
        virtual_workspace_state_test_close(&vm);
        virtual_workspace_state_test_remove(path);
    }
});

TEST_FUNC(virtual_workspace_state_reused_and_ambiguous_native_identity,
{
    @autoreleasepool {
        for (int scenario = 0; scenario < 4; ++scenario) {
            virtual_workspace_state_test_reset();
            char path[PATH_MAX];
            TEST_CHECK(virtual_workspace_state_test_directory(path), true);
            struct virtual_workspace_source source = virtual_workspace_state_test_source();
            struct virtual_workspace_manager vm = {0};
            TEST_CHECK(virtual_workspace_manager_begin_state(&vm, &source, path), true);
            virtual_workspace_state_test_close(&vm);
            if (scenario == 0) virtual_workspace_state_test_identity.changed = 700;
            if (scenario == 1) virtual_workspace_state_test_identity.missing = 700;
            if (scenario == 2) virtual_workspace_state_test_identity.duplicate = 80;
            if (scenario == 3) virtual_workspace_state_test_identity.duplicate_display = true;
            TEST_CHECK(virtual_workspace_manager_begin_state(&vm, &source, path), true);
            TEST_CHECK(vm.workspaces[0].id == 1 && vm.workspaces[0].unresolved, true);
            TEST_CHECK(virtual_workspace_manager_focus(&vm, 1), VIRTUAL_WORKSPACE_SPACE_UNAVAILABLE);
            struct virtual_workspace *active = virtual_workspace_manager_find_space(&vm, 700);
            TEST_CHECK(scenario == 0 ? active && active->id == 5 : active == NULL, true);
            TEST_CHECK(virtual_workspace_manager_persistence(&vm)[0] == 'd', true);
            int count = buf_len(vm.workspaces);
            virtual_workspace_state_test_close(&vm);
            TEST_CHECK(virtual_workspace_manager_begin_state(&vm, &source, path), true);
            TEST_CHECK(buf_len(vm.workspaces), count);
            virtual_workspace_state_test_close(&vm);
            virtual_workspace_state_test_remove(path);
        }
    }
});

TEST_FUNC(virtual_workspace_state_invalid_records,
{
    @autoreleasepool {
        virtual_workspace_state_test_reset();
        struct virtual_workspace_source source = virtual_workspace_state_test_source();
        struct virtual_workspace_manager vm = {0};
        TEST_CHECK(virtual_workspace_manager_begin(&vm, &source), true);
        NSData *original = virtual_workspace_state_encode(vm.workspaces, vm.next_id, 1);
        for (int fault = 0; fault < 11; ++fault) {
            NSMutableDictionary *root = [NSJSONSerialization JSONObjectWithData:original options:NSJSONReadingMutableContainers error:NULL];
            NSMutableArray *array = root[@"workspaces"];
            NSMutableDictionary *first = array[0], *second = array[1];
            switch (fault) {
            case 0: second[@"id"] = first[@"id"]; break;
            case 1: second[@"index"] = first[@"index"]; break;
            case 2: first[@"index"] = @"3"; break;
            case 3: second[@"space-uuid"] = first[@"space-uuid"]; break;
            case 4: first[@"display-id"] = @"4294967296"; break;
            case 5: first[@"id"] = @"18446744073709551616"; break;
            case 6: first[@"space-id"] = @700; break;
            case 7: first[@"space-uuid"] = @"not-a-uuid"; break;
            case 8: root[@"next-id"] = @"3"; break;
            case 9: first[@"index"] = @"2147483648"; break;
            case 10: root[@"generation"] = @"0"; break;
            }
            NSData *data = [NSJSONSerialization dataWithJSONObject:root options:0 error:NULL];
            struct virtual_workspace *decoded = NULL;
            uint64_t next, generation;
            TEST_CHECK(virtual_workspace_state_decode(data, &decoded, &next, &generation), -1);
            buf_free(decoded);
        }
        virtual_workspace_state_test_close(&vm);
    }
});

TEST_FUNC(virtual_workspace_state_recovery_and_future_schema,
{
    @autoreleasepool {
        for (int scenario = 0; scenario < 4; ++scenario) {
            virtual_workspace_state_test_reset();
            char path[PATH_MAX];
            TEST_CHECK(virtual_workspace_state_test_directory(path), true);
            NSData *original = scenario == 0 || scenario == 3 ? [@"{truncated" dataUsingEncoding:NSUTF8StringEncoding] :
                               scenario == 1 ? [NSMutableData dataWithLength:VIRTUAL_WORKSPACE_STATE_LIMIT + 1] :
                               [@"{\"version\":\"2\",\"unknown\":true}" dataUsingEncoding:NSUTF8StringEncoding];
            TEST_CHECK(virtual_workspace_state_test_put(path, original), true);
            if (scenario == 3) {
                char lock_path[PATH_MAX];
                snprintf(lock_path, sizeof(lock_path), "%s/state.lock", path);
                int lock = open(lock_path, O_CREAT | O_RDWR, 0600);
                if (lock >= 0) close(lock);
                chmod(path, 0500); /* Readable invalid state, recovery link cannot be created. */
            }
            struct virtual_workspace_source source = virtual_workspace_state_test_source();
            struct virtual_workspace_manager vm = {0};
            TEST_CHECK(virtual_workspace_manager_begin_state(&vm, &source, path), true);
            TEST_CHECK(vm.store->session_only, scenario >= 2);
            TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_SUCCESS);
            if (scenario >= 2) { TEST_CHECK([original isEqualToData:virtual_workspace_state_test_data(path)], true); }
            else {
                DIR *directory = opendir(path);
                struct dirent *entry;
                int found = 0;
                while ((entry = readdir(directory))) {
                    if (strncmp(entry->d_name, "state.recovery-", 15)) continue;
                    NSData *saved = [NSData dataWithContentsOfFile:[[NSString stringWithUTF8String:path] stringByAppendingPathComponent:[NSString stringWithUTF8String:entry->d_name]]];
                    TEST_CHECK([original isEqualToData:saved], true);
                    ++found;
                }
                closedir(directory);
                TEST_CHECK(found, 1);
            }
            virtual_workspace_state_test_close(&vm);
            chmod(path, 0700);
            virtual_workspace_state_test_remove(path);
        }
    }
});

TEST_FUNC(virtual_workspace_state_commit_boundaries,
{
    @autoreleasepool {
        for (int fault = 1; fault <= 3; ++fault) {
            virtual_workspace_state_test_reset();
            char path[PATH_MAX];
            TEST_CHECK(virtual_workspace_state_test_directory(path), true);
            struct virtual_workspace_source source = virtual_workspace_state_test_source();
            struct virtual_workspace_manager vm = {0};
            TEST_CHECK(virtual_workspace_manager_begin_state(&vm, &source, path), true);
            NSData *original = virtual_workspace_state_test_data(path);
            vm.store->fault = fault;
            TEST_CHECK(virtual_workspace_manager_create(&vm), fault < 3 ? VIRTUAL_WORKSPACE_PERSISTENCE_FAILED : VIRTUAL_WORKSPACE_DURABILITY_UNCERTAIN);
            TEST_CHECK(buf_len(vm.workspaces), fault < 3 ? 4 : 5);
            TEST_CHECK(vm.next_id == (fault < 3 ? 5 : 6), true);
            TEST_CHECK([original isEqualToData:virtual_workspace_state_test_data(path)], fault < 3);
            TEST_CHECK(vm.store->uncertain, fault == 3);
            virtual_workspace_state_test_close(&vm);
            TEST_CHECK(virtual_workspace_manager_begin_state(&vm, &source, path), true);
            TEST_CHECK(buf_len(vm.workspaces), fault < 3 ? 4 : 5);
            if (fault < 3) TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_SUCCESS);
            TEST_CHECK(vm.next_id == 6 && buf_last(vm.workspaces).id == 5, true);
            vm.store->fault = fault;
            original = virtual_workspace_state_test_data(path);
            struct virtual_workspace_refresh_entry *entries = NULL;
            TEST_CHECK(virtual_workspace_manager_refresh(&vm, &entries), fault < 3 ? VIRTUAL_WORKSPACE_PERSISTENCE_FAILED : VIRTUAL_WORKSPACE_DURABILITY_UNCERTAIN);
            TEST_CHECK(buf_len(vm.workspaces), fault < 3 ? 5 : 4);
            TEST_CHECK([original isEqualToData:virtual_workspace_state_test_data(path)], fault < 3);
            buf_free(entries);
            virtual_workspace_state_test_close(&vm);
            virtual_workspace_state_test_remove(path);
        }
    }
});

TEST_FUNC(virtual_workspace_state_writer_ownership,
{
    @autoreleasepool {
        virtual_workspace_state_test_reset();
        char path[PATH_MAX];
        TEST_CHECK(virtual_workspace_state_test_directory(path), true);
        struct virtual_workspace_source source = virtual_workspace_state_test_source();
        struct virtual_workspace_manager first = {0}, second = {0};
        TEST_CHECK(virtual_workspace_manager_begin_state(&first, &source, path), true);
        NSData *original = virtual_workspace_state_test_data(path);
        TEST_CHECK(virtual_workspace_manager_begin_state(&second, &source, path), true);
        TEST_CHECK(second.store->session_only, true);
        TEST_CHECK(virtual_workspace_manager_create(&second), VIRTUAL_WORKSPACE_SUCCESS);
        TEST_CHECK([original isEqualToData:virtual_workspace_state_test_data(path)], true);
        virtual_workspace_state_test_close(&second);
        TEST_CHECK(virtual_workspace_manager_create(&first), VIRTUAL_WORKSPACE_SUCCESS);
        virtual_workspace_state_test_close(&first);
        TEST_CHECK(virtual_workspace_manager_begin_state(&second, &source, path), true);
        TEST_CHECK(!second.store->session_only && buf_last(second.workspaces).id == 5, true);
        virtual_workspace_state_test_close(&second);
        virtual_workspace_state_test_remove(path);
    }
});

TEST_FUNC(virtual_workspace_state_destroy_failure_and_restart,
{
    @autoreleasepool {
        for (int fault = 1; fault <= 3; ++fault) {
            virtual_workspace_state_test_reset();
            char path[PATH_MAX];
            TEST_CHECK(virtual_workspace_state_test_directory(path), true);
            struct virtual_workspace_manager vm = {0};
            TEST_CHECK(virtual_workspace_destroy_test_begin(&vm), true);
            struct virtual_workspace_source source = virtual_workspace_destroy_test_source;
            source.identity = virtual_workspace_state_test_native_identity;
            vm.source = &source;
            for (int i = 0; i < buf_len(vm.workspaces); ++i) source.identity(vm.workspaces[i].sid, vm.workspaces[i].did, vm.workspaces[i].space_uuid, vm.workspaces[i].display_uuid);
            struct virtual_workspace *unused = NULL;
            uint64_t next = 1;
            vm.store = virtual_workspace_state_open(path, &unused, &next);
            TEST_CHECK(virtual_workspace_state_commit(vm.store, vm.workspaces, vm.next_id), 1);
            virtual_workspace_destroy_test_add(10, 700);
            TEST_CHECK(virtual_workspace_manager_destroy(&vm, 1, 2, NULL, virtual_workspace_destroy_test_complete), VIRTUAL_WORKSPACE_PENDING);
            vm.store->fault = fault;
            virtual_workspace_destroy_test_drain(&vm);
            TEST_CHECK(virtual_workspace_destroy_test.result, fault < 3 ? VIRTUAL_WORKSPACE_PERSISTENCE_FAILED : VIRTUAL_WORKSPACE_DURABILITY_UNCERTAIN);
            TEST_CHECK(virtual_workspace_destroy_test.windows[0].sid == 300, true);
            TEST_CHECK(virtual_workspace_destroy_test.moved, 1);
            TEST_CHECK(virtual_workspace_manager_find_index(&vm, 1) != NULL, fault < 3);
            virtual_workspace_state_test_close(&vm);
            TEST_CHECK(virtual_workspace_manager_begin_state(&vm, &source, path), true);
            TEST_CHECK(virtual_workspace_manager_find_index(&vm, 1) != NULL, fault < 3);
            if (fault < 3) {
                TEST_CHECK(virtual_workspace_manager_destroy(&vm, 1, 2, NULL, virtual_workspace_destroy_test_complete), VIRTUAL_WORKSPACE_PENDING);
                virtual_workspace_destroy_test_drain(&vm);
                TEST_CHECK(virtual_workspace_destroy_test.result, VIRTUAL_WORKSPACE_SUCCESS);
            }
            virtual_workspace_state_test_close(&vm);
            virtual_workspace_state_test_remove(path);
        }
    }
});

TEST_FUNC(virtual_workspace_state_crash_boundaries,
{
    @autoreleasepool {
        for (int fault = 4; fault <= 5; ++fault) {
            virtual_workspace_state_test_reset();
            char path[PATH_MAX];
            TEST_CHECK(virtual_workspace_state_test_directory(path), true);
            struct virtual_workspace_source source = virtual_workspace_state_test_source();
            struct virtual_workspace_manager vm = {0};
            TEST_CHECK(virtual_workspace_manager_begin_state(&vm, &source, path), true);
            pid_t child = fork();
            TEST_CHECK(child >= 0, true);
            if (child == 0) {
                vm.store->fault = fault;
                virtual_workspace_manager_create(&vm);
                _exit(1);
            }
            int status = -1;
            TEST_CHECK(waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0, true);
            virtual_workspace_state_test_close(&vm);
            TEST_CHECK(virtual_workspace_manager_begin_state(&vm, &source, path), true);
            TEST_CHECK(buf_len(vm.workspaces), fault == 4 ? 4 : 5);
            TEST_CHECK(vm.next_id == (fault == 4 ? 5 : 6), true);
            DIR *directory = opendir(path);
            struct dirent *entry;
            while ((entry = readdir(directory))) TEST_CHECK(strncmp(entry->d_name, ".state-tmp-", 11) == 0, false);
            closedir(directory);
            virtual_workspace_state_test_close(&vm);
            virtual_workspace_state_test_remove(path);
        }
    }
});

TEST_FUNC(virtual_workspace_state_crash_during_destroy,
{
    @autoreleasepool {
        virtual_workspace_state_test_reset();
        char path[PATH_MAX];
        TEST_CHECK(virtual_workspace_state_test_directory(path), true);
        struct virtual_workspace_manager vm = {0};
        TEST_CHECK(virtual_workspace_destroy_test_begin(&vm), true);
        struct virtual_workspace_source source = virtual_workspace_destroy_test_source;
        source.identity = virtual_workspace_state_test_native_identity;
        vm.source = &source;
        for (int i = 0; i < buf_len(vm.workspaces); ++i) source.identity(vm.workspaces[i].sid, vm.workspaces[i].did, vm.workspaces[i].space_uuid, vm.workspaces[i].display_uuid);
        struct virtual_workspace *unused = NULL;
        uint64_t next = 1;
        vm.store = virtual_workspace_state_open(path, &unused, &next);
        TEST_CHECK(virtual_workspace_state_commit(vm.store, vm.workspaces, vm.next_id), 1);
        NSData *original = virtual_workspace_state_test_data(path);
        virtual_workspace_destroy_test_add(10, 700);
        virtual_workspace_destroy_test_add(11, 700);
        TEST_CHECK(virtual_workspace_manager_destroy(&vm, 1, 2, NULL, NULL), VIRTUAL_WORKSPACE_PENDING);
        virtual_workspace_manager_destroy_continue(&vm, vm.destroy.token);
        TEST_CHECK(virtual_workspace_destroy_test.windows[0].sid == 300 && virtual_workspace_destroy_test.windows[1].sid == 700, true);
        TEST_CHECK([original isEqualToData:virtual_workspace_state_test_data(path)], true);
        virtual_workspace_state_test_close(&vm);
        TEST_CHECK(virtual_workspace_manager_begin_state(&vm, &source, path), true);
        TEST_CHECK(virtual_workspace_manager_find_index(&vm, 1)->id == 1, true);
        TEST_CHECK(virtual_workspace_manager_destroy(&vm, 1, 2, NULL, virtual_workspace_destroy_test_complete), VIRTUAL_WORKSPACE_PENDING);
        virtual_workspace_destroy_test_drain(&vm);
        TEST_CHECK(virtual_workspace_destroy_test.result, VIRTUAL_WORKSPACE_SUCCESS);
        TEST_CHECK(virtual_workspace_destroy_test.windows[0].moves, 1);
        TEST_CHECK(virtual_workspace_destroy_test.windows[1].moves, 1);
        virtual_workspace_state_test_close(&vm);
        virtual_workspace_state_test_remove(path);
    }
});

TEST_FUNC(virtual_workspace_state_allocation_and_permission_failures,
{
    @autoreleasepool {
        virtual_workspace_state_test_reset();
        char path[PATH_MAX];
        TEST_CHECK(virtual_workspace_state_test_directory(path), true);
        struct virtual_workspace_source source = virtual_workspace_state_test_source();
        struct virtual_workspace_manager vm = {0};
        TEST_CHECK(virtual_workspace_manager_begin_state(&vm, &source, path), true);
        NSData *original = virtual_workspace_state_test_data(path);
        virtual_workspace_buffer_fail_after = 2;
        TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_LIMIT);
        virtual_workspace_buffer_fail_after = -1;
        TEST_CHECK(buf_len(vm.workspaces), 4);
        TEST_CHECK([original isEqualToData:virtual_workspace_state_test_data(path)], true);
        chmod(path, 0500);
        TEST_CHECK(virtual_workspace_manager_create(&vm), VIRTUAL_WORKSPACE_PERSISTENCE_FAILED);
        TEST_CHECK(buf_len(vm.workspaces), 4);
        TEST_CHECK([original isEqualToData:virtual_workspace_state_test_data(path)], true);
        chmod(path, 0700);
        virtual_workspace_state_test_close(&vm);
        virtual_workspace_buffer_fail_after = 0;
        struct virtual_workspace *records = NULL;
        uint64_t next = 1;
        struct virtual_workspace_store *store = virtual_workspace_state_open(path, &records, &next);
        TEST_CHECK(store->session_only && records == NULL, true);
        TEST_CHECK([original isEqualToData:virtual_workspace_state_test_data(path)], true);
        virtual_workspace_buffer_fail_after = -1;
        virtual_workspace_state_close(store);
        TEST_CHECK(virtual_workspace_manager_begin_state(&vm, &source, path), true);
        virtual_workspace_state_test_close(&vm);
        virtual_workspace_state_test_remove(path);
    }
});

TEST_FUNC(virtual_workspace_state_unresolved_last_and_compaction,
{
    @autoreleasepool {
        virtual_workspace_state_test_reset();
        struct virtual_workspace_source source = virtual_workspace_state_test_source();
        struct virtual_workspace_manager vm = {0};
        TEST_CHECK(virtual_workspace_manager_begin(&vm, &source), true);
        vm.workspaces[0].unresolved = true;
        vm.workspaces[1].unresolved = true;
        vm.workspaces[2].unresolved = true;
        TEST_CHECK(virtual_workspace_manager_destroy(&vm, 4, 1, NULL, NULL), VIRTUAL_WORKSPACE_LAST);
        /* All visible desktops are intentionally unmapped; only 400 is usable. */
        source.occupancy = virtual_workspace_test_empty;
        vm.workspaces[3].index = 9;
        struct virtual_workspace_refresh_entry *entries = NULL;
        TEST_CHECK(virtual_workspace_manager_refresh(&vm, &entries), VIRTUAL_WORKSPACE_SUCCESS);
        TEST_CHECK(buf_len(vm.workspaces), 4);
        TEST_CHECK(vm.workspaces[3].index, 4);
        TEST_CHECK(entries[3].retention, VIRTUAL_WORKSPACE_RETAIN_LAST);
        TEST_CHECK(entries[0].retention, VIRTUAL_WORKSPACE_RETAIN_MAPPING);
        TEST_CHECK(virtual_workspace_manager_find_space(&vm, 700) == NULL, true);
        buf_free(entries);
        virtual_workspace_state_test_close(&vm);
    }
});

TEST_FUNC(vspace_persistence_status_and_unresolved,
{
    @autoreleasepool {
        TEST_CHECK(vspace_test_begin(), true);
        g_virtual_workspace_manager.workspaces[0].unresolved = true;
        TEST_CHECK(vspace_test_response("vspace\0--focus\0" "1\0", FAILURE_MESSAGE "cannot focus virtual workspace because its native space or display is unavailable or unresolved.\n"), true);
        char *response = NULL;
        size_t length = 0;
        FILE *rsp = open_memstream(&response, &length);
        char message[] = "vspace\0--list\0";
        handle_message(rsp, message);
        fclose(rsp);
        TEST_CHECK(strstr(response, "\"availability\":\"unresolved\"") != NULL, true);
        TEST_CHECK(strstr(response, "\"persistence\":\"session-only\"") != NULL, true);
        free(response);
        vspace_test_end();
    }
});

TEST_FUNC(vspace_persistence_commit_errors,
{
    @autoreleasepool {
        virtual_workspace_state_test_reset();
        char path[PATH_MAX];
        TEST_CHECK(virtual_workspace_state_test_directory(path), true);
        struct virtual_workspace_source source = virtual_workspace_state_test_source();
        struct virtual_workspace_manager *vm = &g_virtual_workspace_manager;
        TEST_CHECK(virtual_workspace_manager_begin_state(vm, &source, path), true);
        vm->store->fault = 1;
        TEST_CHECK(vspace_test_response("vspace\0--create\0", FAILURE_MESSAGE "state snapshot could not be replaced; metadata commit rejected.\n"), true);
        TEST_CHECK(buf_len(vm->workspaces), 4);
        vm->store->fault = 3;
        TEST_CHECK(vspace_test_response("vspace\0--create\0", FAILURE_MESSAGE "state snapshot replaced and metadata committed; durability is uncertain; inspect --list before retrying.\n"), true);
        TEST_CHECK(buf_len(vm->workspaces), 5);
        TEST_CHECK(vspace_test_response("vspace\0--refresh\0", FAILURE_MESSAGE "state snapshot replaced and metadata committed; durability is uncertain; inspect --list before retrying.\n"), true);
        TEST_CHECK(buf_len(vm->workspaces), 4);
        for (int committed = 0; committed < 2; ++committed) {
            char *response = NULL;
            size_t length = 0;
            FILE *rsp = open_memstream(&response, &length);
            struct virtual_workspace_window window = { .wid = 10, .moved = true };
            struct virtual_workspace_destroy operation = { .context = rsp };
            buf_push(operation.windows, window);
            vspace_destroy_complete(&operation, committed ? VIRTUAL_WORKSPACE_DURABILITY_UNCERTAIN : VIRTUAL_WORKSPACE_PERSISTENCE_FAILED);
            TEST_CHECK(strstr(response, committed ? "removal committed" : "moved windows: [10]; source mapping retained.") != NULL, true);
            TEST_CHECK(strstr(response, "source mapping retained") != NULL, !committed);
            buf_free(operation.windows);
            free(response);
        }
        virtual_workspace_state_test_close(vm);
        virtual_workspace_state_test_remove(path);
    }
});
