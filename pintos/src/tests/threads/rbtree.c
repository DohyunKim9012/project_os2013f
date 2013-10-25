/* Test program for lib/kernel/rbtree.c.*/

#undef NDEBUG
#include <debug.h>
#include <rbtree.h>
#include <random.h>
#include <stdio.h>
#include <stdbool.h>
#include "tests/threads/tests.h"

/* Maximum number of elements in a rb tree that we will
   test. */
#define MAX_SIZE 64

struct rb_root root = RB_ROOT;

/* A rb tree element. */
struct value 
  {
    struct rb_node elem;      /* List element. */
    int value;                  /* Item value. */
  };

struct value* rb_search(struct rb_root *root, int key);
int rb_insert(struct rb_root *root, struct value *data);
void rb_remove(struct rb_root *root, struct value *datA);

/* Test the linked list implementation. */
void
test_rbtree (void) 
{
  bool flag;
  int i, j, random, prev;
  static struct value values[MAX_SIZE];
  struct rb_node *node;
  struct value *this;

  printf("Inserting: ");
  /* Initialize MAX_SIZE Nodes with random number */
  for (i = 0; i < MAX_SIZE; i++)
  {
    flag= false;
    while (!flag)
    {
      flag= true;
      random = random_ulong() & 0xffffff;

      /* Check for Duplicates */
      for (j = 0; j < i; j++)
      {
        if (values[j].value == random)
        {
          flag = false;
          break;
        }
      }
    }
    
    values[i].value = random;
    printf("%d ", values[i].value);
    rb_insert(&root, &values[i]);
  }
  printf(": COMPLETED\n");

  printf("Insertion Check: \n");
  /* For every entry, make sure that all entries are properly inserted */
  for (i = 0; i < MAX_SIZE; i++)
  {
    ASSERT (rb_search (&root,values[i].value) != NULL); 
  }
  printf("Verified\n");

  printf("Removal Check:\n");
  flag = false;
  /* Pop all nodes in increasing Order */
  do
  {
    node = rb_first (&root);
    this = rb_entry (node, struct value, elem);
    printf("%d ", this->value);

    if (flag)
    {
      ASSERT (prev < this->value);
    }
    
    flag = true;
    prev = this->value;
    rb_remove(&root, this);

  } while (root.rb_node != NULL);

  printf ("Verified\n");
  printf ("rbtree: PASS\n");
}

struct value* rb_search(struct rb_root *root, int key)
{
  int result;
  struct value *data;
  struct rb_node *node = root->rb_node;
  
  while (node)
  {
    data = rb_entry(node, struct value, elem);
    
    result = key-data->value;
    
    if (result < 0)
      node = node->rb_left;
     else if (result > 0)
       node = node->rb_right;
    else
      return data;
  }
  return NULL;
}

int rb_insert(struct rb_root *root, struct value *data)
{
  int result;
  struct value *this;
  struct rb_node **new = &(root->rb_node), *parent = NULL;

  /* Figure out where to put new node */
  while (*new)
  {
    this = rb_entry (*new, struct value, elem);

    result = data->value - this->value;   
    
    parent = *new;
    
    if (result < 0)    
      new = &((*new)->rb_left);
    else if (result > 0)    
      new = &((*new)->rb_right);      
    else     
      return false;
  }
  
  /* Add new node and rebalance tree. */
  rb_link_node(&data->elem, parent, new);
  rb_insert_color(&data->elem, root);
  
  return true;
}

void rb_remove(struct rb_root *root, struct value *data)
{
  ASSERT (data != NULL);
  rb_erase(&data->elem, root);
}
