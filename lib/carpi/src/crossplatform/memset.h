#ifndef MEMSET_H
#define MEMSET_H

#include <stddef.h>

void *
memset (void *dest, int val, size_t len)
{
  unsigned char *ptr = (unsigned char*)dest;
  while (len-- > 0)
    *ptr++ = val;
  return dest;
}

#endif