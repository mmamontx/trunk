#ifndef _LSTACK_H
#define _LSTACK_H 1

/*extern unsigned char lsize;
extern unsigned char *lhead;*/

extern int lstack_push(const unsigned char);
extern int lstack_release();
extern unsigned char lstack_pop();
extern void lstack_destroy();

#endif /* !_LSTACK_H */