/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 c-file-style: "gnu" -*- */
/* The midi_buffer_t is time based on the global timebase */
#ifndef _LIBMIDI_MIDI_PIPE_H
#define _LIBMIDI_MIDI_PIPE_H


#include "midi.h"
#include "list.h"


typedef struct midi_pipe_s midi_pipe_t;


struct midi_pipe_s
{
  midi_flags_t flags;
  list_t msgs;
  int timestamp_put;
  int timestamp_get;
};


int midi_pipe_open (midi_pipe_t * pipe, char *pipe_name,
                    midi_flags_t flags);
int midi_pipe_get_msg (midi_pipe_t * pipe, midi_msg_t * msg);
int midi_pipe_put_msg (midi_pipe_t * pipe, midi_msg_t * msg, int ticks);
int midi_pipe_close (midi_pipe_t * pipe);

int midi_pipe_get_data (midi_pipe_t * pipe, unsigned char *data,
                        int size);
int midi_pipe_put_data (midi_pipe_t * pipe, unsigned char *data,
                        int size);


#endif
