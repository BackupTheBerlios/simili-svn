#ifndef _SIMILI_PORT_H
#define _SIMILI_PORT_H


#include "simili.h"


typedef struct midi_port_s midi_port_t;


struct midi_port_s
{
  midi_flags_t flags;
  int fd;
  unsigned char status;
  midi_msg_t msg;
  int ppqn;			//Pulse Per Quarter of Note
};


int midi_port_open (midi_port_t * port, char *file_name, midi_flags_t flags);
int midi_port_get_msg (midi_port_t * port, midi_msg_t * msg);
int midi_port_put_msg (midi_port_t * port, midi_msg_t * msg, int ticks);
int midi_port_get_sysex (midi_port_t * port, void *buff, int *size);
int midi_port_close (midi_port_t * port);


#endif
