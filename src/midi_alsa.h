#ifndef _SIMILI_ALSA_H
#define _SIMILI_ALSA_H


#include "simili.h"
#include "obj.h"


obj_c *midi_alsa_class();


#define MIDI_ALSA_CLASS(class) ((midi_c*)(class))
#define IS_MIDI_ALSA(obj)      ((midi_alsa_t*)check_ancestor(OBJ(obj), OBJ_CLASS(midi_alsa_class()), 0, 0))
#ifdef DEBUG
#define MIDI_ALSA(obj)        ((midi_alsa_t*)check_ancestor(OBJ(obj), OBJ_CLASS(midi_alsa_class()),      __FILE__, __LINE__))
#else
#define MIDI_ALSA(obj)        ((midi_alsa_t*)OBJ(obj))
#endif


#endif
