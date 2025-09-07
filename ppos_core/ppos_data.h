// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>       // biblioteca POSIX de trocas de contexto
#include "queue.h"

// Tipo enum que define o status da tarefa
typedef enum {
    TASK_STATUS_READY = 1,
    TASK_STATUS_RUNNING,
    TASK_STATUS_TERMINATED,
} task_status_t;

// Tipo enum que define o tipo da tarefa
typedef enum {
    TASK_TYPE_MAIN = 1,
    TASK_TYPE_DISPATCHER,
    TASK_TYPE_USER,
} task_type_t;

typedef enum {
    PPOS_CORE_SUCCESS = 0,
    PPOS_CORE_ERR_NULL_PTR = -1,
    PPOS_CORE_ERR_CNTEX = -2,
} ppos_core_status_t;


// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
  struct task_t *prev, *next ;		// ponteiros para usar em filas
  int id ;				// identificador da tarefa
  ucontext_t context ;			// contexto armazenado da tarefa
  short status ;			// pronta, rodando, suspensa, ...
  task_type_t type ;			// tipo da tarefa
  int vg_id ;				// ID da pilha da tarefa no Valgrind
  // ... (outros campos serão adicionados mais tarde)
} task_t;

// Estrutura que define o core do sistema
typedef struct ppos_core {
  unsigned int task_cnt;
  task_t *current_task;
  task_t *dispatcher_task;
  task_t *main_task;
  queue_t *ready_queue;
} ppos_core_t;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

typedef struct
{
  // preencher quando necessário
} mqueue_t ;

#endif

