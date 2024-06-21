#ifndef __VFS_H
#define __VFS_H
/**
 * VFS is interface for kernel
 */
#define PATHNAME_MAX_LEN    256
#define MAX_OPEN_FD         16

#define O_CREAT 00000100

/**
 * vnode: an abstract class that provides an unified interface
 * methods & creating instance is done by underlying file system (tmpfs)
 */

struct vnode {
    struct vnode* parent;    
    struct mount* mount;
    struct vnode_operations* v_ops;
    struct file_operations* f_ops;
    void* internal;
};

// file handle
struct file {
    struct vnode* vnode;
    unsigned long f_pos;  // RW position of this file handle
    struct file_operations* f_ops;
    int flags;
};

struct mount {
    struct vnode* root;
    struct filesystem* fs;
};

struct filesystem {
    const char* name;
    int (*setup_mount)(struct filesystem* fs, struct mount* mount);
};

struct file_operations {
    int (*write)(struct file* file, const void* buf, unsigned long len);
    int (*read)(struct file* file, void* buf, unsigned long len);
    int (*open)(struct vnode* file_node, struct file** target);
    int (*close)(struct file* file);
    // long lseek64(struct file* file, long offset, int whence);
};

struct vnode_operations {
    int (*lookup)(struct vnode* dir_node, struct vnode** target,
                    const char* component_name);
    int (*create)(struct vnode* dir_node, struct vnode** target,
                    const char* component_name);
    int (*mkdir)(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
};

struct mount* rootfs;

void init_rootfs();

int register_filesystem(struct filesystem* fs);
int vfs_open(const char* pathname, int flags, struct file** target);
int vfs_close(struct file* file);
int vfs_write(struct file* file, const void* buf, unsigned long len);
int vfs_read(struct file* file, void* buf, unsigned long len);
int vfs_mkdir(const char* pathname);
int vfs_mount(const char* target, const char* filesystem);
int vfs_lookup(const char* pathname, struct vnode** target);

#endif