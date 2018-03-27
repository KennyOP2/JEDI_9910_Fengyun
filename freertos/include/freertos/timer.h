/*
    OpenRTOS V5.4.2 - Copyright (C) Wittenstein High Integrity Systems.

    http://www.smediatech.com/
    * RISC porting from SMedia Tech. Corp. 2008
 */

#ifndef __OSTIMER_H
#define __OSTIMER_H

void            timer_except_handler(void);
void            timer_init(void);
int             timer_start(unsigned period);
void            timer_update(void);
inline unsigned timer_get_ticks(void);
unsigned        timer_get_milliseconds(void);
unsigned        timer_get_time(unsigned *sec, unsigned *ms, unsigned *us);

inline void     timer_disable_exception(void);
inline void     timer_enable_exception(void);

unsigned        timer_ms_to_xticks(unsigned ms);
unsigned        timer_xticks_to_ms(unsigned xticks);
unsigned        timer_us_to_xticks(unsigned us);
unsigned        timer_xticks_to_us(unsigned xticks);

void            timer_sleep(unsigned ticks);

#endif /* __OSTIMER_H */

