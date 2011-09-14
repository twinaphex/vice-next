#ifndef __NETDBG_H_
#define __NETDBG_H_

#include <stdarg.h>
#include <netinet/in.h>

void net_init();
void net_shutdown();

//void net_send(char *text);
void net_send(int sleepms, const char *text,...);

#endif
