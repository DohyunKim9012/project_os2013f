/* Checks that when the alarm clock wakes up threads, the
   higher-priority threads run first. */

#include <stdio.h>
#include <random.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/sched.h"
#include "devices/timer.h"

static thread_func wfq_overhead_thread;
void test_overhead(int num_of_threads, int iteration);

/* Test call function */
void
test_wfq_overhead(void) {
  /* Can configure test data.
   * threads : positive integer
   * iteration : positive integer */
  int i = 0;
  int array[11] = { 2, 5, 10, 25, 50, 75, 100, 250, 500, 750, 1000};
  for (i = 0; i < 11; i++)
  {
    msg("%d", array[i]);
    test_overhead(array[i], 200);
  }
}

int64_t wake_time;

/* Sub function */
void
test_overhead(int threads, int iteration)
{
  int i;

  thread_set_priority(PRI_MAX);
  wake_time = timer_ticks () + TIMER_FREQ * 5;
  
  init_overhead_measurement ();

  /* Initialization process */
  for (i = 0; i < threads; i++)
  {
    /* Sets priority & name. */
    int priority = PRI_DEFAULT;
    char name[16];
    snprintf(name, sizeof name, "priority %d", priority);
    
    /* Creates thread */
    printf("%d ",thread_create(name, priority, wfq_overhead_thread, &iteration));
  }

  thread_set_priority(PRI_MIN);

  while (overhead_measurement_times() < iteration);

  msg("completed");

  timer_sleep (10000);

  return;
}

static void
wfq_overhead_thread(void *iter)
{/*
  int iteration = *(int*)iter;
  uint64_t overhead;
  int64_t start_time = timer_ticks ();

  while (timer_elapsed (start_time) == 0);

  timer_sleep (wake_time - timer_ticks());

  while (overhead_measurement_times () < iteration)
  {
    overhead = thread_overhead ();
    
    if (overhead > 0)
    {
      printf("%d: %llu\n", overhead_measurement_times (), overhead);
    }
  }*/
}
