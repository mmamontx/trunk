#ifndef _SYSTEM_H
#define _SYSTEM_H 1

#define SYSTEM_DELAY    50000
#define SYSTEM_MAXDELAY 10000000

extern int sys_state;

extern int system_check(const int);
extern int system_control(const unsigned short *buf);
extern int system_refresh();
extern void *system_poll(void *);

#endif /* !_SYSTEM_H */