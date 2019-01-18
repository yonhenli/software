#include <stdio.h>
#include <assert.h>

#include "types.h"
#include "process.h"
#include "global.h"
#include "swapfile.h"
#include "tlb.h"
#include "macros.h"

/*******************************************************************************
 * Implement the pagefault handler.
 * Page fault handler. When the CPU encounters an invalid address mapping in a
 * process' page table, it invokes OS through this handler. The OS then
 * allocates a physical frame for the requested page (either by using a free
 * frame or evicting one), changes the process' page table to reflect the
 * mapping and then restarts the interrupted process.
 *
 * @param vpn The virtual page number requested.
 * @param write If the CPU is writing to the page, this is 1. Otherwise, it's 0.
 * @return The physical frame the OS has mapped to the virtual page.
 */
pfn_t pagefault_handler(vpn_t request_vpn, int write) {
	pfn_t victim_pfn;
	victim_pfn = 0;

	/* Find a victim frame. Hint: Refer page-replacement.c.  */
	/* Use the reverse lookup table to find the victim's owner process and corresponding virtual page. */
	victim_pfn = get_free_frame();

	/*
	 * If victim page is occupied:
	 *
	 * 1) If it's dirty, save it to disk with page_to_disk()
	 * 2) Invalidate the page's entry in the victim's page table.
	 * 3) Clear the victim page's TLB entry (hint: tlb_clearone()).
	 */
	if (rlt[victim_pfn].pcb) {
		if (IS_SET(rlt[victim_pfn].pcb -> pagetable[rlt[victim_pfn].vpn].flags, DIRTY)) {
			page_to_disk(victim_pfn, rlt[victim_pfn].vpn, rlt[victim_pfn].pcb -> pid);
		 	CLEAR_BIT(rlt[victim_pfn].pcb -> pagetable[rlt[victim_pfn].vpn].flags, DIRTY);
		}

		CLEAR_BIT(rlt[victim_pfn].pcb -> pagetable[rlt[victim_pfn].vpn].flags, VALID);
		tlb_clearone(rlt[victim_pfn].vpn);
	}

	/* Update the reverse lookup table so that the frame's ownership is updated to the requesting process (which is nothing but the current process!).
	 * Update the requesting process' page table. Set the page table entry as used and valid. Also, if writing to the page, set the dirty bit.
	 *
	 * Finally, retrieve the page from disk. Note that is really a lie: we save pages in
	 * memory (since doing file I/O for this simulation would be annoying and
	 * wouldn't add that much to the learning). Also, if the page technically
	 * does't exist yet (i.e., the page has never been accessed yet, we return a
	 * blank page. Real systems would check for invalid pages and possibly read
	 * stuff like code pages from disk. For purposes of this simulation, we won't
	 * worry about that. =)
	 * Hint: page_from_disk()
	 */

	rlt[victim_pfn].pcb = current;
    rlt[victim_pfn].vpn = request_vpn;
    rlt[victim_pfn].pcb -> pagetable[rlt[victim_pfn].vpn].pfn = victim_pfn;

	SET_BIT(rlt[victim_pfn].pcb -> pagetable[rlt[victim_pfn].vpn].flags, USED);
	SET_BIT(rlt[victim_pfn].pcb -> pagetable[rlt[victim_pfn].vpn].flags, VALID);

	if (write) SET_BIT(rlt[victim_pfn].pcb -> pagetable[rlt[victim_pfn].vpn].flags, DIRTY);

	page_from_disk(victim_pfn, rlt[victim_pfn].vpn, rlt[victim_pfn].pcb -> pid);

    return victim_pfn;
}
