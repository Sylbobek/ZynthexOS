#ifndef VFS_H
#define VFS_H

#include <stdint.h>

#define VFS_FILE  0x1
#define VFS_DIR   0x2

typedef struct vfs_node vfs_node_t;
typedef struct vfs_ops  vfs_ops_t;

typedef struct vfs_ops {
    vfs_node_t* (*lookup)(vfs_node_t*, const char*);
    int (*create)(vfs_node_t*, const char*, uint32_t);
    int (*read)(vfs_node_t*, uint32_t, uint32_t, uint8_t*);
    int (*write)(vfs_node_t*, uint32_t, uint32_t, uint8_t*);
    int (*list)(vfs_node_t*);   
} vfs_ops_t;


struct vfs_node {
    char name[64];
    uint32_t flags;
    uint32_t size;

    vfs_node_t* parent;

    void* fs_private;
    vfs_ops_t* ops;
};

typedef struct vfs_mount {
    vfs_node_t* mountpoint;
    vfs_node_t* root;
} vfs_mount_t;

void vfs_mount(const char* path, vfs_node_t* fs_root);
vfs_node_t* vfs_check_mount(vfs_node_t* node);

extern vfs_node_t* vfs_root;

void vfs_init();
vfs_node_t* vfs_lookup(const char* path);

#endif
