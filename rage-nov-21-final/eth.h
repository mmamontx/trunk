#ifndef _ETH_H
#define _ETH_H 1

#define POLL_COUNT    3
#define POLL_DELAY    50000
#define POLL_MAXDELAY 500000

/*extern char flags[6];*/

extern int collect(const unsigned char);
extern int solve(const char *, const unsigned short, int, const struct SIM_UserData *);
extern void *eth_apk(void *);
extern void *eth_ksau(void *);
extern void *eth_oep(void *);
extern void *eth_rmi(void *);
extern void *eth_sugp(void *);

#endif /* !_ETH_H */