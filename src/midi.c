#include "midi.h"
#include "misc.h"
#include "obj.h"


int current_tempo = 120;
int current_ppqn = 480;
int current_timestamp = 0;


int
get_timestamp()
{
  return 0;
}


midi_t*
midi_open(char *url)
{
  return 0;
}


int
dummy_int()
{
  return 0;
}


obj_c*
midi_class()
{
  static obj_c *class=0;

  if(!class)
    {
      create_class(midi, obj);
      MIDI_CLASS(class)->put_msg = (midi_put_msg_t*)dummy_int;
      MIDI_CLASS(class)->get_msg = (midi_get_msg_t*)dummy_int;
      MIDI_CLASS(class)->put_data = (midi_put_data_t*)dummy_int;
      MIDI_CLASS(class)->get_data = (midi_get_data_t*)dummy_int;
      MIDI_CLASS(class)->timestamp = (midi_timestamp_t*)dummy_int;
    }

  return class;
}

/** name is "type:path" where type can be alsa, file or rtp */
midi_t*
midi_create(char const * url, midi_flags_t flags)
{
  midi_t *midi=0;
  /*   "alsa:64:0&129"; */
  /*   "file:jazz.mid"; */

#ifdef ALSA
  if(!strncmp(url, "alsa:", 5))
      return midi_alsa_create(url+5, flags);
#endif
  if(!strncmp(url, "file:", 5))
      return midi_file_create(url+5, flags);
  if(!strncmp(url, "rtp:", 4))
      printf("rtp\n");
  return midi_file_create(url, flags);
}

void
midi_msg_print (midi_msg_t * msg)
{
  //  int i;

  switch (MIDI_MSG_TYPE (msg))
    {
    case MIDI_KEY_TOUCH:
      printf ("MIDI_KEY_TOUCH:canal:%2x,note:%2x,value:%2x\n",
	      MIDI_MSG_CHAN (msg), MIDI_MSG_NOTE (msg), MIDI_MSG_DATA2 (msg));
      break;
    case MIDI_CONTROL:
      printf ("MIDI_CONTROL  :canal:%2x,ctrl:%2x,value:%2x\n",
	      MIDI_MSG_CHAN (msg), MIDI_MSG_CTRL (msg), MIDI_MSG_VALUE (msg));
      break;
    case MIDI_PROG_CHNG:
      printf ("MIDI_PRGM_CHNG:canal:%2x,prgm:%2x\n",
	      MIDI_MSG_CHAN (msg), MIDI_MSG_PRGM (msg));
      break;
    case MIDI_CHAN_TOUCH:
      printf ("MIDI_KEY_TOUCH:canal:%2x,value:%2x\n",
	      MIDI_MSG_CHAN (msg), MIDI_MSG_DATA1 (msg));
      break;
    case MIDI_PITCHBEND:
      printf ("MIDI_PITCHBEND:canal:%2x,value:%2x\n",
	      MIDI_MSG_CHAN (msg), MIDI_MSG_PITCH (msg));
      break;
    case MIDI_NOTE_OFF:
      printf ("MIDI_NOTE_OFF :canal:%2x,note:%2x,velo:%2x\n",
	      MIDI_MSG_CHAN (msg), MIDI_MSG_NOTE (msg), MIDI_MSG_VELO (msg));
      break;
    case MIDI_NOTE_ON:
      printf ("MIDI_NOTE_ON  :canal:%2x,note:%2x,velo:%2x\n",
	      MIDI_MSG_CHAN (msg), MIDI_MSG_NOTE (msg), MIDI_MSG_VELO (msg));
      break;
    }
}


int
midi_note_period(int note, int fe)
{
   return 0;
}
