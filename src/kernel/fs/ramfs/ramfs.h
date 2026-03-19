#ifndef RAMFS_H
#define RAMFS_H

#include "fs/vfs/vfs.h"

void ramfs_init();
static int ramfs_create(vfs_node_t* parent, const char* name, uint32_t flags);
static int ramfs_write(vfs_node_t* node,
                       uint32_t offset,
                       uint32_t size,
                       uint8_t* buffer);
static int ramfs_read(vfs_node_t* node,
                      uint32_t offset,
                      uint32_t size,
                      uint8_t* buffer);
void ramfs_list_root(void);
vfs_node_t* ramfs_create_instance(void);
static int ramfs_list(vfs_node_t* node);    

#endif
