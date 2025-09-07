#ifndef __TASK_H__
#define __TASK_H__

#include "ppos.h"

/*
* @brief Create task context with stack and function
* @param task: pointer to the task structure
* @param start_func: function to be executed by the task
* @param arg: argument to be passed to the function
* @return int: PPOS_CORE_SUCCESS on success, error code on failure
*/
int task_create_context(task_t *task, void (*start_func)(void *), void *arg);

/*
* @brief Handle user task initialization and add to ready queue
* @param task: pointer to the task to be initialized
* @return int: PPOS_CORE_SUCCESS on success, error code on failure
*/
int task_handle_user_initialization(task_t *task);

/*
* @brief Validate context switch parameters
* @param task: pointer to the task to switch to
* @return int: PPOS_CORE_SUCCESS on success, error code on failure
*/
int task_validate_context_switch_params(task_t *task);

/*
* @brief Execute a task by switching to it
* @param task: pointer to the task to execute
* @return void
*/
void task_execute(task_t *task);

/*
* @brief Handle task status and cleanup if terminated
* @param task: pointer to the task to handle
* @return void
*/
void task_handle_status(task_t *task);

/*
* @brief Cleanup terminated task resources
* @param task: pointer to the terminated task
* @return void
*/
void task_cleanup_terminated(task_t *task);

#endif
