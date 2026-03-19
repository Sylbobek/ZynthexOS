#ifndef FD_H
#define FD_H

#include "../vfs/vfs.h"
#include <stdint.h>

#define MAX_FDS 32

typedef struct file_desc {
    int used;
    vfs_node_t* node;
    uint32_t offset;
} file_desc_t;

void fd_init();

int sys_open(const char* path);
int sys_write(int fd, uint8_t* buffer, uint32_t size);
int sys_read(int fd, uint8_t* buffer, uint32_t size);
void sys_close(int fd);
int sys_seek(int fd, uint32_t offset);

#endif
