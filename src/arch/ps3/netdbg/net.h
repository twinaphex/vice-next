#ifndef __NETDBG_H_
#define __NETDBG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <netinet/in.h>

void net_init();
void net_shutdown();

//void net_send(char *text);
void net_send(int sleepms, const char *text,...);

#ifdef __cplusplus
}
#endif

#endif
