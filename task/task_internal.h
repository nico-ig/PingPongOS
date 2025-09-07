#ifndef __TASK_INTERNAL_H__
#define __TASK_INTERNAL_H__

#include "ppos.h"

/*
* @brief Create task context with stack and function
* @param task: pointer to the task structure
* @param start_func: function to be executed by the task
* @param arg: argument to be passed to the function
* @return int: PPOS_CORE_SUCCESS on success, error code on failure
*/
int _create_task_context_internal(task_t *task, void (*start_func)(void *), void *arg);

/*
* @brief Handle user task initialization
* @param task: pointer to the task structure
* @return int: PPOS_CORE_SUCCESS on success, error code on failure
*/
int _handle_user_task_initialization_internal(task_t *task);

/*
* @brief Validate context switch parameters
* @param task: pointer to the task structure
* @return int: PPOS_CORE_SUCCESS on success, error code on failure
*/
int _validate_context_switch_params_internal(task_t *task);

/*
* @brief Cleanup terminated task
* @param task: pointer to the task structure
* @return void
*/
void _cleanup_terminated_task_internal(task_t *task);

#endif // __TASK_INTERNAL_H__
