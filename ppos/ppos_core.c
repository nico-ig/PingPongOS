#include <stdlib.h>

#include "ppos.h"
#include "logger.h"

#define STACKSIZE 64*1024

typedef enum {
    PPOS_CORE_ERR_NULL_PTR = -1,
    PPOS_CORE_ERR_CNTEX = -2,
} ppos_core_status_t;

typedef struct ppos_core {
    unsigned int task_cnt;
    task_t *current_task;
    task_t *main_task;
} ppos_core_t;

static ppos_core_t* ppos_core = NULL;

static void _ppos_core_destroy();
static void _ppos_core_incr_task_cnt();
static int _get_task_id(task_t *task);
static void _set_task_id(task_t *task);
static void _ppos_core_set_current_task(task_t *task);

void ppos_init () {
    LOG_TRACE0("ppos_init: entered");

    LOG_DEBUG0("ppos_init: setting stdout to unbuffered");
    setvbuf(stdout, 0, _IONBF, 0);

    LOG_DEBUG0("ppos_init: allocating ppos_core");
    ppos_core = (ppos_core_t*)calloc(1, sizeof(ppos_core_t));

    if (ppos_core == NULL) {
        LOG_ERR("ppos_init: failed to allocate ppos_core");
        goto exit;
    }

    LOG_DEBUG0("ppos_init: initializing ppos_core");
    ppos_core->task_cnt = 0;
    ppos_core->main_task = (task_t*)calloc(1, sizeof(task_t));

    if (ppos_core->main_task == NULL) {
        LOG_ERR("ppos_init: failed to allocate main task");
        goto exit;
    }

    LOG_DEBUG0("ppos_init: initializing main task");
    int ret = task_init(ppos_core->main_task, NULL, NULL);

    if (ret < 0) {
        LOG_ERR("ppos_init: failed to initialize main task");
        goto exit;
    }

    LOG_DEBUG0("ppos_init: setting current task to main task");
    _ppos_core_set_current_task(ppos_core->main_task);

exit:
    LOG_TRACE0("ppos_init: exiting (void)");
}

int task_init (task_t *task, void (*start_func)(void *), void *arg) {
    LOG_TRACE0("task_init: entered");
    int ret;

    LOG_TRACE0("task_init: getting context");
    getcontext(&task->context);

    LOG_TRACE0("task_init: allocating stack");
    stack_t *stack = calloc(1, STACKSIZE);

    if (stack == NULL) {
        LOG_ERR("task_init: failed to allocate stack");
        ret = PPOS_CORE_ERR_NULL_PTR;
        goto exit;
    }
    
    task->context.uc_stack.ss_sp = stack;
    task->context.uc_stack.ss_size = STACKSIZE;
    task->context.uc_stack.ss_flags = 0;
    task->context.uc_link = 0;

    LOG_TRACE0("task_init: making context");
    makecontext(&task->context, (void (*)(void))start_func, 1, arg);

    _set_task_id(task);
    LOG_INFO("task_init: new task with id %d created", _get_task_id(task));

    LOG_TRACE("task_init: exiting (%d)", _get_task_id(task));
    ret = _get_task_id(task);

exit:
    return ret;
}

int task_id () {
    LOG_TRACE0("task_id: entered");
    int id = _get_task_id(ppos_core->current_task);
    LOG_TRACE("task_id: exiting (%d)", id);
    return id;
}

void task_exit (int exit_code) {
    LOG_TRACE0("task_exit: entered");
    
    LOG_INFO("task_exit: task %d exiting", _get_task_id(ppos_core->current_task));

    if (ppos_core->current_task->id == 0) {
        LOG_DEBUG("task_exit: destroying ppos_core");
        _ppos_core_destroy();
    }
    else {
        LOG_DEBUG("task_exit: setting current task to main task");
        int ret = task_switch(ppos_core->main_task);

        if (ret < 0) {
            LOG_ERR("task_exit: failed to switch to main task");
            goto exit;
        }
    }

exit:
    LOG_TRACE0("task_exit: exiting (void)");
    return;
}

int task_switch (task_t *task) {
    LOG_TRACE0("task_switch: entered");

    task_t *current_task = ppos_core->current_task;

    LOG_DEBUG("task_switch: setting current task to task %d", _get_task_id(task));
    _ppos_core_set_current_task(task);

    LOG_INFO("task_switch: switching from task %d to task %d", _get_task_id(current_task), _get_task_id(task));
    int ret = swapcontext(&current_task->context, &task->context);

    if (ret < 0) {
        LOG_ERR("task_switch: failed to switch context (%d)", ret);
        ret = PPOS_CORE_ERR_CNTEX;
        goto exit;
    }
    
    ret = 0;

exit:
    LOG_TRACE("task_switch: exiting (%d)", ret);
    return ret;
}

static void _ppos_core_destroy() {
    LOG_TRACE0("_ppos_core_destroy: entered");

    if (ppos_core->main_task != NULL) {
        LOG_DEBUG0("_ppos_core_destroy: freeing main task");
        free(ppos_core->main_task);
        ppos_core->main_task = NULL;
    }

    if (ppos_core != NULL) {
        LOG_DEBUG0("_ppos_core_destroy: freeing ppos_core");
        free(ppos_core);
        ppos_core = NULL;
    }

    LOG_TRACE0("_ppos_core_destroy: exiting (void)");
}

static void _ppos_core_incr_task_cnt() {
    LOG_TRACE0("_ppos_core_incr_task_cnt: entered");
    ppos_core->task_cnt = ppos_core->task_cnt + 1;
    LOG_TRACE("_ppos_core_incr_task_cnt: task_cnt incremented to %d", ppos_core->task_cnt);
    LOG_TRACE0("_ppos_core_incr_task_cnt: exiting (void)");
}

static int _get_task_id(task_t *task) {
    LOG_TRACE0("_get_task_id: entered");
    int ret = task->id;
    LOG_TRACE("_get_task_id: exiting (%d)", ret);
    return ret;
}

static void _set_task_id(task_t *task) {
    LOG_TRACE0("_set_task_id: entered");

    LOG_TRACE("_set_task_id: setting task id to %d", task->id);
    task->id = ppos_core->task_cnt;
    _ppos_core_incr_task_cnt();

    LOG_TRACE0("_set_task_id: exiting (void)");
}

static void _ppos_core_set_current_task(task_t *task) {
    LOG_TRACE0("_ppos_core_set_current_task: entered");
    LOG_TRACE("_ppos_core_set_current_task: setting current task to task %d", _get_task_id(task));
    ppos_core->current_task = task;
    LOG_TRACE0("_ppos_core_set_current_task: exiting (void)");
}
