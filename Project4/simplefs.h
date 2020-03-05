#ifndef SIMPLE_FS_
#define SIMPLE_FS_

#include <inttypes.h>

#define KB 1024
#define MB (KB * KB)
#define BLOCK_SIZE (1 * KB)
#define NAME_SIZE 32
#define FAT_ENTRIES (128 * KB)
#define MAX_DISK_SIZE (128 * MB)
#define MAX_FILES 52

#define ENTRY_BLOCKS 7
#define FAT_BLOCKS 1024

#define ENTRIES_PER_BLOCK 8
#define FAT_PER_BLOCK 128

// Occupied but no next
#define FAT_NULL ((int64_t) -1)
// Unoccupied
#define FAT_EMPTY ((int64_t) -2)

extern int VD_SIZE;
// Virtual disk file descriptor
extern FILE *VD_FP;

// size should be 128 bytes
// (inode)
struct DirEntry {
    char filename[NAME_SIZE];
    int64_t first_block;     // points to first block in FAT
    int64_t last_block;
    // int64_t last_block;
    int64_t size;
    int64_t read_buf;
    int64_t read_fat;
    enum {
        READ,
        APPEND,
        // not opened but created
        CLOSED,
        // not created
        EMPTY
    } status;
    int64_t capacity;
};
typedef struct DirEntry DirEntry;

struct Metadata {
    int blocks; // No of blocks
};
typedef struct Metadata Metadata;

struct Root {
    // Block 0
    Metadata metadata;

    // Block 1-7
    DirEntry dir_entries[MAX_FILES];
};
typedef struct Root Root;

union Block {
    Metadata superblock;
    DirEntry entries[ENTRIES_PER_BLOCK];
    int64_t fat[FAT_PER_BLOCK];
    int8_t data[BLOCK_SIZE];
};
typedef union Block Block;

enum Mode {
    MODE_READ,
    MODE_APPEND
};

int create_vdisk (char *vdiskname, int m);

int sfs_format (char *vdiskname);

int sfs_mount (char *vdiskname);

int sfs_umount ();

int sfs_create (char *filename);

int sfs_open (char *filename, int mode);

int sfs_getsize (int fd);

int sfs_close (int fd);

int sfs_read (int fd, void *buf, int n);

int sfs_append (int fd, void *buf, int n);

int sfs_delete (char *filename);

#endif // SIMPLE_FS_