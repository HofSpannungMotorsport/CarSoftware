#ifndef MEMCPY_H
#define MEMCPY_H

#include <stddef.h>

void *
memcpy (void *dest, const void *src, size_t len)
{
  char *d = (char*)dest;
  const char *s = (const char*)src;
  while (len--)
    *d++ = *s++;
  return dest;
}

#endif // MEMCPY_H