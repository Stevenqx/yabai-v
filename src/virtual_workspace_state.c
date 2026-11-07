/* Included by manifest.m. State is separate from configuration and native state. */
#include <sys/file.h>
#include <errno.h>
#include <limits.h>

#define VIRTUAL_WORKSPACE_STATE_LIMIT (1024 * 1024)
#define VIRTUAL_WORKSPACE_RECORD_LIMIT 4096

struct virtual_workspace_store
{
    int directory;
    int lock;
    uint64_t generation;
    bool session_only;
    bool uncertain;
#ifdef TESTS
    int fault;
#endif
};

/* Unlike upstream's buffer helper, candidate construction must handle ENOMEM. */
#ifdef TESTS
static int virtual_workspace_buffer_fail_after = -1;
#endif

static bool virtual_workspace_buffer_grow(void **buffer, size_t size)
{
#ifdef TESTS
    if (virtual_workspace_buffer_fail_after == 0) return false;
    if (virtual_workspace_buffer_fail_after > 0) --virtual_workspace_buffer_fail_after;
#endif
    if (buf_len(*buffer) >= VIRTUAL_WORKSPACE_RECORD_LIMIT) return false;
    if (buf_len(*buffer) < buf_cap(*buffer)) return true;
    int capacity = min(VIRTUAL_WORKSPACE_RECORD_LIMIT, 1 + 2 * buf_cap(*buffer));
    struct buf_hdr *header = realloc(*buffer ? buf__hdr(*buffer) : NULL, sizeof(*header) + capacity * size);
    if (!header) return false;
    header->cap = capacity;
    if (!*buffer) header->len = 0;
    *buffer = header->buf;
    return true;
}

#define virtual_workspace_push(b, ...) (virtual_workspace_buffer_grow((void **)&(b), sizeof(*(b))) ? ((b)[buf__hdr(b)->len++] = (__VA_ARGS__), true) : false)

static bool virtual_workspace_uuid(const char *value)
{
    if (strlen(value) != 36) return false;
    for (int i = 0; i < 36; ++i) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            if (value[i] != '-') return false;
        } else if (!((value[i] >= '0' && value[i] <= '9') || (value[i] >= 'A' && value[i] <= 'F'))) return false;
    }
    return strcmp(value, "00000000-0000-0000-0000-000000000000") != 0;
}

static bool virtual_workspace_state_number(id value, uint64_t *number)
{
    if (![value isKindOfClass:[NSString class]]) return false;
    const char *text = [value UTF8String];
    if (!text || strlen(text) != [value lengthOfBytesUsingEncoding:NSUTF8StringEncoding] || !*text || (text[0] == '0' && text[1])) return false;
    uint64_t result = 0;
    for (const char *c = text; *c; ++c) {
        if (*c < '0' || *c > '9' || result > (UINT64_MAX - (*c - '0')) / 10) return false;
        result = result * 10 + (*c - '0');
    }
    *number = result;
    return true;
}

static NSString *virtual_workspace_state_decimal(uint64_t value)
{
    return [NSString stringWithFormat:@"%llu", (unsigned long long)value];
}

static bool virtual_workspace_state_uuid(id value, char *uuid)
{
    if (![value isKindOfClass:[NSString class]]) return false;
    const char *text = [value UTF8String];
    if (!text || strlen(text) != [value lengthOfBytesUsingEncoding:NSUTF8StringEncoding] || (*text && !virtual_workspace_uuid(text))) return false;
    strlcpy(uuid, text, 37);
    return true;
}

/* Array order is global relative order; strictly increasing indices preserve gaps. */
static bool virtual_workspace_state_valid(struct virtual_workspace *records, uint64_t next_id)
{
    if (!buf_len(records) || buf_len(records) > VIRTUAL_WORKSPACE_RECORD_LIMIT) return false;
    int previous = 0;
    for (int i = 0; i < buf_len(records); ++i) {
        struct virtual_workspace *record = &records[i];
        if (!record->id || !record->sid || !record->did || record->index <= previous ||
            (next_id && next_id <= record->id) ||
            (*record->space_uuid && !virtual_workspace_uuid(record->space_uuid)) ||
            (*record->display_uuid && !virtual_workspace_uuid(record->display_uuid)) ||
            (!!*record->space_uuid != !!*record->display_uuid)) return false;
        previous = record->index;
        for (int j = 0; j < i; ++j) {
            if (record->id == records[j].id) return false;
            /* UUID is native identity; numeric hints may be reused across boots. */
            if (*record->space_uuid && !strcmp(record->space_uuid, records[j].space_uuid)) return false;
            if (!*record->space_uuid && !*records[j].space_uuid && record->sid == records[j].sid && record->did == records[j].did) return false;
        }
    }
    return true;
}

/* -1: malformed supported schema, 0: valid, 1: future schema. */
static int virtual_workspace_state_decode(NSData *data, struct virtual_workspace **records, uint64_t *next_id, uint64_t *generation)
{
    @try {
        id root = [NSJSONSerialization JSONObjectWithData:data options:0 error:NULL];
        if (![root isKindOfClass:[NSDictionary class]]) return -1;
        uint64_t version;
        if (!virtual_workspace_state_number(root[@"version"], &version) || !version) return -1;
        if (version > 1) return 1;
        id array = root[@"workspaces"];
        if ([root count] != 4 || ![array isKindOfClass:[NSArray class]] || ![array count] ||
            [array count] > VIRTUAL_WORKSPACE_RECORD_LIMIT ||
            !virtual_workspace_state_number(root[@"next-id"], next_id) ||
            !virtual_workspace_state_number(root[@"generation"], generation) || !*generation) return -1;
        for (id item in array) {
            struct virtual_workspace record = { .unresolved = true };
            uint64_t index, did;
            if (![item isKindOfClass:[NSDictionary class]] || [item count] != 6 ||
                !virtual_workspace_state_number(item[@"id"], &record.id) ||
                !virtual_workspace_state_number(item[@"space-id"], &record.sid) ||
                !virtual_workspace_state_number(item[@"display-id"], &did) || did > UINT32_MAX ||
                !virtual_workspace_state_number(item[@"index"], &index) || index > INT_MAX ||
                !virtual_workspace_state_uuid(item[@"space-uuid"], record.space_uuid) ||
                !virtual_workspace_state_uuid(item[@"display-uuid"], record.display_uuid)) return -1;
            record.did = (uint32_t)did;
            record.index = (int)index;
            if (!virtual_workspace_push(*records, record)) return -2;
        }
        return virtual_workspace_state_valid(*records, *next_id) ? 0 : -1;
    } @catch (NSException *exception) {
        return -2;
    }
}

static NSData *virtual_workspace_state_encode(struct virtual_workspace *records, uint64_t next_id, uint64_t generation)
{
    @try {
        if (!generation || !virtual_workspace_state_valid(records, next_id)) return nil;
        NSMutableArray *array = [NSMutableArray arrayWithCapacity:buf_len(records)];
        for (int i = 0; i < buf_len(records); ++i) {
            struct virtual_workspace *record = &records[i];
            [array addObject:@{ @"id": virtual_workspace_state_decimal(record->id),
                                @"index": virtual_workspace_state_decimal(record->index),
                                @"space-id": virtual_workspace_state_decimal(record->sid),
                                @"display-id": virtual_workspace_state_decimal(record->did),
                                @"space-uuid": [NSString stringWithUTF8String:record->space_uuid],
                                @"display-uuid": [NSString stringWithUTF8String:record->display_uuid] }];
        }
        NSData *data = [NSJSONSerialization dataWithJSONObject:@{ @"version": @"1", @"generation": virtual_workspace_state_decimal(generation),
                                                                @"next-id": virtual_workspace_state_decimal(next_id), @"workspaces": array }
                                                      options:0 error:NULL];
        return [data length] <= VIRTUAL_WORKSPACE_STATE_LIMIT ? data : nil;
    } @catch (NSException *exception) {
        return nil;
    }
}

static bool virtual_workspace_state_write(int fd, const void *bytes, size_t length)
{
    while (length) {
        ssize_t count = write(fd, bytes, length);
        if (count < 0 && errno == EINTR) continue;
        if (count <= 0) return false;
        bytes = (const char *)bytes + count;
        length -= count;
    }
    return true;
}

static bool virtual_workspace_state_sync_directory(int fd)
{
    if (fsync(fd) == 0) return true;
    /* APFS/macOS versions may not support syncing directories. */
    return errno == EINVAL || errno == ENOTSUP;
}

static bool virtual_workspace_state_private(int fd, bool directory)
{
    struct stat info;
    return fstat(fd, &info) == 0 && info.st_uid == getuid() && !(info.st_mode & 0077) &&
           (directory ? S_ISDIR(info.st_mode) : S_ISREG(info.st_mode) && info.st_nlink == 1);
}

/* Walk from / with no symlink traversal, creating missing directories privately. */
static int virtual_workspace_state_directory(const char *path)
{
    if (!path || *path != '/' || strlen(path) >= PATH_MAX) return -1;
    char copy[PATH_MAX];
    strlcpy(copy, path, sizeof(copy));
    int fd = open("/", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (fd < 0) return -1;
    char *context = NULL;
    for (char *part = strtok_r(copy, "/", &context); part; part = strtok_r(NULL, "/", &context)) {
        if (!strcmp(part, ".") || !strcmp(part, "..")) { close(fd); return -1; }
        int made = mkdirat(fd, part, 0700);
        if (made != 0 && errno != EEXIST) { close(fd); return -1; }
        if (made == 0 && !virtual_workspace_state_sync_directory(fd)) { close(fd); return -1; }
        int child = openat(fd, part, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
        close(fd);
        if (child < 0) return -1;
        fd = child;
    }
    if (!virtual_workspace_state_private(fd, true)) { close(fd); return -1; }
    return fd;
}

static void virtual_workspace_state_close(struct virtual_workspace_store *store)
{
    if (!store) return;
    if (store->lock >= 0) close(store->lock);
    if (store->directory >= 0) close(store->directory);
    free(store);
}

static void virtual_workspace_state_session(struct virtual_workspace_store *store, const char *reason)
{
    store->session_only = true;
    warn("yabai: vspace persistence: %s; session-only fallback, logical changes will not survive restart; saved state will not be overwritten.\n", reason);
}

/* Keep original bytes via a private recovery name; replacement never edits inode data. */
static bool virtual_workspace_state_preserve(struct virtual_workspace_store *store)
{
    char name[80];
    for (int i = 0; i < 8; ++i) {
        snprintf(name, sizeof(name), "state.recovery-%08x.json", arc4random());
        if (linkat(store->directory, "state.json", store->directory, name, 0) == 0) {
            if (!virtual_workspace_state_sync_directory(store->directory)) return false;
            warn("yabai: vspace persistence: invalid state preserved as %s in the state directory.\n", name);
            return true;
        }
        if (errno != EEXIST) return false;
    }
    return false;
}

static void virtual_workspace_state_cleanup(struct virtual_workspace_store *store)
{
    int scan = dup(store->directory);
    DIR *directory = scan >= 0 ? fdopendir(scan) : NULL;
    if (!directory) { if (scan >= 0) close(scan); return; }
    struct dirent *entry;
    while ((entry = readdir(directory))) {
        if (strlen(entry->d_name) != 19 || strncmp(entry->d_name, ".state-tmp-", 11)) continue;
        bool valid = true;
        for (int i = 11; i < 19; ++i) if (!strchr("0123456789abcdef", entry->d_name[i])) valid = false;
        struct stat info;
        if (valid && fstatat(store->directory, entry->d_name, &info, AT_SYMLINK_NOFOLLOW) == 0 &&
            S_ISREG(info.st_mode) && info.st_uid == getuid() && !(info.st_mode & 0077) && info.st_nlink == 1) {
            unlinkat(store->directory, entry->d_name, 0);
        }
    }
    closedir(directory);
}

/* Called once, before logical initialization, with the lifetime writer lock held. */
static struct virtual_workspace_store *virtual_workspace_state_open(const char *path, struct virtual_workspace **records, uint64_t *next_id)
{
    struct virtual_workspace_store *store = calloc(1, sizeof(*store));
    if (!store) return NULL;
    store->directory = virtual_workspace_state_directory(path);
    store->lock = -1;
    if (store->directory < 0) { virtual_workspace_state_session(store, "state directory unavailable or not private"); return store; }
    store->lock = openat(store->directory, "state.lock", O_RDWR | O_CREAT | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK, 0600);
    if (store->lock < 0 || !virtual_workspace_state_private(store->lock, false) || flock(store->lock, LOCK_EX | LOCK_NB) != 0) {
        virtual_workspace_state_session(store, "state writer lock unavailable (including another socket/daemon owner)");
        return store;
    }
    virtual_workspace_state_cleanup(store);
    int fd = openat(store->directory, "state.json", O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_NONBLOCK);
    if (fd < 0) {
        if (errno != ENOENT) virtual_workspace_state_session(store, "saved state unreadable");
        return store;
    }
    struct stat info;
    if (fstat(fd, &info) != 0 || !S_ISREG(info.st_mode) || info.st_uid != getuid() || (info.st_mode & 0077)) {
        close(fd);
        virtual_workspace_state_session(store, "saved state is not a private regular file");
        return store;
    }
    int decoded = -1;
    if (info.st_size > 0 && info.st_size <= VIRTUAL_WORKSPACE_STATE_LIMIT) {
        NSMutableData *data = nil;
        @try { data = [NSMutableData dataWithLength:(NSUInteger)info.st_size]; }
        @catch (NSException *exception) { data = nil; }
        if (!data) { close(fd); virtual_workspace_state_session(store, "state allocation failed"); return store; }
        size_t offset = 0;
        while (offset < (size_t)info.st_size) {
            ssize_t count = read(fd, (char *)[data mutableBytes] + offset, (size_t)info.st_size - offset);
            if (count < 0 && errno == EINTR) continue;
            if (count <= 0) break;
            offset += count;
        }
        char extra;
        if (offset == (size_t)info.st_size && read(fd, &extra, 1) == 0) {
            decoded = virtual_workspace_state_decode(data, records, next_id, &store->generation);
        } else {
            close(fd);
            virtual_workspace_state_session(store, "saved state read incomplete");
            return store;
        }
    }
    close(fd);
    if (decoded == 0) return store;
    buf_free(*records);
    *records = NULL;
    *next_id = 1;
    store->generation = 0;
    if (decoded == -2) virtual_workspace_state_session(store, "state allocation failed");
    else if (decoded > 0) virtual_workspace_state_session(store, "newer unsupported state schema");
    else if (!virtual_workspace_state_preserve(store)) virtual_workspace_state_session(store, "could not preserve invalid state");
    return store;
}

/* 0 rejects before replacement, 1 commits, 2 commits with uncertain durability. */
static int virtual_workspace_state_commit(struct virtual_workspace_store *store, struct virtual_workspace *records, uint64_t next_id)
{
    if (!store || store->session_only) return 1;
    if (store->generation == UINT64_MAX) return 0;
    @autoreleasepool {
        NSData *data = virtual_workspace_state_encode(records, next_id, store->generation + 1);
        if (!data) return 0;
        char name[64];
        int fd = -1;
        for (int i = 0; i < 8 && fd < 0; ++i) {
            snprintf(name, sizeof(name), ".state-tmp-%08x", arc4random());
            fd = openat(store->directory, name, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
            if (fd < 0 && errno != EEXIST) break;
        }
        if (fd < 0) return 0;
        bool ready;
#ifdef TESTS
        if (store->fault == 1) {
            virtual_workspace_state_write(fd, [data bytes], [data length] / 2);
            errno = ENOSPC;
            ready = false;
        } else
#endif
        ready = virtual_workspace_state_write(fd, [data bytes], [data length]) && fsync(fd) == 0;
#ifdef TESTS
        if (store->fault == 4) _exit(0); /* Crash with a complete but unpublished temp. */
#endif
        if (ready && fcntl(fd, F_FULLFSYNC) != 0 && errno != ENOTSUP && errno != EINVAL) ready = false;
        if (close(fd) != 0) ready = false;
#ifdef TESTS
        if (store->fault == 2) ready = false;
#endif
        if (!ready || renameat(store->directory, name, store->directory, "state.json") != 0) {
            unlinkat(store->directory, name, 0);
            return 0;
        }
        /* Replacement is the commit point: callers MUST publish the candidate. */
        ++store->generation;
#ifdef TESTS
        if (store->fault == 5) _exit(0); /* Crash after replacement, before publication. */
#endif
        bool durable = virtual_workspace_state_sync_directory(store->directory);
#ifdef TESTS
        if (store->fault == 3) durable = false;
#endif
        store->uncertain = !durable;
        if (!durable) warn("yabai: vspace state replacement committed; directory durability uncertain; memory follows the committed generation.\n");
        return durable ? 1 : 2;
    }
}

const char *virtual_workspace_manager_persistence(struct virtual_workspace_manager *vm)
{
    if (!vm->store || vm->store->session_only) return "session-only";
    return vm->store->uncertain ? "durability-uncertain" : "durable";
}
