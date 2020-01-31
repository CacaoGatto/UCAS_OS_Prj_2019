#include "fs.h"
#include "screen.h"
#include "string.h"
#include "sched.h"

superblock_t superblock;
inode_t inode_buffer;
//dentry_t dentry_buffer;
//uint8_t  BUFFER[32*1024];
//uint8_t write_buffer[32*1024];

char shell_path[DIR_DEPTH][28];
int shell_path_depth = 0;
int absolute = 1;
char inst_path[DIR_DEPTH][28];
int inst_path_depth = 0;

void read_block(uint32_t block)
{
    sdread((char*)BUFFER, FS_BASE_ADDR + block * BLOCK_SIZE, 4 * 1024);
}

void write_block(uint32_t block)
{
    sdwrite((char*)BUFFER, FS_BASE_ADDR + block * BLOCK_SIZE, 4 * 1024);
}

void read_imap()
{
    sdread((char*)BUFFER, IM_BASE_ADDR, 4 * 1024);
}

void write_imap()
{
    sdwrite((char*)BUFFER, IM_BASE_ADDR, 4 * 1024);
}

void read_bmap()
{
    sdread((char*)BUFFER, BM_BASE_ADDR, 32 * 1024);
}

void write_bmap()
{
    sdwrite((char*)BUFFER, BM_BASE_ADDR, 32 * 1024);
}

void read_inode(uint16_t inode)
{
    uint16_t sector = inode / 16, part = inode % 16;
    sdread((char*)BUFFER, IN_BASE_ADDR + sector * 512, 512);
    memcpy(&inode_buffer, BUFFER + part * sizeof(inode_t), sizeof(inode_t));
}

void write_inode(uint16_t inode)
{
    uint16_t sector = inode / 16, part = inode % 16;
    sdread((char*)BUFFER, IN_BASE_ADDR + sector * 512, 512);
    memcpy(BUFFER + part * sizeof(inode_t), &inode_buffer, sizeof(inode_t));
    sdwrite((char*)BUFFER, IN_BASE_ADDR + sector * 512, 512);
}

uint16_t path_locate()
{
    int i;
    uint16_t inode;
    if (absolute) inode = 0x0000;
    else inode = pcb[0].path_inode;
    for (i = 0; i < inst_path_depth; i++) {
        read_inode(inode);
        uint32_t dnum = inode_buffer.dnum;
        uint32_t block = inode_buffer.direct[0];
        read_block(block);
        
        int j = 0, target = 0, found = 0;
        while (target < dnum) {
            dentry_t* dir = (dentry_t*)(BUFFER + j * sizeof(dentry_t));
            j++;
            if (dir->name[0] == '\0') continue;
            target++;
            if (dir->type != 0x80) continue;
            if (strcmp(inst_path[i], dir->name) == 0) {
                inode = dir->id;
                found = 1;
                break;
            }
        }
        if (!found) {
            inode = 0xffff;
            break;
        }
    }
    return inode;
}

uint16_t alloc_inode()
{
    read_imap();
    int i, j;
    for (i = 0; i < 4 * 1024; i++) {
        if (*((uint8_t*)(BUFFER + i)) != 0xff) break;
    }
    uint8_t temp = *((uint8_t*)(BUFFER + i));
    for (j = 0; j < 8; j++) {
        if ((temp & 0x01) == 0) break;
        else temp = temp >> 1;
    }
    temp = 0x01 << j;
    *((uint8_t*)(BUFFER + i)) |= temp;
    write_imap();
    return (8 * i + j);
}

uint32_t alloc_block()
{
    read_bmap();
    int i, j;
    for (i = 0; i < 32 * 1024; i++) {
        if (*((uint8_t*)(BUFFER + i)) != 0xff) break;
    }
    uint8_t temp = *((uint8_t*)(BUFFER + i));
    for (j = 0; j < 8; j++) {
        if ((temp & 0x01) == 0) break;
        else temp = temp >> 1;
    }
    temp = 0x01 << j;
    *((uint8_t*)(BUFFER + i)) |= temp;
    write_bmap();
    return (8 * i + j);
}

void free_inode(uint16_t inode)
{
    int i = inode / 8, j = inode % 8;
    uint8_t temp = 0x01 << j;
    read_imap();
    *((uint8_t*)(BUFFER + i)) &= (~temp);
    write_imap();
}

void free_block(uint32_t block)
{
    int i = block / 8, j = block % 8;
    uint8_t temp = 0x01 << j;
    read_bmap();
    *((uint8_t*)(BUFFER + i)) &= (~temp);
    write_bmap();
}

void init_superblock()
{
    superblock.magic = MAGIC;
    superblock.fs_size = FS_SIZE;
    superblock.addr = SB_BASE_ADDR;
    superblock.size = IM_BASE_ADDR - SB_BASE_ADDR;
    superblock.inodemap_offset = IM_BASE_ADDR - FS_BASE_ADDR;
    superblock.inodemap_size = BM_BASE_ADDR - IM_BASE_ADDR;
    superblock.blockmap_offset = BM_BASE_ADDR - FS_BASE_ADDR;
    superblock.blockmap_size = IN_BASE_ADDR - BM_BASE_ADDR;
    superblock.block_size = BLOCK_SIZE;
    superblock.block_all = FS_SIZE / BLOCK_SIZE;
    superblock.block_used = (DATA_BASE_ADDR - FS_BASE_ADDR) / BLOCK_SIZE;
    superblock.inodes_offset = IN_BASE_ADDR - FS_BASE_ADDR;
    superblock.inodes_size = DATA_BASE_ADDR - IN_BASE_ADDR;
    superblock.inodes_num = 1;
    superblock.data_offset = DATA_BASE_ADDR - FS_BASE_ADDR; 
    superblock.data_size = FS_SIZE + FS_BASE_ADDR - DATA_BASE_ADDR;
    
    memcpy(BUFFER, &superblock, sizeof(superblock_t));
    write_block(0);
}

void init_inodemap()
{
    int i;
    *((uint8_t*)BUFFER) = 0x01;
    for (i = 1; i < 4*1024; i++) *((uint8_t*)(BUFFER+i)) = 0x00;
    write_imap();
}

void init_blockmap()
{
    int i;
    for (i = 0; i < 64; i++) *((uint8_t*)(BUFFER+i)) = 0xff;
    *((uint8_t*)(BUFFER+64)) = 0x01;
    for (i = 65; i < 32*1024; i++) *((uint8_t*)(BUFFER+i)) = 0x00;
    write_bmap();
}

void init_inode(uint32_t block, uint16_t owner, uint16_t father, uint16_t self, uint8_t mode, uint8_t type)
{
    inode_buffer.direct[0] = block;
    int k;
    for (k = 1; k < 8; k++) inode_buffer.direct[k] = 0x00000000;
    
    inode_buffer.bnum = 1;
    
    inode_buffer.dnum = 2;

    inode_buffer.owner = owner;
    inode_buffer.mode = mode;
    inode_buffer.type = type;
    
    if (type == 0x80) inode_buffer.size = 2 * sizeof(dentry_t);
    else inode_buffer.size = 0;
    
    write_inode(self);
    
    if (self == 0x0000) return;
    
    read_inode(father);
    uint32_t dnum = ++(inode_buffer.dnum);
    uint32_t fblock = inode_buffer.direct[0];
    write_inode(father);
    
    read_block(fblock);
    int i, j = 0;
    dentry_t* dir;
    for (i = 0; i < dnum; i++) {
        dir = (dentry_t*)(BUFFER + i * sizeof(dentry_t));
        if (dir->name[0] == '\0') break;
    }
    while (inst_path[0][j] != '\0') {
        dir->name[j] = inst_path[0][j];
        j++;
    }
    dir->name[j] = '\0';
    dir->id = self;
    dir->type = type;
    write_block(fblock);
}

void init_block(uint32_t block, uint16_t father, uint16_t self)
{
    read_block(block);
    
    dentry_t* dir = (dentry_t*)BUFFER;
    dir->name[0] = '.';
    dir->name[1] = '.';
    dir->name[2] = '\0';
    dir->id = father;
    dir->type = 0x80;
    
    dir = (dentry_t*)(BUFFER + sizeof(dentry_t));
    dir->name[0] = '.';
    dir->name[1] = '\0';
    dir->id = self;
    dir->type = 0x80;
    
    write_block(block);
}

void init_root()
{
    init_inode(256, 0x0000, 0x0000, 0x0000, 0x03, 0x80);
    init_block(256, 0x0000, 0x0000);
}

void do_mkfs()
{
    read_block(0);
    uint32_t number = *((uint32_t*)BUFFER);
    if (number == MAGIC) {
        vt100_move_cursor(1, ++pcb[0].line);
        printk("File System has already existed!");
        do_statfs();
    }
    else {
        pcb[0].line = SHELL_BOARD;
        vt100_move_cursor(1, ++pcb[0].line);
        printk("[FS] Start initializing file system!");
        vt100_move_cursor(1, ++pcb[0].line);
        printk("[FS] Setting superblock...");
        init_superblock();
        vt100_move_cursor(1, ++pcb[0].line);
        printk("[FS] Setting inode-map...");
        init_inodemap();
        vt100_move_cursor(1, ++pcb[0].line);
        printk("[FS] Setting block-map...");
        init_blockmap();
        vt100_move_cursor(1, ++pcb[0].line);
        printk("[FS] Setting root...");
        init_root();
        vt100_move_cursor(1, ++pcb[0].line);
        printk("[FS] Initializing file system finished!");
    }
}


void do_statfs()
{
    read_block(0);
    //sdread(&superblock, SB_BASE_ADDR, sizeof(superblock_t));
    memcpy(&superblock, BUFFER, sizeof(superblock_t));
    
    vt100_move_cursor(1, ++pcb[0].line);
    printk("Magic number: 0x%x", superblock.magic);
    vt100_move_cursor(1, ++pcb[0].line);
    printk("Used blocks: %d/%d. Start block: %d", superblock.block_used, superblock.block_all, superblock.addr / superblock.block_size);
    vt100_move_cursor(1, ++pcb[0].line);
    printk("Inode map offset: %d. Occupied block: %d. Used: %d/%d", superblock.inodemap_offset / superblock.block_size, superblock.inodemap_size / superblock.block_size, superblock.inodes_num, superblock.inodes_size / sizeof(inode_t));
    vt100_move_cursor(1, ++pcb[0].line);
    printk("Block map offset: %d. Occupied block: %d", superblock.blockmap_offset / superblock.block_size, superblock.blockmap_size / superblock.block_size);
    vt100_move_cursor(1, ++pcb[0].line);
    printk("Inode offset: %d. Occupied block: %d", superblock.inodes_offset / superblock.block_size, superblock.inodes_size / superblock.block_size);
    vt100_move_cursor(1, ++pcb[0].line);
    printk("Data offset: %d. Occupied block: %d", superblock.data_offset / superblock.block_size, superblock.data_size / superblock.block_size);
    vt100_move_cursor(1, ++pcb[0].line);
    printk("Inode entry size: %dB. Directory entry size: %dB.",sizeof(inode_t),sizeof(dentry_t));
}


void do_ls()
{
    uint16_t inode = path_locate();
    if (inode == 0xffff) {
        vt100_move_cursor(1, ++pcb[0].line);
        printk("No such directory!");
        return;
    }
    else {
        read_inode(inode);
        uint32_t dnum = inode_buffer.dnum;
        uint32_t block = inode_buffer.direct[0];
        read_block(block);
        vt100_move_cursor(1, ++pcb[0].line);
        int j = 2, target = 2;
        while (target < dnum) {
            dentry_t* dir = (dentry_t*)(BUFFER + j * sizeof(dentry_t));
            j++;
            if (dir->name[0] == '\0') continue;
            int i = 0;
            while (dir->name[i] != '\0') printk("%c", dir->name[i++]);
            printk("   ");
            target++;
        }
    }
}

void do_cd()
{
    uint16_t inode = path_locate();
    if (inode == 0xffff) {
        vt100_move_cursor(1, ++pcb[0].line);
        printk("No such directory!");
        return;
    }
    pcb[0].path_inode = inode;
    int i, j, idx = 0;
    if (absolute) {
        for (i = 0; i < inst_path_depth; i++) {
            if (inst_path[i][0] == '.') {
                idx++;
                if (inst_path[i][1] == '.') {
                    if (i - idx >= 0) idx++;
                }
                continue;
            }
            for (j = 0; j < 28; j++) {
                shell_path[i - idx][j] = inst_path[i][j];
                if (inst_path[i][j] == '\0') break;
            }
        }
        shell_path_depth = inst_path_depth - idx;
    }
    else {
        for (i = 0; i < inst_path_depth; i++) {
            if (inst_path[i][0] == '.') {
                idx++;
                if (inst_path[i][1] == '.') {
                    if (shell_path_depth + i - idx >= 0) idx++;
                }
                continue;
            }
            for (j = 0; j < 28; j++) {
                shell_path[shell_path_depth + i - idx][j] = inst_path[i][j];
                if (inst_path[i][j] == '\0') break;
            }
        }
        shell_path_depth = shell_path_depth + inst_path_depth - idx;
    }
}


void do_mkdir()
{
    uint16_t inode = alloc_inode();
    uint32_t block = alloc_block();
    init_inode(block, 0x0000, pcb[0].path_inode, inode, 0x03, 0x80);
    init_block(block, pcb[0].path_inode, inode);
    vt100_move_cursor(1, ++pcb[0].line);
    printk("Create Success!");
}


int do_rmdir()
{
    uint16_t inode = pcb[0].path_inode;
    read_inode(inode);
    uint32_t dnum = inode_buffer.dnum--;
    uint32_t block = inode_buffer.direct[0];
    write_inode(inode);
    read_block(block);
    int j = 2, target = 2, found = 0;
    while (target < dnum) {
        dentry_t* dir = (dentry_t*)(BUFFER + j * sizeof(dentry_t));
        j++;
        if (dir->name[0] == '\0') continue;
        target++;
        if (dir->type != 0x80) continue;
        if (strcmp(inst_path[0], dir->name) == 0) {
            dir->name[0] = '\0';
            inode = dir->id;
            found = 1;
            write_block(block);
            break;
        }
    }
    if (!found) return 1;
    
    read_inode(inode);
    for (j = 0; j < 8; j++) free_block(inode_buffer.direct[j]);
    free_inode(inode);
    
    return 0;
}


void do_touch()
{
    uint16_t inode = alloc_inode();
    uint32_t block = alloc_block();
    init_inode(block, 0x0000, pcb[0].path_inode, inode, 0x03, 0x00);
    vt100_move_cursor(1, ++pcb[0].line);
    printk("Create Success!");;
}

int do_cat()
{
    uint16_t inode = pcb[0].path_inode;
    read_inode(inode);
    uint32_t num = inode_buffer.dnum;
    uint32_t block = inode_buffer.direct[0];
    read_block(block);
    int j = 2, target = 2, found = 0;
    while (target < num) {
        dentry_t* dir = (dentry_t*)(BUFFER + j * sizeof(dentry_t));
        j++;
        if (dir->name[0] == '\0') continue;
        target++;
        if (dir->type != 0x00) continue;
        if (strcmp(inst_path[0], dir->name) == 0) {
            inode = dir->id;
            found = 1;
            break;
        }
    }
    if (!found) return 1;
    
    vt100_move_cursor(1, ++pcb[0].line);
    read_inode(inode);
    num = inode_buffer.bnum;
    for (j = 0; j < num; j++) {
        block = inode_buffer.direct[j];
        read_block(block);
        int i;
        for (i = 0; i < inode_buffer.size; i++) {
            char ch = *((char*)(BUFFER + i));
            if (ch == '\n') vt100_move_cursor(1, ++pcb[0].line);
            else printk("%c", ch);
        }
    }
    return 0;
}

int do_fopen(char* name, int mode)
{
    uint16_t inode = pcb[0].path_inode;
    read_inode(inode);
    uint32_t num = inode_buffer.dnum;
    uint32_t block = inode_buffer.direct[0];
    read_block(block);
    int j = 2, target = 2, found = 0;
    while (target < num) {
        dentry_t* dir = (dentry_t*)(BUFFER + j * sizeof(dentry_t));
        j++;
        if (dir->name[0] == '\0') continue;
        target++;
        if (dir->type != 0x00) continue;
        if (strcmp(name, dir->name) == 0) {
            inode = dir->id;
            found = 1;
            break;
        }
    }
    j = 0;
    if (!found) {
        while (*name != '\0') {
            inst_path[0][j] = *name++;
            //printk("%c", *name);
            j++;
        }
        inst_path[0][j] = '\0';
        inode = alloc_inode();
        block = alloc_block();
        init_inode(block, 0x0000, pcb[0].path_inode, inode, 0x03, 0x00);
    }
    
    read_inode(inode);
    
    int i;
    fd_t* file;
    for (i = 0; i < 64; i++) {
        file = (fd_t*)(FD_ADDR + i * sizeof(fd_t));
        if (file->occupied == 0x00) break;
    }
    file->size = inode_buffer.size;
    for (j = 0; j < 8; j++) file->direct[j] = inode_buffer.direct[j];
    file->first = inode_buffer.first;
    file->second = inode_buffer.second;
    file->bnum = inode_buffer.bnum;
    file->r_ptr = 0;
    file->w_ptr = 0;
    file->inode = inode;
    file->mode = inode_buffer.mode & mode;
    file->occupied == 0xff;
    return i;
}

void do_fclose(int fd)
{
    fd_t* file = (fd_t*)(FD_ADDR + fd * sizeof(fd_t));
    file->occupied = 0x00;
}

int do_fread(int fd, char* buff, int size)
{
    fd_t* file = (fd_t*)(FD_ADDR + fd * sizeof(fd_t));
    int i;
    uint32_t pointer = file->r_ptr;
    file->r_ptr += size;
    int start = pointer / BLOCK_SIZE, end = file->r_ptr / BLOCK_SIZE;
    for (i = start; i < end; i++) {
        int pos = pointer - i * BLOCK_SIZE;
        read_block(file->direct[i]);
        int k;
        for (k = pos; k < BLOCK_SIZE; k++) {
            *buff++ = *((char*)(BUFFER + k));
        }
        pointer = BLOCK_SIZE * (1 + i);
    }
    read_block(file->direct[end]);
    for (i = 0; i < file->r_ptr - pointer; i++) {
        *buff++ = *((char*)(BUFFER + i + pointer % BLOCK_SIZE));
    }
    return size;
}

int do_fwrite(int fd, char* buff, int size)
{
    fd_t* file = (fd_t*)(FD_ADDR + fd * sizeof(fd_t));
    int i;
    uint32_t pointer = file->w_ptr;
    file->w_ptr += size;
    int start = pointer / BLOCK_SIZE, end = file->w_ptr / BLOCK_SIZE;
    for (i = start; i < end; i++) {
        int pos = pointer - i * BLOCK_SIZE;
        read_block(file->direct[i]);
        int k;
        for (k = pos; k < BLOCK_SIZE; k++) {
            *((char*)(BUFFER + k)) = *buff++;
        }
        write_block(file->direct[i]);
        pointer = BLOCK_SIZE * (1 + i);
    }
    read_block(file->direct[end]);
    for (i = 0; i < file->w_ptr - pointer; i++) {
        *((char*)(BUFFER + i + pointer % BLOCK_SIZE)) = *buff++;
    }
    write_block(file->direct[end]);
    if (file->w_ptr > file->size) {
        read_inode(file->inode);
        inode_buffer.size = file->w_ptr;
        write_inode(file->inode);
    }
    return size;
}
