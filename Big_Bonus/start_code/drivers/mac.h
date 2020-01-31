#ifndef INCLUDE_MAC_H_
#define INCLUDE_MAC_H_

#include "type.h"
#include "queue.h"
//#define TEST_REGS1
//#define TEST_REGS2
//#define TEST_REGS3
#define GMAC_BASE_ADDR (0xbfe10000)
#define DMA_BASE_ADDR (0xbfe11000)
#define PSIZE (256)
#define PNUM (64)
//#define TDES_BASE_ADDR 0xa0f00000
#define RECV_BASE_ADDR 0xa0fd0000
#define NUM_DMA_DESC 20

extern queue_t recv_block_queue;
//extern uint32_t recv_flag[PNUM];
//extern uint32_t ch_flag;

//extern uint32_t tbuf[PNUM][24];
//extern uint32_t rbuf[PNUM][19];

enum GmacRegisters
{
    GmacAddr0Low = 0x0044,  /* Mac frame filtering controls */
    GmacAddr0High = 0x0040, /* Mac address0 high Register  */
};
enum DmaRegisters
{
    DmaStatus = 0x0014,    /* CSR5 - Dma status Register                        */
    DmaInterrupt = 0x001C, /* CSR7 - Interrupt enable                           */
    DmaControl = 0x0018,   /* CSR6 - Dma Operation Mode Register                */
};
enum DmaControlReg
{

    DmaStoreAndForward = 0x00200000, /* (SF)Store and forward                            21      RW        0       */
    DmaRxThreshCtrl128 = 0x00000018, /* (RTC)Controls thre Threh of MTL tx Fifo 128      4:3   RW                */

    DmaTxStart = 0x00002000, /* (ST)Start/Stop transmission                      13      RW        0       */

    DmaTxSecondFrame = 0x00000004, /* (OSF)Operate on second frame                     4       RW        0       */
};
enum InitialRegisters
{
    DmaIntDisable = 0,
};
typedef struct desc
{
    uint32_t des0;
    uint32_t des1;
    uint32_t des2;
    uint32_t des3;
} desc_t;
typedef struct mac
{
    uint32_t psize; // backpack size
    uint32_t pnum;
    uint32_t mac_addr; // MAC base address
    uint32_t dma_addr; // DMA base address

    uint32_t saddr; // send address
    uint32_t daddr; // receive address

    uint32_t saddr_phy; // send phy address
    uint32_t daddr_phy; // receive phy address

    uint32_t td; // DMA send desc
    uint32_t rd; // DMA receive desc

    uint32_t td_phy;
    uint32_t rd_phy;

} mac_t;

//extern desc_t tdes[PNUM];
extern desc_t rdes[PNUM];
extern int mac_cnt;

uint32_t read_register(uint32_t base, uint32_t offset);
void reg_write_32(uint32_t addr, uint32_t data);
uint32_t reg_read_32(uint32_t addr);
void clear_interrupt();
uint32_t do_net_recv(uint32_t rd, uint32_t rd_phy, uint32_t daddr);
void do_init_mac(void);
void do_wait_recv_package(void);
void mac_irq_handle(void);


#endif
