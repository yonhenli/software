#include <stdio.h>

#include "stats.h"

/*******************************************************************************
 * Number of pagefaults generated during execution.
 */
unsigned long int pagefaults_count = 0;

/*******************************************************************************
 * Number of TLB hits generated during execution.
 */
unsigned long int tlbhits_count = 0;

/*******************************************************************************
 * Raw number of stores executed.
 */
unsigned long int writes_count = 0;

/*******************************************************************************
 * Raw number of loads executed.
 */
unsigned long int reads_count = 0;

/*******************************************************************************
 * Raw number of disk accesses.
 */
unsigned long int diskio_count = 0;

/*******************************************************************************
 * Display the stastics.
 */
void display_stats() {
  printf("\n-----------------------\n"
         "Statistics:\n"
         "-----------------------\n"
         "Reads:         %10lu\n"
         "Writes:        %10lu\n"
         "TLB Hits:      %10lu\n"
         "Page Faults:   %10lu\n"
	 "Disk IO: %10lu\n\n"
         "Effective Mean Access Time:          %f ns\n",
         reads_count, writes_count, tlbhits_count, pagefaults_count, 
         diskio_count, compute_access_time());
}
