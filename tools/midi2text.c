/*
 *  Copyright (C) 2004 Frederic Motte
 *
 * This program read a midi file a stdout a text file transcription readable by
 * midi_file_write to create a midi file
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
#include "../src/simili.h"
//#include <errno.h>
#include <getopt.h>
#include "debug.h"
//#include "midi.h"
//#include "timestamp.h"

#define BUFF_SIZE 1024

unsigned int dbg_filter;

typedef struct
{
  FILE *text;
  char *text_FN;
  midi_t *midi;
  char *midi_FN;
  char chan;
  char velo;
}
M2T_t;

void usage (char *);
int M2T_getopt(M2T_t *M2T, int ac, char **av);
int M2T_process(M2T_t*);
int M2T_data2text(M2T_t*, unsigned char*, int);
int M2T_mmsg2text(M2T_t*, midi_msg_t*);

int log_level = 0;
int midi_file_track=-1;

int
main (int ac, char **av)
{
  M2T_t M2T;
  M2T_getopt(&M2T, ac, av);
  M2T_process(&M2T);

  return 0;
 error:
  return -1;
}

int
M2T_mmsg2text(M2T_t *M2T, midi_msg_t *mmsg)
{
  fprintf(M2T->text, "@%d:", mmsg->timestamp);
  switch(MIDI_MSG_TYPE(mmsg))
    {
      //System level
    case MIDI_SYSTEM:
      fprintf(M2T->text, "System:");
      if(dbg_filter&DBG_MIDI)
	printf("M2T_midi_process: system: %x: ", MIDI_MSG_STATUS(mmsg));
      switch(MIDI_MSG_STATUS(mmsg))
	{
	case MIDI_SYS_META:
	  fprintf(M2T->text, "META, ");
	  if(dbg_filter&DBG_MIDI)
	    printf("system meta: %x: ", MIDI_MSG_DATA1(mmsg));
	  switch(MIDI_MSG_DATA1(mmsg))
	    {
	    case MIDI_SYS_META_TEMPO:
	      fprintf(M2T->text, "Tempo=%d;\n", MIDI_MSG_TEMPO(mmsg));
	      break;
	    case MIDI_SYS_META_KEY:
	      fprintf(M2T->text, "Key=%d;\n", MIDI_MSG_DATA2(mmsg));
	      break;
	    case MIDI_SYS_META_CHANNEL:
	      fprintf(M2T->text, "Channel=%d;\n", M2T->chan=MIDI_MSG_DATA2(mmsg));
	      break;
	    case MIDI_SYS_META_MIDIPORT:
	      fprintf(M2T->text, "Midiport=%d;\n", MIDI_MSG_DATA2(mmsg));
	      break;
	    case MIDI_SYS_META_METRONOME:
	      fprintf(M2T->text, "Metronome=%d;\n", MIDI_MSG_DATA2(mmsg));
	      break;
	    default:
	      fprintf(M2T->text, "Unknown_meta=%x;\n", MIDI_MSG_DATA1(mmsg));
	    }
	  break;
	default:
	  fprintf(M2T->text, "Unknown_system: %x;\n", MIDI_MSG_STATUS(mmsg));
	  break;
	}
      break;
    case MIDI_NOTE_ON:
      fprintf(M2T->text, "Note_on: %d", MIDI_MSG_NOTE(mmsg));
      if(MIDI_MSG_CHAN(mmsg) != M2T->chan)
	{
	  fprintf(M2T->text, ", chan=%d", MIDI_MSG_CHAN(mmsg));
	  if(M2T->chan==-1)
	    M2T->chan = MIDI_MSG_CHAN(mmsg);
	}
      if(MIDI_MSG_VELO(mmsg) != M2T->velo)
	fprintf(M2T->text, ", velo=%d", M2T->velo = MIDI_MSG_VELO(mmsg));
      fprintf(M2T->text, ";\n");
      break;
    case MIDI_NOTE_OFF:
      fprintf(M2T->text, "Note_off: %d", MIDI_MSG_NOTE(mmsg));
      if(MIDI_MSG_CHAN(mmsg) != M2T->chan)
	{
	  fprintf(M2T->text, ", chan=%d", MIDI_MSG_CHAN(mmsg));
	  if(M2T->chan==-1)
	    M2T->chan = MIDI_MSG_CHAN(mmsg);
	}
      if(MIDI_MSG_VELO(mmsg) && MIDI_MSG_VELO(mmsg) != M2T->velo)
	fprintf(M2T->text, ", velo=%d", M2T->velo = MIDI_MSG_VELO(mmsg));
      fprintf(M2T->text, ";\n");
      break;
    case MIDI_PROG_CHNG:
      fprintf(M2T->text, "Program_change;\n");
      break;
    default:
      if(dbg_filter&DBG_MIDI)
	printf("M2T_midi_process: default msg: %x: ", MIDI_MSG_TYPE(mmsg));
      fprintf(M2T->text, "NOP;\n");
    }
  return 0;
 error:
  return -1;
}

int
M2T_process(M2T_t *M2T)
{
  int nb_tracks=0, t, size, TPQN;
  char buff[BUFF_SIZE];
  midi_msg_t mmsg;

  dbg(DBG_MIDI, "M2T_mmsg2text\n");

  ck_err (!(M2T->midi = midi_file_create (M2T->midi_FN, MIDI_FLAG_GET)));
  if(M2T->text_FN)
    ck_err (!(M2T->text = fopen(M2T->text_FN, "w+")));
  else
    M2T->text=stdout;
  fprintf(M2T->text, "[HEADER]\n");

  ck_err ((nb_tracks = midi_file_get_nb_tracks(M2T->midi)) < 0);
  TPQN = midi_file_get_tpqn(M2T->midi);
  fprintf(M2T->text, "TPQN: %d;\n", TPQN);

  //FIXME: if file
  for(t=0; t<nb_tracks; t++)
    {
      fprintf(M2T->text, "\n[TRACK]\n", t);
      M2T->chan=-1;

      while(midi_get_msg (M2T->midi, &mmsg)>=0)
	{
	  dbg(DBG_MIDI, "MMSG\n");
	  if(MIDI_MSG_EOF (&mmsg))
	    goto eof;
	  if(MIDI_MSG_EOT (&mmsg))
	    break;
	  if(MIDI_MSG_NULL (&mmsg))
	    break;

	  M2T_mmsg2text(M2T, &mmsg);
	  if (MIDI_MSG_DATA (&mmsg))
	    {
	      ck_err ((size=midi_get_data (M2T->midi, buff, BUFF_SIZE-1)) < 0);
	      buff[size]=0;
	      M2T_data2text(M2T, buff, size);
	      if(dbg_filter&DBG_MIDI)
		printf("midi data(%d): %s\n", size, buff);
	    }
	}
    }
 eof:
  fclose(M2T->text);
  return 0;
 error:
  return -1;
}

int
M2T_data2text(M2T_t *M2T, unsigned char *data, int size)
{
  int i;
  fprintf(M2T->text, "Data: \"");
  for(i=0; i<size; i++)
    {
      if(isprint(data[i]))
	fprintf(M2T->text, "%c", data[i]);
      else
	fprintf(M2T->text, "\%%02x", data[i]);
    }

  fprintf(M2T->text, "\";\n");
  return 0;
}

int
M2T_getopt(M2T_t *M2T, int ac, char **av)
{
  int i;

  M2T->midi_FN="input.midi";
  M2T->text_FN=0;

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
	break;

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
      M2T->text_FN=strdup(av[optind+1]);
    case 1:
      M2T->midi_FN=strdup(av[optind]);
    case 0:
      break;
    default:
      goto err_usage;
    }
  errno = 0;

  return 0;
 err_usage:
  usage(av[0]);
  return -1;
}

void usage (char *name)
{
  printf ("usage:%s [options] <input midi file> <output text file>\n", name);
  puts ("");
  puts (" options:");
  puts ("   -h, --help  :???");
  puts ("   -d, --debug :<PMBFI>");
  puts ("");
  exit (0);
}
