#ifndef __TMPFS_H
#define __TMPFS_H

#include "vfs.h"

/**
 * a memory-based file system (tmpfs) that mounts as the root file system
 */

#define COMPNAME_MAX_LEN    15
#define DIR_MAX_ENTRY_NUM   16
#define FILE_MAX_BYTE       4096

#define IS_DIRECTORY    1
#define IS_FILE         2

/**
 * Error Code
 */
#define TMPFS_REGISTER_FOP_ALLOC_FAIL    -1
#define TMPFS_REGISTER_VOP_ALLOC_FAIL    -2

#define TMPFS_WRITE_NOT_FILE    -1
#define TMPFS_WRITE_OVERFLOW    -2

#define TMPFS_READ_NOT_FILE     -1
#define TMPFS_READ_REACH_EOF    -2

#define TMPFS_OPEN_ALLOC_FAIL   -1
#define TMPFS_OPEN_SUCCESS      1
#define TMPFS_OPEN_FAIL         0



struct tmpfs_internal {
    char name[COMPNAME_MAX_LEN];
    int type;
    struct tmpfs_internal *parent;
    struct tmpfs_internal *child[DIR_MAX_ENTRY_NUM];
    struct vnode *vnode;
    int size; // use for child count and data size
    void *data;
};

// return negative int on fail

void tmpfs_setup_mount(struct filesystem* fs, struct mount* mnt);
struct vnode* tmpfs_new_node(struct tmpfs_internal *parent, const char *name, int type);
int tmpfs_register();

// file op
int tmpfs_write(struct file* file, const void* buf, unsigned long len);
int tmpfs_read(struct file* file, void* buf, unsigned long len);
int tmpfs_open(struct vnode* file_node, struct file** target);
int tmpfs_close(struct file* file);

// vnode op
int tmpfs_lookup(const char* pathname, struct vnode** target);
int tmpfs_create(struct vnode* dir_node, struct vnode** target, const char* component_name);
int tmpfs_mkdir(const char* pathname);

#endif