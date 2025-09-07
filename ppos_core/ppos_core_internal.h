#ifndef __PPOS_CORE_INTERNAL_H__
#define __PPOS_CORE_INTERNAL_H__

#include "ppos.h"

/*
* @brief Initialize PPOS core
* @return int: PPOS_CORE_SUCCESS on success, error code on failure
*/
int _init_ppos_core();

/*
* @brief Initialize system task
* @param task_ptr: pointer to the task structure
* @param type: type of the task
* @param start_func: function to be executed by the task
* @param arg: argument to be passed to the function
* @return int: PPOS_CORE_SUCCESS on success, error code on failure
*/
int _init_system_task(task_t **task_ptr, task_type_t type, void (*start_func)(void *), void *arg);

/*
* @brief Initialize main task
* @return int: PPOS_CORE_SUCCESS on success, error code on failure
*/
int _init_main_task();

/*
* @brief Initialize dispatcher task
* @return int: PPOS_CORE_SUCCESS on success, error code on failure
*/
int _init_dispatcher_task();

/*
* @brief Destroy PPOS core
* @return void
*/
void _ppos_core_destroy();

/*
* @brief Increment task count
* @return void
*/
void _ppos_core_incr_task_cnt();

/*
* @brief Set task ID
* @param task: pointer to the task structure
* @return void
*/
void _set_task_id(task_t *task);

/*
* @brief Set current task
* @param task: pointer to the task structure
* @return void
*/
void _ppos_core_set_current_task(task_t *task);

/*
* @brief Setup task stack
* @param task: pointer to the task structure
* @param stack: pointer to the stack structure
* @return void
*/
void _setup_task_stack(task_t *task, stack_t *stack);

/*
* @brief Link task to dispatcher
* @param task: pointer to the task structure
* @return void
*/
void _link_task_to_dispatcher(task_t *task);

/*
* @brief Main task wrapper
* @param arg: argument to be passed to the function
* @return void
*/
void _main_task_wrapper(void *arg);

/*
* @brief Cleanup task stack
* @param task: pointer to the task structure
* @return void
*/
void _cleanup_task_stack(task_t *task);

#endif // __PPOS_CORE_INTERNAL_H__
