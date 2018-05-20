#ifndef _ABUS_H
#define _ABUS_H 1

#define ABUS_DELAY 100000

#define ABUS_MSG_CNTR_COM  0xc0c0
#define ABUS_MSG_STATE     0xdcdc
#define ABUS_MSG_STATE_NEW 0xdccd
#define ABUS_MSG_WAIT      0xf1f1

/*extern char abus;*/

extern int abus_poll();
extern int abus_send_msg(const unsigned char, const unsigned short, const char *, const unsigned char);
extern unsigned char abus_get_state();
extern unsigned short abus_csum(const unsigned short *, const unsigned short);
extern void abus_set_state(const unsigned char);

#endif /* !_ABUS_H */