#ifndef __TIMER_H__
#define __TIMER_H__

/**
 * @brief Get the timer signal
 * @return The timer signal
 */
int timer_signal();

/*
 * @brief Initialize the timer
 * @param timer_tick_handler The handler to be called when the timer ticks
 * @param interval_ms The interval in milliseconds
 * @return void
 */
int timer_init(void (*timer_tick_handler)(int), long interval_ms);

#endif