#ifndef _LIBMIDI_MIDI_MUX_H
#define _LIBMIDI_MIDI_MUX_H


#include "obj.h"


obj_c *midi_mux_class();


#define MIDI_MUX_CLASS(class) ((midi_c*)(class))
#define IS_MIDI_MUX(obj)      ((midi_mux_t*)check_ancestor(OBJ(obj), OBJ_CLASS(midi_mux_class()), 0, 0))
#ifdef DEBUG
#define MIDI_MUX(obj)        ((midi_mux_t*)check_ancestor(OBJ(obj), OBJ_CLASS(midi_mux_class()),      __FILE__, __LINE__))
#else
#define MIDI_MUX(obj)        ((midi_mux_t*)OBJ(obj))
#endif


#endif
