#ifndef _LIBMIDI_MIDI_FILE_H
#define _LIBMIDI_MIDI_FILE_H


#include "obj.h"


obj_c *midi_file_class();


#define MIDI_FILE_CLASS(class) ((midi_c*)(class))
#define IS_MIDI_FILE(obj)      ((midi_file_t*)check_ancestor(OBJ(obj), OBJ_CLASS(midi_file_class()), 0, 0))
#ifdef DEBUG
#define MIDI_FILE(obj)        ((midi_file_t*)check_ancestor(OBJ(obj), OBJ_CLASS(midi_file_class()),      __FILE__, __LINE__))
#else
#define MIDI_FILE(obj)        ((midi_file_t*)OBJ(obj))
#endif


#endif
