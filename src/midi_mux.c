#include "midi_mux.h"
#include "midi.h"
#include "misc.h"


//DEFINITIONS
typedef struct midi_mux_s midi_mux_t;
typedef struct midi_mux_class_s midi_mux_c;
typedef struct voice_s voice_t;
typedef int timestamp_t;


static int destroy (midi_mux_t*);
static int get_msg (midi_mux_t*, midi_msg_t*);
static int put_msg (midi_mux_t*, midi_msg_t*);
static int get_data (midi_mux_t*, unsigned char*, int);
static int put_data (midi_mux_t*, unsigned char*, int);


struct midi_mux_class_s
{
  midi_c midi;
};


obj_c*
midi_mux_class()
{
  static obj_c *class=0;

  if(!class)
    {
      create_class(midi_mux, midi);
      class->destroy = (obj_destroy_t*)destroy;
      MIDI_CLASS(class)->put_msg = (midi_put_msg_t*)put_msg;
      MIDI_CLASS(class)->get_msg = (midi_get_msg_t*)get_msg;
      MIDI_CLASS(class)->put_data = (midi_put_data_t*)put_data;
      MIDI_CLASS(class)->get_data = (midi_get_data_t*)get_data;
    }

  return class;
}


struct midi_mux_s
{
  midi_t midi;
  voice_t *voices;
  int nb_voices;
  u16_t note_old;
  u16_t nada_old;
  u8_t note_on[8];//bit field
  int timestamp_put;
  int timestamp_get;
};


struct voice_s{
  timestamp_t timestamp;
  u8_t note;
  midi_t *pipe;
  int off;
};


midi_t*
midi_mux_create (int nb_voices, midi_flags_t flags)
{
  midi_mux_t * mux=0;

  mux = obj_create(midi_mux);
  MIDI(mux)->flags = flags;

  mux->nb_voices = nb_voices;
  if(nb_voices)
    mux->voices = Xalloc(voice_t, mux->nb_voices);
  return MIDI(mux);
}


midi_t*
midi_mux_get_voice(midi_t * midi, int voice_num, midi_flags_t flags)
{
  midi_mux_t * mux = MIDI_MUX(midi);
  ck_err(!mux || !mux->voices || voice_num < 0 || voice_num > mux->nb_voices);

  if(!mux->voices[voice_num].pipe)
    ck_err(!(mux->voices[voice_num].pipe = midi_pipe_create(flags | MIDI_FLAG_PUT | MIDI_FLAG_GET)));
  mux->voices[voice_num].off = -1;
  mux->voices[voice_num].timestamp = -1;
  return mux->voices[voice_num].pipe;
 error:
  return 0;
}


static int
voice_put_msg(voice_t * voice, midi_msg_t * msg)
{
  ck_err(!voice->pipe);
  ck_err(midi_put_msg(voice->pipe, msg) < 0);
  voice->note = MIDI_MSG_NOTE(msg);
  voice->timestamp = msg->timestamp;
  if(MIDI_MSG_TYPE(msg)==MIDI_NOTE_OFF)
    voice->off=1;
  else
    voice->off=0;
  return 0;
 error:
  return -1;
}


/* midi_mux assume that is the same channel */
static int
put_msg (midi_mux_t * mux, midi_msg_t * msg)
{
  int v, last_voice_on = -1, last_voice_off = -1,
    last_voice_on_timestamp = msg->timestamp,
    last_voice_off_timestamp = msg->timestamp;

  if(!mux->nb_voices)
    return 0;

  /* dispatch midi notes */
  switch(MIDI_MSG_TYPE(msg))
    {
    case MIDI_NOTE_ON:
    case MIDI_NOTE_OFF:
    case MIDI_KEY_TOUCH:
      for(v=0; v<mux->nb_voices; v++)
	{
	  if(mux->voices[v].note == MIDI_MSG_NOTE(msg))
	    {
	      ck_err(voice_put_msg(&mux->voices[v], msg) < 0);
	      return 0;
	    }

	  if(mux->voices[v].off &&
 	     mux->voices[v].timestamp < last_voice_off_timestamp)
	    {
	      last_voice_off_timestamp = mux->voices[v].timestamp;
	      last_voice_off = v;
	    }
	  
	  if(mux->voices[v].timestamp < last_voice_on_timestamp)
	    {
	      last_voice_on_timestamp = mux->voices[v].timestamp;
	      last_voice_on = v;
	    }
	}

      /* note off go to voice where note == msg->note */
      if(MIDI_MSG_TYPE(msg) == MIDI_NOTE_OFF)
	break;

      if(last_voice_off >= 0) v = last_voice_off;
      else if(last_voice_on >= 0) v = last_voice_on;

      ck_err(v >= mux->nb_voices);
      ck_err(voice_put_msg(&mux->voices[v], msg) < 0);

      break;
    default:
      for(v=0; v<mux->nb_voices; v++)
	{
	  midi_put_msg(mux->voices[v].pipe, msg);
	}      
    }

  return 0;
 error:
  return -1;
}


//msg
static int
get_msg (midi_mux_t *mux, midi_msg_t *msg)
{
  return -1;
}


static int
destroy (midi_mux_t * mux)
{
  int v;
  for(v=0; v<mux->nb_voices; v++)
    {
      midi_destroy(mux->voices[v].pipe);
    }

  free(mux);

  return 0;
/*  error: */
/*   return -1; */
}


static int
get_data (midi_mux_t * mux, unsigned char *data, int size)
{
  return -1;
}


static int
put_data (midi_mux_t * mux, unsigned char *data, int size)
{
  return -1;
}
