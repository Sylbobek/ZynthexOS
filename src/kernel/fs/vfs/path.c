#include "path.h"
#include "lib/string.h"

vfs_node_t* path_resolve(vfs_node_t* root, const char* path)
{
    vfs_node_t* current = root;
    char part[64];

    while (*path)
    {
        while (*path == '/')
            path++;

        if (*path == '\0')
            break;

        int j = 0;
        while (*path && *path != '/')
            part[j++] = *path++;

        part[j] = 0;

        if (!current->ops->lookup)
            return 0;

        current = current->ops->lookup(current, part);
        if (!current)
            return 0;

        current = vfs_check_mount(current);
    }

    return current;
}
