#ifndef _LIBMIDI_MISC_H
#define _LIBMIDI_MISC_H


#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>


typedef unsigned char u8_t;	/* must be 1 octet  */
typedef unsigned short u16_t;	/* must be 2 octet  */
typedef unsigned long u32_t;	/* must be 4 octets */


#define COUT stderr


#ifdef DEBUG
#define debug(str) \
  do \
    { \
      fprintf(COUT, "*** %s, l.%d: %s\n", __FILE__, __LINE__, str); \
    } \
  while(0)
#else
#define debug(str)
#endif


#ifdef DEBUG
#define error(str) \
  do \
    { \
      fflush(stdout); \
      fprintf(COUT, "ERROR:%s,\tl.%-3d: %s: %s\n", __FILE__, __LINE__, str, strerror(errno)); \
    } \
  while(0)
#else
#define error(str)
#endif


#ifdef DEBUG
#define ck_err(exp) \
  do \
    { \
      int _O=(int)(exp); \
      if(_O) \
        { \
          error(#exp); \
          goto error; \
        } \
    } \
  while(0)
#else
#define ck_err(exp) \
  do \
    { \
      int _O=(int)(exp); \
      if(_O) \
        { \
          goto error; \
        } \
    } \
  while(0)
#endif


#define TVGetDelay(tv) ((tv.sec*1000)+tv.usec/1000)

#define TVSetDelay(tv, delay) \
  do \
    { \
      tv.sec  = delay/1000; \
      tv.usec = (delay%1000)*1000; \
    } \
  while(0)


#define Free(mem) (free(mem))
#define Calloc(size, nbr) (calloc(size, nbr))
#define Malloc(size) (calloc(size, 1))
#define Talloc(type) ((type*)Calloc(sizeof(type), 1))
#define Xalloc(type, nbr) ((type*)Calloc(sizeof(type), nbr))


#define Min(a,b) ((a) < (b) ? (a) : (b))
#define Max(a,b) ((a) > (b) ? (a) : (b))


inline static void bit_set(u8_t *field, u16_t bit)
{
  field[bit>>3] |= ((1<<bit)&0xFF);
}

inline static void bit_zero(u8_t *field, u16_t bit)
{
  field[bit>>3] &= ~((1<<bit)&0xFF);
}

inline static u16_t bit_get(u8_t *field, u16_t bit)
{
  return (field[bit>>3] & ((1<<bit)&0xFF));
}

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#endif
