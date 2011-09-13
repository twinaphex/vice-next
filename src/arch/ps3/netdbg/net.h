
#include <stdarg.h>

void net_init();
void net_shutdown();

//void net_send(char *text);
void net_send(int sleepms, const char *text,...);
