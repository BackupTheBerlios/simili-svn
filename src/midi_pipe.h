#ifndef _LIBMIDI_MIDI_PIPE_H
#define _LIBMIDI_MIDI_PIPE_H


#include "obj.h"


obj_c *midi_pipe_class();


#define MIDI_PIPE_CLASS(class) ((midi_c*)(class))
#define IS_MIDI_PIPE(obj)      ((midi_pipe_t*)check_ancestor(OBJ(obj), OBJ_CLASS(midi_pipe_class()), 0, 0))
#ifdef DEBUG
#define MIDI_PIPE(obj)        ((midi_pipe_t*)check_ancestor(OBJ(obj), OBJ_CLASS(midi_pipe_class()),      __FILE__, __LINE__))
#else
#define MIDI_PIPE(obj)        ((midi_pipe_t*)OBJ(obj))
#endif


#endif
