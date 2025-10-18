#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h> 
#include <stdbool.h>

#include "queue.h"

typedef enum {
    TASK_STATUS_READY = 1,
    TASK_STATUS_RUNNING,
    TASK_STATUS_SUSPENDED,
    TASK_STATUS_TERMINATED,
} task_status_t;

typedef enum {
    TASK_TYPE_SYSTEM = 1,
    TASK_TYPE_USER,
} task_type_t;

typedef struct task_time_t
{
    unsigned int creation_time;
    unsigned int total_cpu_time;
    unsigned int activations;
    unsigned int last_start;
} task_time_t;

typedef struct task_t
{
  struct task_t *prev, *next;
  int id;
  ucontext_t context;
  short status;
  task_type_t type;
  int vg_id;
  int priority;
  int dynamic_priority;	
  short quantum;			
  short remaining_quantum;
  task_time_t time;
} task_t;

typedef struct ppos_core {
  unsigned int task_cnt;
  task_t *current_task;
  task_t *dispatcher_task;
  queue_t *ready_queue;
} ppos_core_t;

typedef struct
{
} semaphore_t ;

typedef struct
{
} mutex_t ;

typedef struct
{
} barrier_t ;

typedef struct
{
} mqueue_t ;

#endif

