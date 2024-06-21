#include "vfs.h"
#include "tmpfs.h"
#include "dyn_alloc.h"
#include "str_util.h"

void init_rootfs() {
    // setup tmpfs as the root file system
    struct filesystem *tmpfs = (struct filesystem *)alloc_chunk(sizeof(struct filesystem));
    tmpfs->name = (char *)alloc_chunk(COMPNAME_MAX_LEN + 1);
    strcpy(tmpfs->name, "tmpfs");
    tmpfs->setup_mount = tmpfs_setup_mount;
    if(register_filesystem(tmpfs)) { return; };
    
    rootfs = (struct mount *)chunk_alloc(sizeof(struct mount));    
    tmpfs->setup_mount(tmpfs, rootfs);
}

int register_filesystem(struct filesystem* fs) {
  // register the file system to the kernel.
  // you can also initialize memory pool of the file system here.
  if(str_cmp_len(fs->name, "tmpfs", 5) == 0) { return tmpfs_register(); }
  return 1;
}

int vfs_open(const char* pathname, int flags, struct file** target) {
  // 1. Lookup pathname
  // 2. Create a new file handle for this vnode if found.
  // 3. Create a new file if O_CREAT is specified in flags and vnode not found
  // lookup error code shows if file exist or not or other error occurs
  // 4. Return error code if fails
  
}

int vfs_close(struct file* file) {
  // 1. release the file handle
  // 2. Return error code if fails
}

int vfs_write(struct file* file, const void* buf, unsigned long len) {
  // 1. write len byte from buf to the opened file.
  // 2. return written size or error code if an error occurs.
}

int vfs_read(struct file* file, void* buf, unsigned long len) {
  // 1. read min(len, readable size) byte to buf from the opened file.
  // 2. block if nothing to read for FIFO type
  // 2. return read size or error code if an error occurs.
}

int vfs_mkdir(const char* pathname) {

}

int vfs_mount(const char* target, const char* filesystem) {

}

int vfs_lookup(const char* pathname, struct vnode** target) {

}