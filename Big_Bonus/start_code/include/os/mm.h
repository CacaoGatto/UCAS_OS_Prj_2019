#ifndef INCLUDE_MM_H_
#define INCLUDE_MM_H_

#include "type.h"

/*
-----------------------------------------------------------------
|    Virtual Mapping    | PTE
-----------------------------------------------------------------
| 0xb0000000+0x10000000 |
------------------------------------------------------------------
*/

#define TLB_ENTRY_NUMBER 64
#define MAX_PAGE 256
#define MAX_PTE  1024
#define P_ZONE   0x800
//#define PTE_SRORE_BASE 0xa0e00000
//#define P_MAPPING_BASE 0x00000800
#define VM_BASE_ADDR   0xa1000000
#define PTE_BASE_ADDR  0xa0ffd000
#define VM_BITMAPPING  0xa0ffe000


typedef struct pte{
    uint32_t entryhi;
    uint32_t entrylo;
} pte_t; //8B
/*
typedef struct ref{
    pte_t* pte_list[MAX_PTE];
//    int valid;
} pte_page;
*/
//pte_t pte_list[MAX_PTE];
//extern pte_page* pte_reference[MAX_PAGE];
//extern char physical_valid[P_ZONE];

void do_TLB_Refill();
void do_page_fault();

void mod_helper();
/*
void init_page(uint8_t);
void init_pte(uint8_t, uint16_t, uint32_t);
*/
uint16_t alloc_page();
void free_page(uint32_t);
uint16_t alloc_top_pte(uint8_t);

extern uint8_t tlb_index;
/*
extern uint16_t pte_count;
extern uint8_t page_count;
*/
#endif
