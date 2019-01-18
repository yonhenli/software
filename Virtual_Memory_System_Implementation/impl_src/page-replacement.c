#include <stdlib.h>

#include "types.h"
#include "pagetable.h"
#include "global.h"
#include "process.h"
#include "macros.h"

/*******************************************************************************
 * Finds a free physical frame. If none are available, uses a clock sweep
 * algorithm to find a used frame for eviction.
 *
 * @return The physical frame number of a free (or evictable) frame.
 */

/* A physical page is also called a physical frame or frame */
pfn_t get_free_frame(void)
{
	/* Iterate through the reverse lookup table to see if there is a free frame. If so, return it. 
	 * HINT: The pcb (or the owning process) for a free frame will be NULL. 
	 */
	int i = 0;
	for (i = 0; i < NUM_PHYS_PAGES; i++) {
		if (rlt[i].pcb == NULL) {
			return (pfn_t) i;
		}
	}

	/* If none of the frames are free, we must evict a frame.
	 * For each frame, if it is used, set it as unused. If it is already unused, return the frame. 
	 * Repeat the previous step till you find an unused frame. 
	 */ 
	while (1) {
		for (i = 0; i < NUM_PHYS_PAGES; i++) {
			if (IS_SET(rlt[i].pcb->pagetable[rlt[i].vpn].flags, USED)) {
				CLEAR_BIT(rlt[i].pcb->pagetable[rlt[i].vpn].flags, USED);
			}
			else {
				/*QUESTION: not sure about this step!*/
				return rlt[i].pcb->pagetable[rlt[i].vpn].pfn;
			}
		}
	}

	/* This final return is just to satisfy the compiler. 
	 * Control should never reach here. We should have found a frame earlier. 
	 */
	return 0;

}
