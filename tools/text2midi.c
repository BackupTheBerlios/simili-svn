/*
 *  Copyright (C) 2004 Frederic Motte
 *
 * This program read a text file and output a midi file transcription.
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
#define DEBUG
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <errno.h>
#include <getopt.h>
#include "../src/simili.h"
#include "debug.h"
//#include "timestamp.h"
#include "text2midi.h"

#define BUFF_SIZE 1024

#define T2M_TYPE_NULL   0
#define T2M_TYPE_ID     1
#define T2M_TYPE_DIGIT  2
#define T2M_TYPE_BUFFER 3


typedef struct
{
  unsigned char *data;
  int size;
}
T2M_buffer_t;

typedef union
{
  char *id;
  int digit;
  T2M_buffer_t buffer;
}
T2M_param_t;

//My beard makes up for my hair
typedef int func_new_t(T2M_t*, char*, int);
typedef int func_param_t(T2M_t*, char*, T2M_param_t*, int);
typedef int func_flush_t(T2M_t*);

typedef struct
{
  func_new_t *new;
  func_param_t *param;
  func_flush_t *flush;
}
T2M_funcs_t;

typedef struct
{
  char *alias;
  int value;
}
T2M_alias_t;

typedef struct
{
  char *name;
  int type;
  int offset;//bits
  T2M_alias_t *aliases;
}
T2M_mmsg_param_sym_t;

typedef struct
{
  char *name;
  char nb_bytes;
  char *bytes;
  T2M_funcs_t funcs[1];
  int nb_syms;
  T2M_mmsg_param_sym_t *syms;
}
T2M_mmsg_sym_t;

struct T2M_s
{
  char *text_FN;
  FILE *text;
  char *midi_FN;
  midi_t *midi;
  //MMSG
  midi_msg_t mmsg;
  T2M_funcs_t funcs;
  int nb_syms;
  T2M_mmsg_sym_t *syms;
  int nb_params;
  T2M_mmsg_param_sym_t *params;
  int track;
};

//TODO:Fill the syms tables
static int TPQN_new(T2M_t*, char*, int);
static int TPQN_param(T2M_t*, char*, T2M_param_t *, int);
//T2M_funcs_t TPQN_funcs={TPQN_new, TPQN_param, (void*)0};
//static int system_param(T2M_t*, char*, T2M_param_t*, int);

#define T2M_PARAM_TYPE_NULL   0
#define T2M_PARAM_TYPE_UI4    1
#define T2M_PARAM_TYPE_UI7    2
#define T2M_PARAM_TYPE_UI2x7  3
#define T2M_PARAM_TYPE_BUFFER 4

#define NOTE_NB_PARAM_SYMS 3
T2M_mmsg_param_sym_t note_param_syms[]={
  {"chan", T2M_PARAM_TYPE_UI4, 4,  0},
  {0,      T2M_PARAM_TYPE_UI7, 8,  0},
  {"velo", T2M_PARAM_TYPE_UI7, 16, 0}};
#define CHAT_NB_PARAM_SYMS 2
T2M_mmsg_param_sym_t channel_touch_param_syms[]={
  {"chan", T2M_PARAM_TYPE_UI4, 4,  0},
  {"velo", T2M_PARAM_TYPE_UI7, 8, 0}};
#define KEYT_NB_PARAM_SYMS 3
T2M_mmsg_param_sym_t key_touch_param_syms[]={
  {"chan", T2M_PARAM_TYPE_UI4, 4,  0},
  {0,      T2M_PARAM_TYPE_UI7, 8,  0},
  {"velo", T2M_PARAM_TYPE_UI7, 16, 0}};
#define PROG_NB_PARAM_SYMS 1
T2M_mmsg_param_sym_t prog_param_syms[]= {{0, T2M_PARAM_TYPE_UI7,   8, 0}};
#define CTRL_NB_PARAM_SYMS 1
T2M_mmsg_param_sym_t ctrl_param_syms[]= {{0, T2M_PARAM_TYPE_UI7,   8, 0}};
#define PITCH_NB_PARAM_SYMS 1
T2M_mmsg_param_sym_t pitch_param_syms[]={{0, T2M_PARAM_TYPE_UI2x7, 8, 0}};
/*T2M_alias_t system_meta_aliases[]={
  {"TRACK_NUMBER",    0x00},//
  {"TEXT",            0x01},
  {"COPYRIGHT",       0x02},
  {"TRACK_NAME",      0x03},
  {"INSTRUMENT_NAME", 0x04},
  {"LIRICS",          0x05},
  {"MARKER",          0x06},
  {"CUE_POINT",       0x07},
  {"CHANNEL",         0x20},
  {"MIDIPORT",        0x21},
  {"END_OF_TRACK",    0x2F},
  {"TEMPO",           0x51},
  {"SMPTE_OFFSET",    0x54},
  {"METRONOME",       0x58},
  {"KEY_SIGNATURE",   0x59},
  {"SEQUENCER",       0x74},
  {"PROPRIO",         0x7F}};*/
//sys: meta=tempo, data=120;
//sys: song=42;
//or:
//sys: meta, tempo=120;
/*T2M_alias_t system_param_aliases[]={
  {"system_exclusif",  0x00},
  {"song_pos_pointer", 0x02},
  {"song_select",      0x03},
  {"tune_request",     0x06},
  {"timing_clock",     0x08},
  {"continue",         0x0B},
  {"stop",             0x0C},
  {"start",            0x0A},
  {"active_sensive",   0x0E},
  {"meta",             0x0F}};

T2M_mmsg_param_sym_t system_meta_param_syms[]={
  {"meta", T2M_PARAM_TYPE_UI4, 4, system_param_aliases}};
T2M_mmsg_param_sym_t system_param_syms[]={
  {0,      T2M_PARAM_TYPE_UI4, 4, system_param_aliases},
  {"meta", T2M_PARAM_TYPE_UI7, 8, system_meta_aliases}};
//T2M_mmsg_param_sym_t system_param_syms[]={
//  {0,      T2M_PARAM_TYPE_UI7, 0, system_aliases}};
*/

#define NB_HEADER_SYMS 1
T2M_mmsg_sym_t header_syms[NB_HEADER_SYMS]={
  {"TPQN", 1, "", {(void*)0, TPQN_param, (void*)0}, 0, 0}};
#define NB_MMSG_SYMS 8
T2M_mmsg_sym_t mmsg_syms[NB_MMSG_SYMS]={
  {"note_on",        3, "\x90\x20\x7F", {0,0,0}, NOTE_NB_PARAM_SYMS, note_param_syms},
  {"note_off",       3, "\x80\x20\x7F", {0,0,0}, NOTE_NB_PARAM_SYMS, note_param_syms},
  {"key_touch",      3, "\xA0\x42\x42", {0,0,0}, KEYT_NB_PARAM_SYMS, key_touch_param_syms},
  {"controle",       3, "\xB0\x00\x00", {0,0,0}, CTRL_NB_PARAM_SYMS, ctrl_param_syms},
  {"program_change", 2, "\xC0\x42",     {0,0,0}, PROG_NB_PARAM_SYMS, prog_param_syms},
  {"pitch_bend",     3, "\xE0/x3F/xFF", {0,0,0}, PITCH_NB_PARAM_SYMS, pitch_param_syms},
  {"channel_touch",  2, "\xD0\x42",     {0,0,0}, CHAT_NB_PARAM_SYMS, channel_touch_param_syms},
  {"system",         0, "\xF0",         {0,0,0}, 0, NULL}};
//system_param
static int T2M_getopt(T2M_t *T2M, int ac, char **av);
static void usage(char*);
static int strdata2buffer(char *text, unsigned char *data_buff, int data_size);
static int T2M_process(T2M_t *T2M);

unsigned int dbg_filter;
int buff_size;
//unsigned char buff_data[BUFF_SIZE];
T2M_t *T2M;

int
main(int ac, char **av)
{
  T2M_t _T2M;

  T2M=&_T2M;
  T2M_getopt(&_T2M, ac, av);
  ck_err(!(_T2M.midi=midi_file_create(_T2M.midi_FN, MIDI_FLAG_PUT|MIDI_FLAG_CREATE)));
  T2M_process(&_T2M);
  ck_err(midi_destroy(_T2M.midi)<0);

  return 0;
 error:
  return -1;
}

//marisa@hotmail.com

static int
strvsabrev(char *str, char *abrev)
{
  while(*str&&*abrev)
    {
      if(tolower(*str)==tolower(*abrev))
	abrev++;
      str++;
    }
  if(*abrev)
    return 1;
  return 0;
}

extern int
text2timestamp(T2M_t *T2M, char *str)
{
  switch(*str)
    {
    case '@':
      ck_err(T2M->mmsg.timestamp>atoi(str+1));
      //      printf("timestamp: %d\n", atoi(str+1));
      return T2M->mmsg.timestamp=atoi(str+1);
    case '+':
      ck_err(atoi(str+1)<0);
      //      printf("timestamp: %d\n", T2M->mmsg.timestamp+atoi(str+1));
      return T2M->mmsg.timestamp+=atoi(str+1);
    default:
      break;
    }
 error:
  return -1;
}

T2M_mmsg_sym_t*
T2M_mmsg_lookup(T2M_t *T2M, char *name)
{
  T2M_mmsg_sym_t *syms;
  int i;

  if(!name)return 0;

  syms=T2M->syms;

  for(i=0; i<T2M->nb_syms; i++)
    {
      if(!strvsabrev(syms[i].name, name))
	return syms+i;
    }
  return 0;
}

T2M_mmsg_param_sym_t*
T2M_mmsg_param_lookup(T2M_t *T2M, char *name)
{
  T2M_mmsg_param_sym_t *syms;
  int i;

  syms=T2M->params;//   printf("%s\n", name);
  for(i=0; i<T2M->nb_params; i++)
    if(name && syms[i].name)
      {
	if(!strvsabrev(syms[i].name, name))
	  return syms+i;
      } else {
	if(!name && !syms[i].name)
	  return syms+i;
      }
     
  return 0;
}

extern int
T2M_mmsg_new(T2M_t *T2M, char *type, int timestamp)
{
  int i;
  T2M_mmsg_sym_t *sym;

  ck_err(!(sym=T2M_mmsg_lookup(T2M, type)));
  T2M->funcs=*sym->funcs;
  for(i=0; i<sym->nb_bytes; i++)
    T2M->mmsg.data[i] = sym->bytes[i];
  if(T2M->funcs.new)
    ck_err(T2M->funcs.new(T2M, type, timestamp)<0);

  T2M->params=sym->syms;
  T2M->nb_params=sym->nb_syms;

  return 0;
 error:
  //printf("type: %s, timestamp: %d\n", type, timestamp);
  return -1;
}

//STTT_CCCC_SNNNNNNN_SVVVVVVV

static int
T2M_param_digit(T2M_t *T2M, T2M_mmsg_param_sym_t *sym, int digit)
{
  switch(sym->type)
    {
    case T2M_PARAM_TYPE_UI4:
      //printf("T2M_PARAM_TYPE_UI4: %d\n", digit);
      ck_err(digit&!0xF);
      break;
    case T2M_PARAM_TYPE_UI7://Work only for aligned byte-bit
      //printf("T2M_PARAM_TYPE_UI7: %d\n", digit);
      ck_err(digit&!0x7F);
      ck_err(sym->offset%8);
      T2M->mmsg.data[sym->offset/8]&=0x80;
      T2M->mmsg.data[sym->offset/8]|=digit;
      break;
    case T2M_PARAM_TYPE_UI2x7:
      //printf("T2M_PARAM_TYPE_UI2x7: %d\n", digit);
      ck_err(digit&!0x3FFF);
      break;
    case T2M_PARAM_TYPE_NULL:
      //printf("T2M_PARAM_TYPE_NULL: %d\n", digit);
    }
  return 0;
 error:
  return -1;
}

static int
T2M_param_id(T2M_t *T2M, T2M_mmsg_param_sym_t *sym, char *id)
{
  return 0;
 error:
  return -1;
}

extern int
T2M_mmsg_param_id(T2M_t *T2M, char *name, char *id)
{
  T2M_mmsg_param_sym_t *sym;
  T2M_param_t param;

  if(sym=T2M_mmsg_param_lookup(T2M, name))
    ck_err(T2M_param_id(T2M, sym, id)<0);
  param.id=id;
  if(T2M->funcs.param)
    ck_err(T2M->funcs.param(T2M, name, &param, T2M_TYPE_ID)<0);

  return 0;
 error:
  return -1;
}

extern int
T2M_mmsg_param_digit(T2M_t *T2M, char *name, int digit)
{
  T2M_mmsg_param_sym_t *sym;
  T2M_param_t param;

  param.digit=digit;
  if(sym=T2M_mmsg_param_lookup(T2M, name))
    ck_err(T2M_param_digit(T2M, sym, digit)<0);
  if(T2M->funcs.param)
    ck_err(T2M->funcs.param(T2M, name, &param, T2M_TYPE_DIGIT)<0);

  return 0;
 error:
  return -1;
}

extern int
T2M_mmsg_param_datastr(T2M_t *T2M, char *name, char *str)
{
  T2M_mmsg_param_sym_t *sym;
  T2M_param_t param;
  unsigned char buff_data[BUFF_SIZE];

  param.buffer.data=buff_data;
  ck_err((param.buffer.size=strdata2buffer(str, buff_data, BUFF_SIZE))<0);
  if(sym=T2M_mmsg_param_lookup(T2M, name))/*then do nothing*/;
  //     ck_err(T2M_param_buffer(T2M, sym, &param.buffer)<0);

  if(T2M->funcs.param)
    ck_err(T2M->funcs.param(T2M, name, &param, T2M_TYPE_BUFFER)<0);

  return 0;
 error:
  return -1;
}

extern int
T2M_mmsg_flush(T2M_t *T2M)
{
  //printf("T2M_mmsg_flush: %x\n", T2M->mmsg.data[0]);
  if(T2M->mmsg.data[0])
    ck_err(midi_put_msg(T2M->midi, &T2M->mmsg)<0);
  return 0;
 error:
  return -1;
}

extern int
T2M_header_flush(T2M_t *T2M)
{
  T2M->track=0;
  T2M->syms=mmsg_syms;
  T2M->nb_syms=NB_MMSG_SYMS;
  return 0;
}

extern int
T2M_track_flush(T2M_t *T2M)
{
  T2M->mmsg.timestamp=0;
  T2M->track++;
  ck_err(midi_file_next_track(T2M->midi)<0);

  return 0;
 error:
  return -1;
}

static int
T2M_process(T2M_t *T2M)
{
  T2M->syms=header_syms;
  T2M->nb_syms=NB_HEADER_SYMS;
  T2M->mmsg.timestamp=0;
  ck_err(!(T2M->text = fopen(T2M->text_FN, "r")));
  yyrestart(T2M->text);
  yyparse();
  fclose(T2M->text);

  return 0;
 error:
  return -1;
}

static unsigned char
a2b(char c)
{
  if(c>'0'&&c<'9')
    c-='0';
  else if(c>'a'&&c<'e')
    c-='a'-10;
  else if(c>'A'&&c<'F')
    c-='A'-10;
  else
    return (unsigned char)255;
  return (unsigned char)c;
}

static int
strdata2buffer(char *text, unsigned char *data_buff, int data_size)
{
  int dbi;//Data_Buff Index
  for(dbi=0; dbi<data_size && *text; dbi++, text++)
    {
      if(*text=='%')
	{
	  text++;
	  if(*text=='%')
	    data_buff[dbi]='%';
	  else if(*text==0||*(text+1)==0)
	    break;
	  else
	    {
	      data_buff[dbi]=a2b(*text)+(a2b(*(text+1))<<4);
	      text++;
	    }
	}
    }
  return data_size;
}

static int
T2M_getopt(T2M_t *T2M, int ac, char **av)
{
  //   int digit_optind = 0;
  int i;

  T2M->text_FN="input.text";
  T2M->midi_FN="output.midi";

  while (1)
    {
      //	int this_option_optind = optind ? optind : 1;
      int option_index = 0;
      struct option long_options[] =
	{
	  {"help", 1, 0, 'h'},
	  {"debug", 1, 0, 'd'},
	  {0, 0, 0, 0}
	};

      i = getopt_long (ac, av, "hd:", long_options, &option_index);
      if (i == -1)
	break;//leave the loop

      switch (i)
	{
	case 'h':
	  usage (av[0]);
	  break;

	case 'd':
	  while(*optarg)
	    switch(*(optarg++))
	      {
	      case 'P': dbg_filter|=DBG_PROC; break;
	      case 'M': dbg_filter|=DBG_MIDI; break;
	      case 'B': dbg_filter|=DBG_BANK; break;
	      case 'F': dbg_filter|=DBG_PARSE; break;
	      case 'I': dbg_filter|=DBG_INSTRU; break;
	      }
	  break;

/* 	case '0': */
/* 	  fprintf (stderr, "option %s", long_options[option_index].name); */
/* 	  if (optarg) fprintf (stderr, " with arg %s", optarg); */
/* 	  fprintf (stderr, "\n"); */
/* 	  break; */
/* 	default: */
/* 	  fprintf (stderr, "?? getopt returned character code 0%o ??\n", i); */
	}
    }

  switch(ac-optind)
    {
    case 2:
      T2M->midi_FN=strdup(av[optind+1]);
    case 1:
      T2M->text_FN=strdup(av[optind]);
    case 0:
      break;
    default:
      goto err_usage;
    }
  errno = 0;

  printf("text file name: %s\n", T2M->text_FN);
  printf("midi file name: %s\n", T2M->midi_FN);

  return 0;
 err_usage:
  usage(av[0]);
  return -1;
}

static void usage (char *name)
{
  printf ("usage:%s [options] <input text file> <output midi file>\n", name);
  puts ("");
  puts (" options:");
  puts ("   -h, --help  :???");
  puts ("   -d, --debug :<PMBFI>");
  puts ("");
  exit (0);
}

static int
TPQN_new(T2M_t *T2M, char *name, int timestamp)
{
  //printf("TPQN_new: %s, %d\n", name, timestamp);
  return 0;
}

static int
TPQN_param(T2M_t *T2M, char *nada, T2M_param_t *param, int type)
{
  if(!nada&&type==T2M_TYPE_DIGIT)
    {
      //puts("SET TPQN !!!");
      return midi_file_set_tpqn(T2M->midi, param->digit);
    }
  else
    return -1;
}

/*USELESS
static int
NB_track_new(T2M_t *T2M, char *name, int timestamp)
{
  printf("NB_track_new: %s, %d\n", name, timestamp);
  return 0;
}
static int
NB_track_param(T2M_t *T2M, char *nada, T2M_param_t *param, int type)
{
  if(!nada&&type==T2M_TYPE_DIGIT)
    {
      puts("SET NB_track !!!");
    }
  else
    return -1;
 }
*/
