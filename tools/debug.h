/*
 *  Copyright (C) 2001 Frederic Motte
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef  _SISY_DEBUG_H
#define  _SISY_DEBUG_H

#include <stdio.h>
#include <errno.h>
#include "misc.h"

enum {
   DBG_PROC=1<<0,
   DBG_MIDI=1<<1,
   DBG_BANK=1<<2,
   DBG_PARSE=1<<3,
   DBG_INSTRU=1<<4
};

extern unsigned int dbg_filter;

#if 1
#define dbg(filter, msg) \
     do { \
	if(dbg_filter&(filter)==(filter))\
           printf("DEBUG in %s(l.%d) : %s\n", __FILE__, __LINE__, msg); \
     } while(0);
#else
#define dbg(filter, msg)
#endif

#if 0
#define FAILED_IF(_expr) \
  do { \
    if(_expr){\
      printf("ERROR at %s(l.%d) : %s !!!\n", __FILE__, __LINE__, #_expr); \
      goto error; \
    } \
  } while(0)

#define TEST_FILE(file, ret)  if(access((file),R_OK)){\
                                perror(path);\
                                return (ret);}
#endif
#endif
