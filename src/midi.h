#ifndef _SIMILI_MIDI_H
#define _SIMILI_MIDI_H


#include "simili.h"


extern int current_tempo;
extern int current_ppqn;
extern int current_timestamp;


obj_c *midi_class();



#define MIDI_CLASS(class) ((midi_c*)(class))
#define IS_MIDI(obj)      ((midi_t*)check_ancestor(OBJ(obj), OBJ_CLASS(midi_class()), 0, 0))

#ifdef DEBUG
#define MIDI(obj)        ((midi_t*)check_ancestor(OBJ(obj), OBJ_CLASS(midi_class()),      __FILE__, __LINE__))
#else
#define MIDI(obj)        ((midi_t*)OBJ(obj))
#endif


#endif
