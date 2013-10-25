#ifndef THREADS_SCHED_H
#define THREADS_SCHED_H

/* If WFQ Implemented
   1. #define WFQ
   2. Do Nothing if WFQ is not implemented 
*/
#define WFQ

/* If WFQ is Implemented, there are two choices
   1. #define WFQ_RBTREE : WFQ implementaion with RBTREE
   2. By default, list implementation is used
*/
#define WFQ_RBTREE

#include <stdint.h>

#ifdef WFQ_RBTREE
#include <rbtree.h>
#else
#include <list.h>
#endif

/* Several constants are defined here */

/* Thread priorities. */
#define PRI_MIN -20                     /* Lowest priority. */
#define PRI_DEFAULT 0                   /* Default priority. */
#define PRI_MAX 19                      /* Highest priority. */

/* Priority - Weight - vruntime Table
 * Inspired by CFS, priority value specified here
 * are actually following nice value's scaling for
 * weights.
 * */
static const int prio_to_weight[40] = {
  /* -20 */     88761,     71755,     56483,     46273,     36291,
  /* -15 */     29154,     23254,     18705,     14949,     11916,
  /* -10 */      9548,      7620,      6100,      4904,      3906,
  /*  -5 */      3121,      2501,      1991,      1586,      1277,
  /*   0 */      1024,       820,       655,       526,       423,
  /*   5 */       335,       272,       215,       172,       137,
  /*  10 */       110,        87,        70,        56,        45,
  /*  15 */        36,        29,        23,        18,        15,
};

/* Inverse (2^32/weight) values of the prio_to_weight array
 * This array is also from linux kernel */
static const uint32_t prio_to_vruntime[40] = {
  /* -20 */     48388,     59856,     76040,     92818,    118348,
  /* -15 */    147320,    184698,    229616,    287308,    360437,
  /* -10 */    449829,    563644,    704093,    875809,   1099582,
  /*  -5 */   1376151,   1717300,   2157191,   2708050,   3363326,
  /*   0 */   4194304,   5237765,   6557202,   8165337,  10153587,
  /*   5 */  12820798,  15790321,  19976592,  24970740,  31350126,
  /*  10 */  39045157,  49367440,  61356676,  76695844,  95443717,
  /*  15 */ 119304647, 148102320, 186737708, 238609294, 286331153,
};

/* A schedule entity for scheduling */
struct sched_entity {

  /* Priority value and its given weight */
  int priority;
  int weight;

  /* Virtual runtime value for given weight, 
   * specifically, it is inverse of weight.
   * this value is summed to sum_vruntime if the thread
   * is scheduled to execute. */
  uint32_t vruntime;
  uint64_t sum_vruntime;

  /* Total execution time. The time in unit of ticks from the very
   * first execution of the thread. */
  long sum_runtime;

#ifdef WFQ_RBTREE
  /* rb_node */
  struct rb_node sched_node;
#else
  /* list node */
  struct list_elem sched_elem;
#endif

	/* if thread is sleeping, specify wake up time in units of ticks */
	int64_t wake_up_tick;
};

#ifdef WFQ

// Initializes scheduler
void init_sched (void);

// Initializes schedule entitiy with given priority
void init_sched_entity (struct sched_entity*, int);

void sched_set_priority (struct sched_entity*, int);

struct sched_entity* sched_pick_next(void);
void sched_insert (struct sched_entity*);
void sched_erase (struct sched_entity*);
void sched_update (struct sched_entity*, unsigned);

#endif // WFQ

#endif // _THREADS_SCHED_H
