#ifndef PATH_H
#define PATH_H

#include "vfs.h"

vfs_node_t* path_resolve(vfs_node_t* root, const char* path);

#endif
