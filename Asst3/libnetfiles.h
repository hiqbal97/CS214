#include <stddef.h>
#ifndef __LIBNETFILES_H__
#define __LIBNETFILES_H__

int netserverinit(char *name, int filemode);
int netopen(const char *name, int y);
int netread(int destination, void *ptr, size_t y);
int netwrite(int destination, void *ptr, size_t y);
int netclose(int y);

#endif // __LIBNETFILES_H__