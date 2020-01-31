#include "mac.h"
#include "irq.h"
#include "type.h"
#include "screen.h"
#include "syscall.h"
#include "test.h"

desc_t *receive_desc;
queue_t recv_block_queue;
uint32_t cnt = 1; //record the time of iqr_mac
//uint32_t buffer[PSIZE] = {0x00040045, 0x00000100, 0x5d911120, 0x0101a8c0, 0xfb0000e0, 0xe914e914, 0x00000801,0x45000400, 0x00010000, 0x2011915d, 0xc0a80101, 0xe00000fb, 0x14e914e9, 0x01080000};
//uint32_t buffer[PSIZE] = {0xffffffff, 0x5500ffff, 0xf77db57d, 0x00450008, 0x0000d400, 0x11ff0040, 0xa8c073d8, 0x00e00101, 0xe914fb00, 0x0004e914, 0x0000, 0x005e0001, 0x2300fb00, 0x84b7f28b, 0x00450008, 0x0000d400, 0x11ff0040, 0xa8c073d8, 0x00e00101, 0xe914fb00, 0x0801e914, 0x0000};
/*
void clear_interrupt()
{
    uint32_t ds;
    ds = reg_read_32(0xbfe11000 + DmaStatus);
    reg_write_32(0xbfe11000 + DmaStatus, ds);
}
*/
static void mac_recv_desc_init(mac_t *mac)
{
    int i;
    for (i = 0; i < PNUM - 1; i++) {
        rdes[i].des0 = 0x80000000;
        rdes[i].des1 = 0x81000000 | (4 * PSIZE);
        rdes[i].des2 = (RECV_BASE_ADDR + i * 4 * PSIZE) & 0x1fffffff;
        rdes[i].des3 = ((uint32_t)&rdes[i+1]) & 0x1fffffff;
    }
    rdes[PNUM-1].des0 = 0x80000000;
    rdes[PNUM-1].des1 = 0x83000000 | (4 * PSIZE);
    rdes[PNUM-1].des2 = (RECV_BASE_ADDR + (PNUM-1) * 4 * PSIZE) & 0x1fffffff;
    rdes[PNUM-1].des3 = ((uint32_t)&rdes[0]) & 0x1fffffff;
    
    //memset((void*)RDES_BASE_ADDR, 0, PNUM * 4 * PSIZE);
    
    mac->daddr = RECV_BASE_ADDR;
    mac->daddr_phy = RECV_BASE_ADDR & 0x1fffffff;
    mac->rd = (uint32_t)rdes;
    mac->rd_phy = (uint32_t)rdes & 0x1fffffff;
}


static void mii_dul_force(mac_t *mac)
{
    reg_write_32(mac->dma_addr, 0x80); //?s
                                       //   reg_write_32(mac->dma_addr, 0x400);
    uint32_t conf = 0xc800;            //0x0080cc00;

    // loopback, 100M
    reg_write_32(mac->mac_addr, reg_read_32(mac->mac_addr) | (conf) | (1 << 8));
    //enable recieve all
    reg_write_32(mac->mac_addr + 0x4, reg_read_32(mac->mac_addr + 0x4) | 0x80000001);
}

void dma_control_init(mac_t *mac, uint32_t init_value)
{
    reg_write_32(mac->dma_addr + DmaControl, init_value);
    return;
}

void download_task()
{

    mac_t test_mac;
    uint32_t ret;
    uint32_t i, j;
    uint32_t print_location = 3;
    //uint32_t flag0 = 0, flag1 = 0, flag2 = 0;

    test_mac.mac_addr = 0xbfe10000;
    test_mac.dma_addr = 0xbfe11000;

    test_mac.psize = PSIZE * 4; // 64bytes
    test_mac.pnum = PNUM;       // pnum
    mac_recv_desc_init(&test_mac);

    dma_control_init(&test_mac, DmaStoreAndForward | DmaTxSecondFrame | DmaRxThreshCtrl128);
    clear_interrupt();

    mii_dul_force(&test_mac);

    //irq_enable();
    queue_init(&recv_block_queue);
    sys_move_cursor(1, print_location);
    printf("[RECV TASK] start recv:                    ");
    ret = sys_net_recv(test_mac.rd, test_mac.rd_phy, test_mac.daddr);

    //Recv_desc = (uint32_t *)(test_mac.rd + (PNUM - 1) * 16);
/*
    if (((*Recv_desc) & 0x80000000) == 0x80000000)
    {
        sys_move_cursor(1, print_location);
        printf("> [RECV TASK] waiting receive package.\n");
        sys_wait_recv_package();
    }
    */
    mac_cnt = 0;
    //valid_cnt = 0;
    //irq_enable(1);
    while (mac_cnt < NUM_DMA_DESC) {
        reg_write_32(DMA_BASE_ADDR + 0x8, 0x1);
        sys_wait_recv_package();
    }
    reg_write_32(0xbfd0105c, 0x0);
    mac_cnt = -1;

    int fd = sys_fopen("plane", 0x03);

    for(i=0; i<PNUM; i++)
        if (*((uint32_t*)(RECV_BASE_ADDR + i * 1024)) == 0x335bfa80 && *((uint32_t*)(RECV_BASE_ADDR + i * 1024 + 13 * 4)) == 0xff000000)
        {
            sys_fwrite(fd, (char*)(RECV_BASE_ADDR + i * 1024 + 14 * 4), 512);
            continue;
        }
        
    sys_fclose(fd);

    sys_exit();
}

void mac_init_task()
{
    uint32_t print_location = 1;
    sys_move_cursor(1, print_location);
    printf("> [MAC INIT TASK] Waiting for MAC initialization .\n");
    sys_init_mac();
    sys_move_cursor(1, print_location);
    printf("> [MAC INIT TASK] MAC initialization succeeded.           \n");
    sys_exit();
}