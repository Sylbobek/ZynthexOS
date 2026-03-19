#include "fd.h"
#include "lib/string.h"
#include "core/debug.h"

static file_desc_t fd_table[MAX_FDS];

void fd_init()
{
    for (int i = 0; i < MAX_FDS; i++)
        fd_table[i].used = 0;
}

static int fd_alloc()
{
    for (int i = 0; i < MAX_FDS; i++)
    {
        if (!fd_table[i].used)
        {
            fd_table[i].used = 1;
            fd_table[i].offset = 0;
            return i;
        }
    }

    return -1;
}

int sys_open(const char* path)
{
    vfs_node_t* node = vfs_lookup(path);

    if (!node)
        return -1;

    int fd = fd_alloc();
    if (fd < 0)
        return -1;

    fd_table[fd].node = node;

    return fd;
}

int sys_write(int fd, uint8_t* buffer, uint32_t size)
{
    if (fd < 0 || fd >= MAX_FDS)
        return -1;

    if (!fd_table[fd].used)
        return -1;

    file_desc_t* desc = &fd_table[fd];

    int written = desc->node->ops->write(
        desc->node,
        desc->offset,
        size,
        buffer
    );

    if (written > 0)
        desc->offset += written;

    return written;
}

int sys_read(int fd, uint8_t* buffer, uint32_t size)
{
    if (fd < 0 || fd >= MAX_FDS)
        return -1;

    if (!fd_table[fd].used)
        return -1;

    file_desc_t* desc = &fd_table[fd];

    int read = desc->node->ops->read(
        desc->node,
        desc->offset,
        size,
        buffer
    );

    if (read > 0)
        desc->offset += read;

    return read;
}

void sys_close(int fd)
{
    if (fd >= 0 && fd < MAX_FDS)
        fd_table[fd].used = 0;
}

int sys_seek(int fd, uint32_t offset)
{
    if (fd < 0 || fd >= MAX_FDS)
        return -1;

    if (!fd_table[fd].used)
        return -1;

    fd_table[fd].offset = offset;
    return 0;
}
