#include "tmpfs.h"
#include "dyn_alloc.h"
#include "malloc.h"
#include "str_util.h"

static struct file_operations* tmpfs_file_op_ptr;
static struct vnode_operations* tmpfs_vnode_op_ptr;

void tmpfs_setup_mount(struct filesystem* fs, struct mount* mnt) {
    /**
     * rootfs is at the top of the VFS tree
     * mount tmpfs at rootfs
     * create the root vnode during the mount setup
     */
    mnt->fs = fs;
    mnt->root = tmpfs_new_node((struct tmpfs_internal*)0, "/", IS_DIRECTORY);
}

struct vnode* tmpfs_new_node(struct tmpfs_internal* parent, const char* name, int type) {
    struct vnode* new_vnode_ptr = (struct vnode*)alloc_chunk(sizeof(struct vnode));
    struct tmpfs_internal* new_internal_ptr = (struct tmpfs_internal*)alloc_chunk(sizeof(struct tmpfs_internal));

    if(parent != 0) { new_vnode_ptr->parent = parent->vnode; } 
    new_vnode_ptr->mount = (struct mount*)0;
    new_vnode_ptr->v_ops = tmpfs_vnode_op_ptr;
    new_vnode_ptr->f_ops = tmpfs_file_op_ptr;
    new_vnode_ptr->internal = (void*)new_internal_ptr;

    strcpy(new_internal_ptr->name, name);
    new_internal_ptr->type = type;
    new_internal_ptr->parent = parent;
    new_internal_ptr->vnode = new_vnode_ptr;
    new_internal_ptr->size = 0;
    if(type == IS_DIRECTORY) { new_internal_ptr->data = malloc(FILE_MAX_BYTE); }
    else { new_internal_ptr->data = (void*)0; }

    return new_vnode_ptr;
}

int tmpfs_register() {
    // register the file system to the kernel.
    // you can also initialize memory pool of the file system here.
    tmpfs_file_op_ptr = (struct file_operation*)alloc_chunk(sizeof(struct file_operations));
    if(tmpfs_file_op_ptr == (struct file_operations*)0) { 
        return TMPFS_REGISTER_FOP_ALLOC_FAIL; 
    }
    tmpfs_vnode_op_ptr = (struct vnode_operations*)alloc_chunk(sizeof(struct vnode_operations*));
    if(tmpfs_vnode_op_ptr == (struct vnode_operations*)0) { 
        return TMPFS_REGISTER_VOP_ALLOC_FAIL; 
    }

    tmpfs_file_op_ptr->write = tmpfs_write;
    tmpfs_file_op_ptr->read = tmpfs_read;
    tmpfs_file_op_ptr->open = tmpfs_open;
    tmpfs_file_op_ptr->close = tmpfs_close;

    tmpfs_vnode_op_ptr->lookup = tmpfs_lookup;
    tmpfs_vnode_op_ptr->create = tmpfs_create;
    tmpfs_vnode_op_ptr->mkdir = tmpfs_mkdir;

    return 0;
}

int tmpfs_write(struct file* file, const void* buf, unsigned long len) {
    /**
     * Given the file handle, 
     * VFS calls the corresponding write method to 
     * write the file starting from f_pos, 
     * then updates f_pos and size after write. 
     * (or not if it’s a special file) 
     * Returns size written or error code on error.
     */
    struct tmpfs_internal *internal = (struct tmpfs_internal*)file->vnode->internal;
    if(internal->type != IS_FILE) { return TMPFS_WRITE_NOT_FILE; }
    char *dest = &((char*)internal->data)[file->f_pos];
    char *src = (char*) buf;
    unsigned long w;

    for(w = 0; w < len, internal->size + w < FILE_MAX_BYTE; w++) {
        *(dest + w) = *(src + w);
    }
    internal->size += w;
    file->f_pos += w;

    if(w < len - 1) { return TMPFS_WRITE_OVERFLOW; } // ?
    return len;
}

int tmpfs_read(struct file* file, void* buf, unsigned long len) {
    /**
     * Given the file handle, 
     * VFS calls the corresponding read method to 
     * read the file starting from f_pos, 
     * then updates f_pos after read. 
     * (or not if it’s a special file)
     * 
     * Note that f_pos should not exceed the file size. 
     * Once a file read reaches the end of file(EOF), 
     * it should stop. 
     * Returns size read or error code on error.
     */
    struct tmpfs_internal *internal = (struct tmpfs_internal*)file->vnode->internal;
    if(internal->type != IS_FILE) { return TMPFS_READ_NOT_FILE; }
    char *src = &((char*)internal->data)[file->f_pos];
    char *dest = (char*) buf;
    unsigned long r;

    for(r = 0; r < len, file->f_pos + r < internal->size; r++) {
        *(dest + r) = *(src + r);
    }
    file->f_pos += r;

    if(r < len - 1) { return TMPFS_READ_REACH_EOF; }
    return len;
}

int tmpfs_open(struct vnode* file_node, struct file** target) {
    /**
     * It opens the vnode regardless of the underlying file system and file type, 
     * and creates a file handle for the file.
     */
    // creating file handle is done by vfs    

    return TMPFS_OPEN_SUCCESS;
}

int tmpfs_close(struct file* file) {
    /**
     * close and release the file handle. 
     */
}

int tmpfs_lookup(const char* pathname, struct vnode** target) {
    /**
     * File system iterates through directory entries and 
     * compares the component name to find the target file. 
     * Then, it passes the file’s vnode to the VFS if it finds the file.
     */
}

int tmpfs_create(struct vnode* dir_node, struct vnode** target, const char* component_name) {
    /**
     * create an regular file on underlying file system, 
     * should fail if file exist. 
     * Then passes the file’s vnode back to VFS.
     */
}

int tmpfs_mkdir(const char* pathname) {
    /**
     * create a directory on underlying file system, 
     * same as creating a regular file.
     */
}
