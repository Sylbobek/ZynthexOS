#include "fs/vfs/vfs.h"
#include "fs/vfs/path.h"
#include "lib/string.h"

#define MAX_MOUNTS 8

static vfs_mount_t mounts[MAX_MOUNTS];
static int mount_count = 0;
vfs_node_t* vfs_root = 0;

void vfs_init()
{
    // For now empty
}

vfs_node_t* vfs_lookup(const char* path)
{
    if (!vfs_root)
        return 0;

    return path_resolve(vfs_root, path);
}

void vfs_mount(const char* path, vfs_node_t* fs_root)
{
    vfs_node_t* mp = vfs_lookup(path);

    if (!mp)
        debug_panic("Mount path not found");

    mounts[mount_count].mountpoint = mp;
    mounts[mount_count].root = fs_root;

    mount_count++;
}

vfs_node_t* vfs_check_mount(vfs_node_t* node)
{
    for (int i = 0; i < mount_count; i++)
    {
        if (node == mounts[i].mountpoint)
            return mounts[i].root;
    }

    return node;
}
