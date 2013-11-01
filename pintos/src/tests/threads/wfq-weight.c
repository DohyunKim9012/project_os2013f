/* Checks that when the alarm clock wakes up threads, the
   higher-priority threads run first. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "devices/timer.h"
#include "threads/sched.h"

void test_wfq_weight(void);
void test_weight(int fairness, int iteration);
static void wfq_weight_thread(void *_test);

static thread_func wfq_weight_thread;
static int64_t wake_time;
static struct semaphore wait_sema;

struct test_data
{
  int tid;
  long result;
  long runtime;
  long prev_runtime;
  int terminate;
};

static const int priority_table[5][10] = {
  {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {-4, -1, -1, -1, -1, -1, -1, -1, -1},
  {-6, -1, -1, -1, -1, -1, -1, -1},
  {-7, -1, -1, -1, -1, -1, -1},
  {-8, -1, -1, -1, -1, -1},
};

void
test_wfq_weight (void) 
{
  /* Can configure test data.
   * fairness(threads) : 10(10), 20(9), 30(8), 40(7), 50(6)
   * iteration : positive integer */
  int array[5] = { 10, 20, 30, 40, 50 };
  int i;

  for ( i = 0; i < 5; i++ )
  {
    msg("%d", array[i]);
    test_weight(array[i], 100);
  }
}

void
test_weight (int fairness, int iteration)
{
  int i, j, k;
  int priority_index = (fairness / 10) - 1;
  int num_of_threads = 11 - (fairness / 10);
  struct test_data test[num_of_threads];

  sema_init(&wait_sema, 0);
  thread_set_priority (PRI_MIN);

  /* Creates threads. */
  for(i = 0; i < iteration; i++) {
    wake_time = timer_ticks () + TIMER_FREQ;

    for (j = 0; j < num_of_threads; j++) {
      test[j].terminate = 0;
      int priority = priority_table[priority_index][j];
      char name[16];
      snprintf (name, sizeof name, "priority %d", priority);
      /* Makes thread. */
      thread_create (name, priority, wfq_weight_thread, &test[j]);
    }

    /* Wakes same time with child threads. */
    timer_sleep (wake_time - timer_ticks ());

    /* Checks exec time and prints. */
    for(j = 0; j < num_of_threads; j++)
      test[j].prev_runtime = test[j].runtime;
    timer_sleep(10 * TIMER_FREQ);
    for(j = 0; j < num_of_threads; j++)
      test[j].result = test[j].runtime - test[j].prev_runtime;

    printf("%d ", i);

    /* Prints fairness data. */
    for(j = 0; j < num_of_threads; j++)
      printf("%ld\t", test[j].result);
    printf("\n");

    /* Waits for child thread to terminate. */
    for(j = 0; j < num_of_threads; j++) {
      test[j].terminate = 1;
      sema_down(&wait_sema);
    }
  }
}

static void
wfq_weight_thread (void *_test) 
{
  struct test_data *test = _test;

  /* Busy-wait until the current time changes. */
  int64_t start_time = timer_ticks ();
  while (timer_elapsed (start_time) == 0)
    continue;

  /* Now we know we're at the very beginning of a timer tick, so
     we can call timer_sleep() without worrying about races
     between checking the time and a timer interrupt. */
  timer_sleep (wake_time - timer_ticks ());

  test->tid = thread_tid();
  while(test->terminate != 1)
    test->runtime = thread_runtime();

  sema_up(&wait_sema);
}
