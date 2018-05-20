#ifndef _MKIO_H
#define _MKIO_H 1

#include <pthread.h>

#define ABUS 8
#define BE   10
#define BKD  11
#define CAI  23
#define STAN 29

#define R1  1
#define R2  2
#define R3  3
#define R5  5
#define R8  8
#define R11 15

#define GEO_DATA 3

#define KK_CHECK_READY 0x01
#define KK_POST_READ   0x11
#define KK_POST_WRITE  0x12
#define KK_SET_MODE    0x21

#define MANCHESTER_DEVICE "/dev/manchester0"

/*extern int fd;*/
extern pthread_mutex_t mkio;

extern int mkio_destroy();
extern int mkio_init();
extern int mkio_recv(const unsigned int, const unsigned int, const unsigned int, unsigned short *);
extern int mkio_send(const unsigned int, const unsigned int, const unsigned int, const unsigned short *);

#endif /* !_MKIO_H */