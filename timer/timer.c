#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>

#include "ppos.h"
#include "logger.h"

#define TIMER_SIGNAL (int)SIGALRM

static struct sigaction _action;
static struct itimerval _timer;
static void (*_usr_tick_handler)(int);
static unsigned int _system_ticks = 0;

int timer_signal()
{
    return TIMER_SIGNAL;
}

static void tick_handler(int signum)
{
    _system_ticks++;
    _usr_tick_handler(signum);
}

static void _register_signal(void (*usr_tick_handler)(int))
{
    _usr_tick_handler = usr_tick_handler;
    _action.sa_handler = tick_handler;
    sigemptyset(&_action.sa_mask);
    _action.sa_flags = 0;
    if (sigaction(timer_signal(), &_action, 0) < 0)
    {
        LOG_ERR("register_signal: failed to register signal %d, exiting", timer_signal());
        exit(-1);
    }
}

static void _set_timer(long interval_ms)
{  
    long interval_us = interval_ms * 1000;
    
    _timer.it_value.tv_usec = interval_us;
    _timer.it_value.tv_sec  = 0;
    _timer.it_interval.tv_usec = interval_us;
    _timer.it_interval.tv_sec  = 0;

    if (setitimer(ITIMER_REAL, &_timer, 0) < 0)
    {
        LOG_ERR("set_timer: failed to set timer with error: \"%s\", exiting", strerror(errno));
        exit(-1);
    }
}

unsigned int systime()
{
    int interval_ms = _timer.it_value.tv_usec / 1000;
    return _system_ticks * interval_ms;
}

void timer_init(void (*usr_tick_handler)(int), long interval_ms)
{
    if (usr_tick_handler == NULL)
    {
        LOG_ERR("timer_init: usr_tick_handler is NULL, exiting");
        exit(-1);
    }
    
    LOG_INFO("timer_init: starting timer with interval %ld ms", interval_ms);
    _register_signal(usr_tick_handler);
    _set_timer(interval_ms);
}
