#ifndef INCLUDE_FS_H_
#define INCLUDE_FS_H_

#include "type.h"

/*

--------------------------------------------------
| superblock | block map | inode map |   inodes  |
--------------------------------------------------
|   1 block  |  8 block  |  1 block  | 502 block |
--------------------------------------------------

2MB in total

*/

#define MAGIC          0x23333333

#define FS_BASE_ADDR   0x10000000
#define SB_BASE_ADDR   0x10000000
#define IM_BASE_ADDR   0X10001000
#define BM_BASE_ADDR   0X10002000
#define IN_BASE_ADDR   0x1000a000
#define DATA_BASE_ADDR 0x10200000

#define FS_SIZE 0x40000000
#define BLOCK_SIZE  0x1000

#define DIR_DEPTH 16

#define BUFFER 0xa0ff4000
#define FD_ADDR 0xa0ffc000

extern char shell_path[DIR_DEPTH][28];
extern int shell_path_depth;
extern int absolute;
extern char inst_path[DIR_DEPTH][28];
extern int inst_path_depth;


typedef struct superblock_info {
    uint32_t magic;
    uint32_t fs_size;
    
    uint32_t addr;
    uint32_t size;
    
    uint32_t inodemap_offset;
    uint32_t inodemap_size;
    
    uint32_t blockmap_offset;
    uint32_t blockmap_size;
    
    uint32_t inodes_offset;
    uint32_t inodes_size;
    uint32_t inodes_num;
    
    uint32_t data_offset;
    uint32_t data_size;
    
    uint32_t block_size;
    uint32_t block_all;
    uint32_t block_used;
} superblock_t; // pine to 1 block

typedef struct blockdir_info {
    uint32_t block[8];
    uint16_t length[8];
} blockdir_t; // 48B

typedef struct inode_info {
    uint32_t size;
    
    //uint32_t block_start;
    uint32_t direct[10];
    uint32_t first;
    uint32_t second;
    uint32_t bnum;
    
    uint32_t dnum;
    
    uint16_t owner;
    uint8_t mode; // 0x1 for read. 0x2 for write. 0x3 for read & write.
    uint8_t type; // 0x80 for directory. 0x00 for file. 0x8 for launched file.
    
} inode_t; //64B

typedef struct dentry_info {
    char name[28];
    uint16_t id;
    uint8_t type;
    uint8_t pined;
} dentry_t; //32B

typedef struct file_description_info {
    uint32_t size;
    
    //uint32_t block_start;
    uint32_t direct[8];
    uint32_t first;
    uint32_t second;
    uint32_t bnum;
    
    uint32_t r_ptr;
    uint32_t w_ptr;
    
    uint32_t pined;
    
    uint16_t inode;
    uint8_t mode; // 0x1 for read. 0x2 for write. 0x3 for read & write.
    uint8_t occupied;
} fd_t; //64B

void do_mkfs();
void do_statfs();
void do_ls();
void do_cd();
void do_mkdir();
int do_rmdir();

void do_touch();
int do_cat();
int do_fopen(char*, int);
int do_fread(int, char*, int);
int do_fwrite(int, char*, int);
void do_fclose(int);
void do_loadelf();

#endif