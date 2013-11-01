#include "threads/sched.h"
#include "threads/interrupt.h"

#include <debug.h>
#include <stddef.h>

/* Initializes sched_entity */
void
init_sched_entity(struct sched_entity* se, int new_priority)
{
  se->sum_vruntime = 0;
  se->sum_runtime = 0;
  se->wake_up_tick = 0;
  sched_set_priority(se, new_priority);

  /* For measurement */
  se->overhead = 0;
}

/* Sets priority.
 * Also sets weight and vruntime value following the priority */
void
sched_set_priority (struct sched_entity* se, int new_priority)
{
  /* Sets priority first */
  se->priority = (new_priority > PRI_MAX) ? PRI_MAX : \
                  ((new_priority < PRI_MIN) ? PRI_MIN : new_priority);
  se->weight = prio_to_weight[se->priority+20];
  se->vruntime = prio_to_vruntime[se->priority+20];
}

/* Updates schedule entity if the thread is ready to run */
void sched_update (struct sched_entity* se, unsigned ticks)
{
  se->sum_vruntime += se->vruntime * ticks;
  se->sum_runtime += ticks;
}

#ifdef WFQ_RBTREE // RB TREE IMPLEMENTATION

/* WFQ Scheduler's ready queue
 * Red Black Tree for storing schedule entities */
struct rb_root sched_rq;

/* Initializes WFQ scheduler */
void
init_sched()
{
  sched_rq = RB_ROOT;
}


/* Among the schedule entities in the ready queue, select the minimum node
 * delete it from sched_rq and return */
struct sched_entity*
sched_pick_next (void)
{
  struct rb_node *next = rb_first (&sched_rq);

  ASSERT (intr_get_level () == INTR_OFF);

  if (next != NULL)
  {
    rb_erase(next, &sched_rq);
    return rb_entry (next, struct sched_entity, sched_node);
  }
  else
  {
    return NULL;
  }
}

/* After executing, insert the scheduled entity to ready queue again.
 */
void
sched_insert (struct sched_entity* se)
{
  bool cmp;
  struct rb_node **new = &((&sched_rq)->rb_node), *parent = NULL;

  while (*new)
  {
    struct sched_entity *this = rb_entry (*new, struct sched_entity, sched_node);
    cmp = (se->sum_vruntime <= this->sum_vruntime);

    parent = *new;
    if (cmp)
    {
      new = &((*new)->rb_left);
    }
    else
    {
      new = &((*new)->rb_right);
    }
  }

  rb_link_node(&se->sched_node, parent, new);
  rb_insert_color(&se->sched_node, &sched_rq);
}

void sched_erase (struct sched_entity* se)
{
  rb_erase (&se->sched_node, &sched_rq);
}

#else
// LIST IMPLEMENTATION

/* WFQ Scheduler's ready queue
 * list for storing schedule entities */
static struct list sched_rq;

/* Initializes WFQ scheduler */
void
init_sched()
{
  list_init(&sched_rq);
}

/* Among the schedule entities in the ready queue, select the minimum node
 * delete it from sched_rq and return */
struct sched_entity*
sched_pick_next (void)
{
  struct sched_entity *next;

  ASSERT (intr_get_level () == INTR_OFF);

  if (!list_empty(&sched_rq))
  {
    next = list_entry(list_front(&sched_rq), struct sched_entity, sched_elem);
    list_pop_front(&sched_rq);
    return next;
  }
  else
  {
    return NULL;
  }
}

static bool
vruntime_cmp (const struct list_elem *elem_1, const struct list_elem *elem_2, void *aux UNUSED)
{
  struct sched_entity *se_1 = list_entry(elem_1, struct sched_entity, sched_elem);
  struct sched_entity *se_2 = list_entry(elem_2, struct sched_entity, sched_elem);
  return (se_1->sum_vruntime <= se_2->sum_vruntime);
}

/* After executing, insert the scheduled entity to ready queue again.
 */
void
sched_insert (struct sched_entity* se)
{
  list_insert_ordered(&sched_rq, &se->sched_elem, vruntime_cmp, NULL);
}

void
sched_erase (struct sched_entity* se)
{
  list_remove(&se->sched_elem);
}

#endif
