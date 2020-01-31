#include "mac.h"
#include "irq.h"
#include "sched.h"
#include "string.h"

#define NUM_DMA_DESC 32
queue_t recv_block_queue;
uint32_t recv_flag[PNUM] = {0};
uint32_t ch_flag;
int mac_cnt = -1;
uint32_t valid_cnt = 0;

desc_t tdes[PNUM];
desc_t rdes[PNUM];
uint32_t tbuf[PNUM][24];
//uint32_t rbuf[PNUM][19];
uint32_t pkg[70][19];

uint32_t reg_read_32(uint32_t addr)
{
    return *((uint32_t *)addr);
}
uint32_t read_register(uint32_t base, uint32_t offset)
{
    uint32_t addr = base + offset;
    uint32_t data;

    data = *(volatile uint32_t *)addr;
    return data;
}

void reg_write_32(uint32_t addr, uint32_t data)
{
    *((uint32_t *)addr) = data;
}

static void gmac_get_mac_addr(uint8_t *mac_addr)
{
    uint32_t addr;

    addr = read_register(GMAC_BASE_ADDR, GmacAddr0Low);
    mac_addr[0] = (addr >> 0) & 0x000000FF;
    mac_addr[1] = (addr >> 8) & 0x000000FF;
    mac_addr[2] = (addr >> 16) & 0x000000FF;
    mac_addr[3] = (addr >> 24) & 0x000000FF;

    addr = read_register(GMAC_BASE_ADDR, GmacAddr0High);
    mac_addr[4] = (addr >> 0) & 0x000000FF;
    mac_addr[5] = (addr >> 8) & 0x000000FF;
}

void print_tx_dscrb(mac_t *mac)
{
    uint32_t i;
    printf("send buffer mac->saddr=0x%x ", mac->saddr);
    printf("mac->saddr_phy=0x%x ", mac->saddr_phy);
    printf("send discrb mac->td_phy=0x%x\n", mac->td_phy);
#if 0
    desc_t *send=mac->td;
    for(i=0;i<mac->pnum;i++)
    {
        printf("send[%d].des0=0x%x ",i,send[i].des0);
        printf("send[%d].des1=0x%x ",i,send[i].des1);
        printf("send[%d].des2=0x%x ",i,send[i].des2);
        printf("send[%d].des3=0x%x ",i,send[i].des3);
    }
#endif
}

void print_rx_dscrb(mac_t *mac)
{
    uint32_t i;
    printf("recieve buffer add mac->daddr=0x%x ", mac->daddr);
    printf("mac->daddr_phy=0x%x ", mac->daddr_phy);
    printf("recieve discrb add mac->rd_phy=0x%x\n", mac->rd_phy);
    desc_t *recieve = (desc_t *)mac->rd;
#if 0
    for(i=0;i<mac->pnum;i++)
    {
        printf("recieve[%d].des0=0x%x ",i,recieve[i].des0);
        printf("recieve[%d].des1=0x%x ",i,recieve[i].des1);
        printf("recieve[%d].des2=0x%x ",i,recieve[i].des2);
        printf("recieve[%d].des3=0x%x\n",i,recieve[i].des3);
    }
#endif
}

static uint32_t printf_recv_buffer(uint32_t recv_buffer)
{
    sys_move_cursor(1, 4);
    int i;
    for (i = 0; i < NUM_DMA_DESC; i++) {
        int j;
        printf("receive[%d]:", i);
        for (j = 0; j < 19; j++) printf("%x ", pkg[i][j]);
        printf("\n");
    }
}

//in your design,you have the choice to choose use register_irq_handler or not.
/*void register_irq_handler(int IRQn, irq_handler_t func)
{
    // register the func ,
}*/
void mac_irq_handle(void)
{
    // check whether new recv packet is arriving
    do_unblock_all(&recv_block_queue);
    int num = mac_cnt % PNUM;
    mac_cnt++;
    if (1) {
        //memcpy((uint8_t*)pkg[valid_cnt], (uint8_t*)(RDES_BASE_ADDR+4*PSIZE*num), 4*PSIZE);
        int i;
        for (i = 0; i < 19; i++) pkg[valid_cnt][i] = *((uint32_t*)(RDES_BASE_ADDR+4*PSIZE*num+4*i));
        valid_cnt++;
        vt100_move_cursor(2, 4);
        printk("> [MAC RECV TASK] start recv:(%d)                ",valid_cnt);
    }
    rdes[num].des0 = 0x80000000;
    clear_interrupt();
}

void irq_enable(int IRQn)
{
    //todo :set INT1_EN reg to enable mac interrupt,
    if (IRQn) reg_write_32(0xbfd0105c, 0x8);
    else reg_write_32(0xbfd0105c, 0x0);
    reg_write_32(0xbfd01064, 0xffffffff);
    reg_write_32(0xbfd01068, 0xffffffff);
    reg_write_32(0xbfd0106c, 0x0);
}
/**
 * Clears all the pending interrupts.
 * If the Dma status register is read then all the interrupts gets cleared
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 * you will use it in task3
 */
void clear_interrupt()
{
    uint32_t data;
    data = reg_read_32(0xbfe11000 + DmaStatus);
    reg_write_32(0xbfe11000 + DmaStatus, data);
}
void mac_recv_handle(mac_t *test_mac)
{
    // you will use this function in task 3.
    /*
     while (mac_cnt < NUM_DMA_DESC) {
         int own = rdes[mac_cnt].des0 >> 31;
         if (own) sys_wait_recv_package();
         else {
             rdes[mac_cnt++].des0 = 0x80000300;
             if (mac_cnt < NUM_DMA_DESC) sys_wait_recv_package();
         }
     }*/
    mac_cnt = 0;
    valid_cnt = 0;
    irq_enable(1);
    while (valid_cnt < NUM_DMA_DESC) {
        reg_write_32(DMA_BASE_ADDR + 0x8, 0x1);
        sys_wait_recv_package();
    }
    mac_cnt = -1;
    irq_enable(0);
    printf_recv_buffer(NUM_DMA_DESC);
}

static uint32_t printk_recv_buffer(uint32_t recv_buffer)
{
    //for debug ,you can use this function to print the contents of recv buffer
    
}

void set_sram_ctr()
{
    *((volatile unsigned int *)0xbfd00420) = 0x8000; /* 使能GMAC0 */
}
static void s_reset(mac_t *mac) //reset mac regs
{
    uint32_t time = 1000000;
    reg_write_32(mac->dma_addr, 0x01);

    while ((reg_read_32(mac->dma_addr) & 0x01))
    {
        reg_write_32(mac->dma_addr, 0x01);
        while (time)
        {
            time--;
        }
    };
}
void disable_interrupt_all(mac_t *mac)
{
    reg_write_32(mac->dma_addr + DmaInterrupt, DmaIntDisable);
    return;
}
void set_mac_addr(mac_t *mac)
{
    uint32_t data;
    uint8_t MacAddr[6] = {0x00, 0x55, 0x7b, 0xb5, 0x7d, 0xf7};
    uint32_t MacHigh = 0x40, MacLow = 0x44;
    data = (MacAddr[5] << 8) | MacAddr[4];
    reg_write_32(mac->mac_addr + MacHigh, data);
    data = (MacAddr[3] << 24) | (MacAddr[2] << 16) | (MacAddr[1] << 8) | MacAddr[0];
    reg_write_32(mac->mac_addr + MacLow, data);
}
uint32_t do_net_recv(uint32_t rd, uint32_t rd_phy, uint32_t daddr)
{

    reg_write_32(DMA_BASE_ADDR + 0xc, rd_phy);
    
    reg_write_32(GMAC_BASE_ADDR, reg_read_32(GMAC_BASE_ADDR) | 0x4);
    reg_write_32(DMA_BASE_ADDR + 0x18, reg_read_32(DMA_BASE_ADDR + 0x18) | 0x02200002); // start tx, rx
    reg_write_32(DMA_BASE_ADDR + 0x1c, 0x10001 | (1 << 6));
    // to do :write DMA reg 2
    /*
    int count = 1;
    while (1) {
        int i;
        for (i = 0; i < PNUM; i++) {
            rdes[i].des0 = 0x80000300;
            reg_write_32(DMA_BASE_ADDR + 0x8, 0x1);
            int own = rdes[i].des0 >> 31;
            while (own) {
                vt100_move_cursor(1, 11);
                printk("Waiting receive package...");
                own = rdes[i].des0 >> 31;
            }
            vt100_move_cursor(1, 3);
            printk("%d recv buff, rdes0 = 0x%x:\n", count, rdes[i].des0);
            count++;
            //rdes[i].des0 = rdes[i].des0 | 0x80000000;
            if (count > NUM_DMA_DESC) break;
        }
        if (count >= NUM_DMA_DESC) break;
    }
    */
    /*
    irq_enable(1);
    
    int i;
    for (i = 0; i < PNUM; i++) {
        rdes[i].des0 = 0x80000000;
    }
    */
    return 0;
}

void do_net_send(uint32_t td, uint32_t td_phy)
{
    reg_write_32(DMA_BASE_ADDR + 0x10, td_phy);

    // MAC rx/tx enable

    reg_write_32(GMAC_BASE_ADDR, reg_read_32(GMAC_BASE_ADDR) | 0x8);                    // enable MAC-TX
    reg_write_32(DMA_BASE_ADDR + 0x18, reg_read_32(DMA_BASE_ADDR + 0x18) | 0x02202000); //0x02202002); // start tx, rx
    reg_write_32(DMA_BASE_ADDR + 0x1c, 0x10001 | (1 << 6));
    //to do: write DMA reg 1
    
    int i;
    for (i = 0; i < PNUM; i++) {
        int own = tdes[i].des0 >> 31;
        if (!own) {
            tdes[i].des0 = 0x80000000;
            //tdes[i].des2 = ((uint32_t)&tbuf[i]) & 0x1fffffff;
            reg_write_32(DMA_BASE_ADDR + 0x4, 0x1);
            //do_sleep(1);
        }
        //tdes[i].des0 = tdes[i].des0 | 0x00000000;
    }
    
}

void do_init_mac(void)
{
    mac_t test_mac;
    uint32_t i;

    test_mac.mac_addr = 0xbfe10000;
    test_mac.dma_addr = 0xbfe11000;

    test_mac.psize = PSIZE * 4; // 64bytes
    test_mac.pnum = PNUM;       // pnum

    set_sram_ctr(); /* 使能GMAC0 */
    s_reset(&test_mac);
    disable_interrupt_all(&test_mac);
    set_mac_addr(&test_mac);
}

void do_wait_recv_package(void)
{
    //to do: block the recv thread
    do_block(&recv_block_queue);
    do_scheduler();
}
