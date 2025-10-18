#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>

#include "ppos.h"
#include "logger.h"

#define TIMER_SIGNAL (int)SIGALRM

static struct sigaction action;
static struct itimerval timer;

int timer_signal()
{
    return TIMER_SIGNAL;
}

static void _register_signal(int signum, void (*tick_handler)(int))
{
    action.sa_handler = tick_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(signum, &action, 0) < 0)
    {
        LOG_ERR("register_signal: failed to register signal %d, exiting", signum);
        exit(-1);
    }
}

static void _set_timer(struct itimerval *timer, long interval_ms)
{
    if (timer == NULL)
    {
        LOG_ERR("set_timer: timer is NULL, exiting");
        exit(-1);
    }
    
    long interval_us = interval_ms * 1000;
    
    timer->it_value.tv_usec = interval_us;
    timer->it_value.tv_sec  = 0;
    timer->it_interval.tv_usec = interval_us;
    timer->it_interval.tv_sec  = 0;

    if (setitimer(ITIMER_REAL, timer, 0) < 0)
    {
        LOG_ERR("set_timer: failed to set timer with error: \"%s\", exiting", strerror(errno));
        exit(-1);
    }
}

void timer_init(void (*tick_handler)(int), long interval_ms)
{
    if (tick_handler == NULL)
    {
        LOG_ERR("timer_init: tick_handler is NULL, exiting");
        exit(-1);
    }
    
    LOG_INFO("timer_init: starting timer with interval %ld ms", interval_ms);
    _register_signal(timer_signal(), tick_handler);
    _set_timer(&timer, interval_ms);
}
