#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>

#include "ppos.h"
#include "logger.h"

#define TIMER_SIGNAL (int)SIGALRM
#define MAX_HANDLERS 2
#define BASE_INTERVAL_MS (long)1

typedef struct {
    void (*handler)(int);
    unsigned int interval_ms;
    unsigned int next_tick;
} timer_handler_t;

static timer_handler_t _handlers[MAX_HANDLERS];
static int _next_handler = 0;

static struct sigaction _action;
static struct itimerval _timer;
static unsigned int _system_ticks = 0;

int timer_signal()
{
    return TIMER_SIGNAL;
}

static void tick_handler(int signum) {
    _system_ticks++;
    
    for (int i = 0; i < _next_handler; i++) {
        if (systime() < _handlers[i].next_tick) {
            continue;
        }
        
        _handlers[i].handler(signum);
        _handlers[i].next_tick += _handlers[i].interval_ms;
    }
}

static void _register_signal()
{
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

static void _register_handler(void (*usr_tick_handler)(int), long interval_ms) {
    _handlers[_next_handler].handler = usr_tick_handler;
    _handlers[_next_handler].interval_ms = interval_ms;
    _handlers[_next_handler].next_tick = systime() + _system_ticks * interval_ms;
    _next_handler++;
}

unsigned int systime()
{
    int interval_ms = _timer.it_value.tv_usec / 1000;
    return _system_ticks * interval_ms;
}

void timer_init()
{    
    LOG_INFO("timer_init: starting timer with interval %ld ms", BASE_INTERVAL_MS);
    _register_signal();
    _set_timer(BASE_INTERVAL_MS);
}

void register_timer(void (*usr_tick_handler)(int), long interval_ms)
{
    if (usr_tick_handler == NULL)
    {
        LOG_ERR("register_timer: usr_tick_handler is NULL, exiting");
        exit(-1);
    }
    
    if (_next_handler >= MAX_HANDLERS)
    {
        LOG_ERR("register_timer: maximum number of handlers reached (%d), exiting", MAX_HANDLERS);
        exit(-1);
    }

    LOG_INFO("register_timer: registering timer with interval %ld ms", interval_ms);
    _register_handler(usr_tick_handler, interval_ms);
}
