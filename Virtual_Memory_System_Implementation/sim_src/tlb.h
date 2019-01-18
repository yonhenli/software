#ifndef _TLB_H_
#define _TLB_H_

#include "types.h"

/*******************************************************************************
 * TLB entry.
 */
typedef struct {
  vpn_t vpn;     /* Virtual page number */
  pfn_t pfn;     /* Physical frame number */
  /* Flags have the following bit configuration:
   *  X X X X X UF DF VF
   *  where:
   *    X = Unused, 
   *    DF = Dirty flag. 1 implies this page has changed, and must be written to disk.
   *    VF = Valid flag. 1 implies this page is valid.
   *    UF = Used flag. 1 implies this page was used recently
   */
  uint8_t flags;
} tlbe_t;

/*******************************************************************************
 * Make the TLB an extern variable, so that other files can access it.
 */
extern tlbe_t* tlb;

/*******************************************************************************
 * Looks up an address in the TLB. If no entry is found, attempts to access the
 * current page table via pagetable_lookup().
 *
 * @param vpn The virtual page number to lookup.
 * @param write If the access is a write, this is 1. Otherwise, it is 0.
 * @return The physical frame number of the page we are accessing.
 */
pfn_t tlb_lookup(vpn_t vpn, int write);

/*******************************************************************************
 * Clears ALL entries in the TLB. Used on context switch so that virtual
 * mappings are valid.
 */
void tlb_clearall(void);

/*******************************************************************************
 * Clears a single TLB entry, specified by the virtual page number. Used when
 * pages are swapped out so all virtual mappings remain valid.
 *
 * @param vpn The virtual page number of the TLB entry to invalidate.
 */
void tlb_clearone(vpn_t vpn);

/*******************************************************************************
 * Initializes the memory for representing the TLB.
 */
void tlb_init(void);

/*******************************************************************************
 * Frees the memory used for representing the TLB.
 */
void tlb_free(void);

#endif/*_TLB_H_*/
