#ifndef _STASTICS_H_
#define _STASTICS_H_

/*******************************************************************************
 * Display the stastics gathered from executing the simulator.
 */
void display_stats();

/*******************************************************************************
 * Compute the Average Mean Access Time.
 */
double compute_access_time();

/*******************************************************************************
 * Number of pagefaults generated during execution.
 */
extern unsigned long int pagefaults_count;

/*******************************************************************************
 * Number of TLB hits generated during execution.
 */
extern unsigned long int tlbhits_count;

/*******************************************************************************
 * Raw number of stores executed.
 */
extern unsigned long int writes_count;

/*******************************************************************************
 * Raw number of loads executed.
 */
extern unsigned long int reads_count;

/*******************************************************************************
 * Raw number of loads executed.
 */
extern unsigned long int diskio_count;

#endif/*_STASTICS_H_*/
