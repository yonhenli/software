#ifndef _MACROS_H
#define _MACROS_H

/* Bit positions */
#define VALID			0
#define DIRTY			1
#define USED			2

/* Define a mask with nth bit set */
#define BIT_MASK(n) (1 << n)

/* bit corresponds to the bit number in flags. Least significant bit is 0 */
/* Test if bit is set in flags */
#define IS_SET(flags,bit) ((BIT_MASK(bit) & (flags)) > 0)
/* Set the bit in flags to 1 */
#define SET_BIT(flags,bit) (flags |= BIT_MASK(bit))
/* Set the bit in flags to 0 */
#define CLEAR_BIT(flags,bit) (flags &= ~BIT_MASK(bit))
/* Get the page number from a virtual address. */
#define VADDR_PAGENUM(addr) (addr >> (page_size/2))
/* Get the page offset from a virtual address. */
#define VADDR_OFFSET(addr) (addr & (BIT_MASK((page_size)/2)-1))

#endif
