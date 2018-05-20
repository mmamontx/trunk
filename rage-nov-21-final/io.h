#ifndef _IO_H
#define _IO_H 1

#include <pthread.h>

#define CAI_END_MODE  0x7
#define CAI_ENTER     0xa
#define CAI_END_ENTER 0x1f
#define CAI_POINT     0x2c
#define CAI_MINUS     0x2d
#define CAI_RESET     0x7f

#define CAI_RIGHT 0x19
#define CAI_LEFT  0x1a
#define CAI_UP    0x1c
#define CAI_DOWN  0x1d

#define CAI_ZERO  0x30
#define CAI_ONE   0x31
#define CAI_TWO   0x32
#define CAI_THREE 0x33
#define CAI_FOUR  0x34
#define CAI_FIVE  0x35
#define CAI_SIX   0x36
#define CAI_SEVEN 0x37
#define CAI_EIGHT 0x38
#define CAI_NINE  0x39

#define REG_A1      17
#define REG_BUF_OUT 18
#define REG_A2      19
#define REG_BUF_IN  20
#define REG_ST      26

#define X_MIN -7783215
#define X_MAX 7783215

#define Y_MIN 166574
#define Y_MAX 833426

#define H_MIN -200
#define H_MAX 3000

#define MSGCOUNT 172

struct geodata {
	int x               __attribute__ ((packed));
	int y               __attribute__ ((packed));
	short h             __attribute__ ((packed));
	unsigned short du   __attribute__ ((packed));
	short beta          __attribute__ ((packed));
	short alpha         __attribute__ ((packed));
	unsigned short cond __attribute__ ((packed));
	unsigned short crc  __attribute__ ((packed));
};

/*extern timer_t timer_id;*/
extern struct geodata stpn_data;

extern char *msg[MSGCOUNT];
extern pthread_mutex_t screen;
/*extern pthread_mutex_t stpn;*/

extern int io_devices_init();
extern int io_initmsg();
extern int io_open_code_u82();
extern int io_psk_cmd(const unsigned short);
extern int io_removesec();
extern int io_screen_clear(const int);
extern int io_send_buf(char *, const unsigned short, const int);
extern int io_send_msg(char *, const unsigned short);
extern int io_stpn_recv(unsigned short *);
extern int io_timer_adjust();
extern int io_timer_delete();
extern unsigned short io_wait_push();
extern void io_timer_handler();
extern void io_update_time();

#endif /* !_IO_H */
