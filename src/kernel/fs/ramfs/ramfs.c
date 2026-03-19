#include "fs/ramfs/ramfs.h"
#include "mm/heap.h"
#include "lib/string.h"
#include "core/debug.h"
#include "video/vga.h"

static int ramfs_initialized = 0;

typedef struct ramfs_node {
    vfs_node_t vfs;

    uint8_t* data;
    uint32_t size;
    uint32_t capacity;

    struct ramfs_node* children[32];
    uint32_t child_count;

} ramfs_node_t;

// Forward declarations
static vfs_node_t* ramfs_lookup(vfs_node_t* node, const char* name);
static int ramfs_create(vfs_node_t* parent, const char* name, uint32_t flags);
static int ramfs_read(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
static int ramfs_write(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
static int ramfs_list(vfs_node_t* node);
static vfs_ops_t ramfs_ops;

static void ramfs_init_ops(void)
{
    if (ramfs_initialized)
        return;

    ramfs_ops.lookup = ramfs_lookup;
    ramfs_ops.create = ramfs_create;
    ramfs_ops.read   = ramfs_read;
    ramfs_ops.write  = ramfs_write;
    ramfs_ops.list   = ramfs_list;

    ramfs_initialized = 1;
}

static vfs_node_t* ramfs_lookup(vfs_node_t* node, const char* name)
{
    if (!node || !node->fs_private)
        debug_panic("Lookup: NULL node");
    ramfs_node_t* rnode = (ramfs_node_t*)node->fs_private;

    for (uint32_t i = 0; i < rnode->child_count; i++)
    {
        if (strcmp(rnode->children[i]->vfs.name, name) == 0)
            return &rnode->children[i]->vfs;
    }


    return 0;
}

void ramfs_init()
{
    ramfs_node_t* root = kmalloc(sizeof(ramfs_node_t));
    memset(root, 0, sizeof(ramfs_node_t));

    strcpy(root->vfs.name, "/");
    root->vfs.flags = VFS_DIR;
    root->vfs.fs_private = root;

    ramfs_ops.lookup = ramfs_lookup;
    root->vfs.ops = &ramfs_ops;
    ramfs_ops.create = ramfs_create;
    ramfs_ops.write = ramfs_write;
    ramfs_ops.read = ramfs_read;

    vfs_root = &root->vfs;  
        
}

static int ramfs_create(vfs_node_t* parent, const char* name, uint32_t flags)
{
    if (!parent || !parent->fs_private)
        debug_panic("Create: NULL parent");

    ramfs_node_t* pnode = (ramfs_node_t*)parent->fs_private;

    if (pnode->child_count >= 32)
        return -1;

    ramfs_node_t* child = kmalloc(sizeof(ramfs_node_t));
    memset(child, 0, sizeof(ramfs_node_t));

    strcpy(child->vfs.name, name);
    child->vfs.flags = flags;
    child->vfs.parent = parent;
    child->vfs.fs_private = child;
    child->vfs.ops = parent->ops;

    pnode->children[pnode->child_count++] = child;

    return 0;
}

static int ramfs_write(vfs_node_t* node,
                       uint32_t offset,
                       uint32_t size,
                       uint8_t* buffer)
{
    if (!node || !node->fs_private)
        debug_panic("Write: fs_private NULL");


    ramfs_node_t* rnode = (ramfs_node_t*)node->fs_private;

    if (node->flags & VFS_DIR)
        return -1;

    if (offset + size > rnode->capacity)
    {
        uint32_t new_capacity = offset + size;
        uint8_t* new_data = kmalloc(new_capacity);

        if (rnode->data)
            memcpy(new_data, rnode->data, rnode->size);

        rnode->data = new_data;
        rnode->capacity = new_capacity;
    }

    memcpy(rnode->data + offset, buffer, size);

    if (offset + size > rnode->size)
        rnode->size = offset + size;

    return size;
}

static int ramfs_read(vfs_node_t* node,
                      uint32_t offset,
                      uint32_t size,
                      uint8_t* buffer)
{
    ramfs_node_t* rnode = (ramfs_node_t*)node->fs_private;

    if (offset >= rnode->size)
        return 0;

    if (offset + size > rnode->size)
        size = rnode->size - offset;

    memcpy(buffer, rnode->data + offset, size);

    return size;
}


vfs_node_t* ramfs_create_instance(void)
{
    ramfs_init_ops();

    ramfs_node_t* root = kmalloc(sizeof(ramfs_node_t));
    memset(root, 0, sizeof(ramfs_node_t));

    strcpy(root->vfs.name, "/");
    root->vfs.flags = VFS_DIR;
    root->vfs.fs_private = root;
    root->vfs.ops = &ramfs_ops;

    return &root->vfs;
}


static int ramfs_list(vfs_node_t* node)
{
    ramfs_node_t* rnode = (ramfs_node_t*)node->fs_private;

    for (uint32_t i = 0; i < rnode->child_count; i++)
    {
        vga_print(rnode->children[i]->vfs.name);
        vga_print("\n");
    }

    return 0;
}
