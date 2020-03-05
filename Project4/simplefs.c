#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "simplefs.h"

#define ENTRY_POS(i) (i + 1)
#define FAT_POS(i) (1 + ENTRY_BLOCKS + i)
#define DATA_POS(i) (1 + ENTRY_BLOCKS + FAT_BLOCKS + i)
// Fat block from fat adress
#define FAT_FROM_ADRESS(a) (a / FAT_PER_BLOCK)
#define FAT_OFFSET(a) (a % FAT_PER_BLOCK)

int VD_SIZE = 0;
FILE *VD_FP = 0;

int Metadata_init(Metadata *metadata, int size) {
    if (size > MAX_DISK_SIZE) {
        printf("Error: size can't be bigger than %d", MAX_DISK_SIZE);
        return -1;
    } else {
        int used_blocks = 1 + ENTRY_BLOCKS + FAT_BLOCKS;
        metadata->blocks = (size - (used_blocks * BLOCK_SIZE)) / BLOCK_SIZE;
        printf("Total blocks: %d\n", metadata->blocks);

        return 0;
    }
}

int getBlock(Block *block, int block_pos) {
    // printf("Reading block %d\n", block_pos);
    fseek(VD_FP, block_pos * sizeof(Block), SEEK_SET);
    return fread(block, sizeof(Block), 1, VD_FP);
}

int getDirEntry(DirEntry *entry, int entry_pos) {
    if (entry_pos < 0 || entry_pos >= MAX_FILES) {
        printf("Entry position can't be %d\n", entry_pos);
        return -1;
    }
    int blockOffset = 1 + (entry_pos / ENTRIES_PER_BLOCK);
    int dirOffset = entry_pos % ENTRIES_PER_BLOCK;
    int position = (blockOffset * sizeof(Block)) + (dirOffset * sizeof(DirEntry)); 
    // printf("DirEntry position: %d\n", position);
    fseek(VD_FP, position, SEEK_SET);
    return fread(entry, sizeof(DirEntry), 1, VD_FP);
}

int setBlock(Block *block, int block_pos) {
    // printf("Writing block %d\n", block_pos);
    // printf("Seeked: %d\n", )
    // printf("pos: %ld\n", ftell(VD_FP));
    fseek(VD_FP, block_pos * sizeof(Block), SEEK_SET);
    return fwrite(block, sizeof(Block), 1, VD_FP);
    // printf("pos: %ld\n", ftell(VD_FP));
}

int setDirEntry(DirEntry *entry, int entry_pos) {
    if (entry_pos < 0 || entry_pos >= MAX_FILES) {
        printf("Entry position can't be %d\n", entry_pos);
        return -1;
    }
    int blockOffset = 1 + (entry_pos / ENTRIES_PER_BLOCK);
    int dirOffset = entry_pos % ENTRIES_PER_BLOCK;
    int position = (blockOffset * sizeof(Block)) + (dirOffset * sizeof(DirEntry)); 
    fseek(VD_FP, position, SEEK_SET);
    return fwrite(entry, sizeof(DirEntry), 1, VD_FP);
}

int getFat(int64_t *data, int fat_index, int offset) {
    int block_offset = FAT_POS(fat_index);
    int position = (block_offset * sizeof(Block)) + (offset * sizeof(int64_t));
    // printf("Position: %d\n", position);
    fseek(VD_FP, position, SEEK_SET);
    return fread(data, sizeof(int64_t), 1, VD_FP);
}

int setFat(int64_t *data, int fat_index, int offset) {
    int block_offset = FAT_POS(fat_index);
    int position = (block_offset * sizeof(Block)) + (offset * sizeof(int64_t));

    fseek(VD_FP, position, SEEK_SET);
    return fwrite(data, sizeof(int64_t), 1, VD_FP);
}

void DirEntry_init(DirEntry *entry) {
    entry->filename[0] = '\0';
    entry->first_block = FAT_NULL;
    entry->last_block = FAT_NULL;
    entry->size = 0;
    entry->read_buf = -1;
    entry->read_fat = FAT_NULL;
    entry->status = EMPTY;
    entry->capacity = 0;
}

int64_t DirEntry_free_memory(DirEntry *entry) {
    return entry->capacity - entry->size;
}

int DirEntry_increment_read(DirEntry *entry) {
    entry->read_buf += 1;
    if (entry->read_buf == BLOCK_SIZE) {
        entry->read_buf = 0;
        int64_t fat;
        getFat(&fat, FAT_FROM_ADRESS(entry->read_fat), entry->read_fat % FAT_PER_BLOCK);
        if (fat == FAT_NULL) {
            printf("Next fat block is null\n");
            return 1;
        } else {
            entry->read_fat = fat;
            return 0;
        }
    }
    return 0;
}

int create_vdisk (char *vdiskname, int m) {
    FILE *fp = NULL;
    if ((fp = fopen(vdiskname, "w+")) > 0) {
        VD_SIZE = (1 << m);
        // Resize to m
        fseek(fp, VD_SIZE, SEEK_SET);
        fputc('\n', fp);
        
        fclose(fp);
        return 0;
    } else {
        return -1;
    }
}

int sfs_format (char *vdiskname) {
    FILE *fp = NULL;
    if ((fp = fopen(vdiskname, "r+")) > 0) {
        Block block;
        // for setBlock
        VD_FP = fp;
        // init superblock
        if (Metadata_init(&block.superblock, VD_SIZE) == -1) {  
            return -1;
        } 
        // fwrite(&block, sizeof(Block), 1, fp);
        setBlock(&block, 0);

        Block newblock;
        getBlock(&newblock, 0);
        printf("Data blocks: %d\n", newblock.superblock.blocks);

        // init entries
        for (int i = 0; i < ENTRIES_PER_BLOCK; ++i)
            DirEntry_init(&block.entries[i]);
        for (int i = 0; i < ENTRY_BLOCKS; ++i)
            setBlock(&block, ENTRY_POS(i));
            //fwrite(&block, sizeof(Block), 1, fp);
        
        // init FAT
        for (int i = 0; i < FAT_PER_BLOCK; ++i)
            block.fat[i] = FAT_EMPTY;
        for (int i = 0; i < FAT_BLOCKS; ++i) {
            setBlock(&block, FAT_POS(i));

            /* int64_t f;
            getFat(&f, i, 2);
            printf("%d[2]: %ld\n", i, f); */
        }
            // fwrite(&block, sizeof(Block), 1, fp);

        fclose(fp);
        VD_FP = 0;
        printf("Formatted disk successfully\n");
        return 0;
    } else {
        return -1;
    }
}

int sfs_mount (char *vdiskname) {
    FILE *fp;
    if ((fp = fopen(vdiskname, "r+")) > 0) {
        VD_FP = fp;

        Block block;
        getBlock(&block, 0);
        // printf("Number of data blocks: %d\n", block.superblock.blocks);

        printf("%s mounted successfully\n", vdiskname);
        return 0;
    } else {
        return -1;
    }
}

int sfs_umount () {
    // TODO: error handling
    fflush(VD_FP);
    DirEntry entry;
    for (int i = 0; i < MAX_FILES; ++i) {
        getDirEntry(&entry, i);
        entry.status = CLOSED;
        setDirEntry(&entry, i);
    }

    fclose(VD_FP);
    VD_FP = 0;
    return 0;
}

int sfs_create(char *filename) {
    if (strlen(filename) >= NAME_SIZE) {
        printf("Name can't be longer than %d\n", NAME_SIZE);
        return -1;
    }

    DirEntry entry;
    for (int i = 0; i < MAX_FILES; ++i) {
        getDirEntry(&entry, i);
        // printf("File %d: %d\n", i, );

        if (entry.status == EMPTY) {
            // Found empty dirEntry
            strcpy(entry.filename, filename);
            entry.size = 0;
            entry.first_block = FAT_NULL;
            entry.status = CLOSED;
            // setBlock(&block, ENTRY_POS(i));
            setDirEntry(&entry, i);
            printf("Created file %s\n", filename);
            return 0;
        } else {
            // printf("File %s\n", entry.filename);
            if (strcmp(entry.filename, filename) == 0) {
                printf("File %s already exists\n", filename);
                return -1;
            }  
        }
    }
    // All blocks are traversed
    printf("All files are used\n");
    return -1;
}

int sfs_open (char *filename, int mode) {
    if (mode != 0 && mode != 1) {
        printf("Mode can be either 0 or 1\n");
        return -1;
    }

    DirEntry entry;
    for (int i = 0; i < MAX_FILES; ++i) {
        getDirEntry(&entry, i);
        // printf("File %d: %d\n", i, );
        if (entry.status == EMPTY)
            break;

        if (strcmp(entry.filename, filename) == 0) {
            if (entry.status == CLOSED) {
                printf("Opening file %s in mode %d\n", filename, mode);
                entry.status = mode;
                if (mode == READ) {
                    entry.read_buf = 0;
                    entry.read_fat = entry.first_block;
                    printf("First block is: %ld[%ld]", 
                        FAT_FROM_ADRESS(entry.read_fat), entry.read_fat % BLOCK_SIZE);
                }
                setDirEntry(&entry, i);
                return i;
            } else {
                // READ or WRITE
                printf("File %s has already opened\n", filename);
                return -1;
            }
        }
    }

    printf("File %s is not found\n", filename);
    return -1;
}

int sfs_getsize (int fd) {
    DirEntry entry;
    if (getDirEntry(&entry, fd) > 0) {
        if (entry.status == EMPTY) {
            printf("Invalid file descriptor %d\n", fd);
            return -1;
        } else if (entry.status == CLOSED) {
            printf("Invalid file descriptor %d\n", fd);
            return -1;
        } else {
            return entry.size;
        }
    } else {
        printf("Invalid file descriptor %d\n", fd);
        return -1;
    }
}

int sfs_close (int fd) {
    DirEntry entry;
    if (getDirEntry(&entry, fd) > 0) {
        if (entry.status == EMPTY) {
            printf("File has not been created yet %d\n", fd);
            return -1;
        } else if (entry.status == CLOSED) {
            printf("File is already closed\n");
            return -1;
        } else {
            if (entry.status == READ) {
                entry.read_buf = -1;
            }
            entry.status = CLOSED;
            setDirEntry(&entry, fd);
            return 0;
        }
    } else {
        printf("Invalid file descriptor %d\n", fd);
        return -1;
    }
}

int sfs_read (int fd, void *buf, int n) {
    int8_t *data = buf;
    DirEntry entry;
    if (getDirEntry(&entry, fd) > 0) {
        if (entry.status == EMPTY) {
            printf("File has not been created yet %d\n", fd);
            return -1;
        } else if (entry.status == CLOSED) {
            printf("Can't read closed file %s. Open the file first\n", entry.filename);
            return -1;
        } else if (entry.status == APPEND) {
            printf("File %s is in append mode\n", entry.filename);
            return -1;
        } else {
            if (entry.size < n) {
                printf("File %s has size less than %d\n", entry.filename, n);
                return -1;
            } else {
                
                Block block;
                int64_t blockIndex = entry.read_fat;
                getBlock(&block, DATA_POS(blockIndex));

                for (int i = 0; i < n; ++i) {
                    if (entry.read_fat != blockIndex) {
                        blockIndex = entry.read_fat;
                        getBlock(&block, DATA_POS(blockIndex));
                    }
                    data[i] = block.data[entry.read_buf];

                    DirEntry_increment_read(&entry);
                }

                setDirEntry(&entry, fd);
                return 0;   
            }
        }
    } else {
        printf("Invalid file descriptor %d\n", fd);
        return -1;
    }
}

int64_t allocateFat(DirEntry *entry, int neededBlocks);

int sfs_append (int fd, void *buf, int n) {
    if (n == 0)
        return 0;
    int8_t *data = (int8_t*) buf;
    DirEntry entry;
    if (getDirEntry(&entry, fd) > 0) {
        if (entry.status == EMPTY) {
            printf("File has not been created yet %d\n", fd);
            return -1;
        } else if (entry.status == CLOSED) {
            printf("Can't read closed file %s. Open the file first\n", entry.filename);
            return -1;
        } else if (entry.status == READ) {
            printf("File %s is in read mode\n", entry.filename);
            return -1;
        } else {
            int neededSize = n - DirEntry_free_memory(&entry);
            int neededBlocks = 0;
            if (neededSize > 0) {
                neededBlocks = neededSize / BLOCK_SIZE;
                if (neededSize % BLOCK_SIZE)
                    neededBlocks += 1;
            }

            // printf("Needs %d more blocks\n", neededBlocks);

            DirEntry newEntry = entry;
            int64_t start = allocateFat(&newEntry, neededBlocks);

            // printf("Allocated fats\n");

            Block dataBlock;
            int64_t index = start;
            // printf("Last index: %ld\n", index);
            for (int i = 0; i < n; ++i) {
                // printf("Size: %ld\n", entry.size);
                int last = entry.size % (BLOCK_SIZE);
                // printf("Last: %d\n", last);
                if (i == 0) {
                    if (index == FAT_NULL) {
                        index = newEntry.last_block;
                        // printf("Next block: %ld\n", index);
                        getBlock(&dataBlock, DATA_POS(index));
                    } else {
                        // printf("Getting next block: %ld\n", index);
                        getBlock(&dataBlock, DATA_POS(index));
                    }
                } else if (last == 0) {
                    if (index != FAT_NULL) {
                        // printf("Writing data block %ld to disk\n", index);
                        // printf("Start of data: %s\n", dataBlock.data);
                        setBlock(&dataBlock, DATA_POS(index));
                        int64_t fat;
                        getFat(&fat, FAT_FROM_ADRESS(index), index % FAT_PER_BLOCK);
                        // printf("Next block: %ld\n", fat);
                        getBlock(&dataBlock, DATA_POS(fat));
                        index = fat;
                    } else {
                        printf("Shouldn't be there \n");
                    }
                }
                // printf("Writing byte: %c to index %d\n", data[i], last);
                dataBlock.data[last] = data[i];
                entry.size += 1;
                // printf("Written byte %d\n", i);
            }
            
            // printf("Writing data block %ld to disk\n", index);
            setBlock(&dataBlock, DATA_POS(index));
            // printf("New dirEntry start: %ld, end: %ld\n", newEntry.first_block, newEntry.last_block);
            newEntry.size = entry.size;
            setDirEntry(&newEntry, fd);

            return 0;
        }
    } else {
        printf("Invalid file descriptor %d\n", fd);
        return -1;
    }
}

int64_t allocateFat(DirEntry *entry, int neededBlocks) {
    int64_t start_block = entry->last_block;
    if (neededBlocks == 0)
        return FAT_NULL;
    printf("Need to allocate %d blocks\n", neededBlocks);
    entry->capacity += (neededBlocks * BLOCK_SIZE);
    for (int i = 0; i < FAT_BLOCKS; ++i) {
        for (int j = 0; j < FAT_PER_BLOCK; ++j) {
            int64_t fat;
            getFat(&fat, i, j);
            // printf("Fat %d[%d]: %ld\n", i, j, fat);
            if (fat == FAT_EMPTY) {
                printf("Fat %d[%d] is empty\n", i, j);
                int64_t fatIndex = (i * FAT_PER_BLOCK) + j;
                // printf("FatIndex: %ld\n", fatIndex);
                if (entry->first_block == FAT_NULL) {
                    entry->first_block = fatIndex;
                    entry->last_block = fatIndex;
                    fat = FAT_NULL;
                    setFat(&fat, i, j);
                    start_block = fatIndex;
                } else {
                    // last block points to next last block
                    setFat(&fatIndex, FAT_FROM_ADRESS(entry->last_block), FAT_OFFSET(entry->last_block));
                    // new last block is null
                    entry->last_block = fatIndex;
                    fat = FAT_NULL;
                    setFat(&fat, i, j);
                }
                neededBlocks -= 1;
                if (neededBlocks == 0)
                    return start_block;
            }
        }
    }
    return FAT_NULL;
}