#ifdef ALSA
#include <alsa/asoundlib.h>
#include "midi_alsa.h"
#include "midi.h"
#include "misc.h"
/* as pipe, msg are removed when get it */
/* alsa://64:0 */ 

//Probe: midi_probe_IOs();

//DEFINITIONS
typedef struct midi_alsa_s midi_alsa_t;
typedef struct midi_alsa_class_s midi_alsa_c;


static int destroy (midi_alsa_t*);
static int get_msg (midi_alsa_t*, midi_msg_t*);
static int put_msg (midi_alsa_t*, midi_msg_t*);
static int get_data (midi_alsa_t*, unsigned char*, int);
static int put_data (midi_alsa_t*, unsigned char*, int);


struct midi_alsa_class_s
{
    midi_c midi;
};


obj_c*
midi_alsa_class()
{
    static obj_c *class=0;

    if(!class)
	{
	    create_class(midi_alsa, midi);
	    class->destroy = (obj_destroy_t*)destroy;
	    MIDI_CLASS(class)->put_msg = (midi_put_msg_t*)put_msg;
	    MIDI_CLASS(class)->get_msg = (midi_get_msg_t*)get_msg;
	    MIDI_CLASS(class)->put_data = (midi_put_data_t*)put_data;
	    MIDI_CLASS(class)->get_data = (midi_get_data_t*)get_data;
	}

    return class;
}


struct midi_alsa_s
{
    midi_t midi;
    int portid;
    int timestamp;
};


//STATIC DECLARATIONS
static int midi_alsa_open_local(u32_t seq_flags);
static int n_instances=0;

//IMPLEMENTATION
snd_seq_t *alsa_seq=0;

/** name is the name of the port, all the ports are open under the alsa client name "LIBFMS" or the registered name (through the not yet writen midi_register_name() function)*/
/*init the static & local sequencer if needed and open a port */
midi_t*
midi_alsa_create (char const *name, midi_flags_t flags)
{
    midi_alsa_t * alsa=0;
    u32_t port_flags=0;

    alsa = obj_create(midi_alsa);
    MIDI(alsa)->flags = flags;

    ck_err(midi_alsa_open_local(SND_SEQ_NONBLOCK) < 0);

    if(flags & MIDI_FLAG_PUT)
	{
	    port_flags = SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ;
	    ck_err ((snd_seq_create_simple_port(alsa_seq, name, port_flags, SND_SEQ_PORT_TYPE_APPLICATION))<0);
	}

    if(flags & MIDI_FLAG_GET)
	{
	    port_flags = SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE;
	    ck_err (snd_seq_create_simple_port(alsa_seq, name, port_flags, SND_SEQ_PORT_TYPE_APPLICATION)<0);
	}

    return MIDI(alsa);
  error:
    return 0;
}

/* int */
/* midi_alsa_list_clients() */
/* { */
/* } */


/* int */
/* midi_alsa_plug(midi_alsa_t *alsa, char *receiver) */
/* { */
/*   error: */
/*     return 0; */
/* } */

/* Open THE sequencer
 * FIXME:open in Input or output only if necessary */
static int
midi_alsa_open_local(u32_t seq_flags)
{
    if(n_instances++ && alsa_seq)
	return 0;

    ck_err (snd_seq_open(&alsa_seq, "default", SND_SEQ_OPEN_DUPLEX, seq_flags) < 0);
    ck_err (snd_seq_set_client_name(alsa_seq, "LIBFMS") < 0);
    return 0;
  error:
    if(alsa_seq);
    return -1;
}


/* FIXME: Close THE sequencer client if no port remains */
static int
midi_alsa_close_local()
{
    if(--n_instances)
	return 0;

    //close seq

    return 0;
}


static int
destroy (midi_alsa_t * alsa)
{
    midi_alsa_close_local();

    return 0;
}


static int
put_msg (midi_alsa_t * alsa, midi_msg_t * msg)
{
    snd_seq_event_t ev;

    ck_err (!alsa || !alsa_seq || !msg);

    snd_seq_ev_clear      (&ev);
    snd_seq_ev_set_direct (&ev);
    snd_seq_ev_set_subs   (&ev);

    switch(MIDI_MSG_TYPE(msg))
	{
	case MIDI_NOTE_ON:
	    snd_seq_ev_set_noteon (&ev, MIDI_MSG_CHAN(msg), MIDI_MSG_NOTE(msg), MIDI_MSG_VELO(msg));
	    break;
	case MIDI_NOTE_OFF:
	    snd_seq_ev_set_noteoff (&ev, MIDI_MSG_CHAN(msg), MIDI_MSG_NOTE(msg), MIDI_MSG_VELO(msg));
	    break;
	case MIDI_CONTROL:
	    snd_seq_ev_set_controller (&ev, MIDI_MSG_CHAN(msg), MIDI_MSG_CTRL(msg), MIDI_MSG_VALUE(msg));
	    break;
	default:
	    return 0;
	}

    ck_err(snd_seq_event_output (alsa_seq, &ev) < 0);
    snd_seq_drain_output (alsa_seq);

    return 0;
  error:
    return -1;
}


/* remove and return the oldest message
 * if MIDI_FLAG_RT is set, the message must have a timestamp anterior to the global_timestamp
 * if MIDI_FLAG_EDIT is set, messages are not removed
 * add pulse argument ? manage timestamp at a global level or local ? */
static int
get_msg (midi_alsa_t * alsa, midi_msg_t * msg)
{
    int err;
    snd_seq_event_t *ev=0;

    ck_err (!alsa || !alsa_seq || !msg);

    msg->data[0]=0;

    if(snd_seq_event_input_pending(alsa_seq, 1)>0)
	{
	    if((err=snd_seq_event_input(alsa_seq, &ev)) < 0)
		if(err == -EAGAIN)
		    {
			msg->data[0]=0;
			msg->timestamp=-1;
			return 0;
		    }

	    msg->timestamp = ev->time.tick;//POUETTTT

/*      if(ev->time.tick)
        printf("ev->time.tick %d\n", ev->time.tick);
	if(ev->time.time.tv_sec)
        printf("ev->time.time.tv_sec %d\n", ev->time.time.tv_sec); */
	    switch (ev->type)
		{
		case SND_SEQ_EVENT_NOTE:
//          puts("SND_SEQ_EVENT_NOTE");
		    break;
		case SND_SEQ_EVENT_NOTEON:
		    MIDI_MSG_NOTE_ON(msg, ev->data.note.channel, ev->data.note.note, ev->data.note.velocity);
		    if(!ev->data.note.velocity)
			MIDI_MSG_SET_TYPE(msg, MIDI_NOTE_OFF);
		    //          done=TRUE;
		    break;
		case SND_SEQ_EVENT_NOTEOFF:
		    MIDI_MSG_NOTE_OFF(msg, ev->data.note.channel, ev->data.note.note, ev->data.note.velocity);
		    //          done=TRUE;
		    break;
		case SND_SEQ_EVENT_CONTROLLER://10
		    MIDI_MSG_SET_TYPE(msg, MIDI_CONTROL);
		    MIDI_MSG_SET_CHAN(msg, ev->data.control.channel);
		    MIDI_MSG_SET_PRGM(msg, ev->data.control.param);
		    MIDI_MSG_SET_VALUE(msg, ev->data.control.value);
		    //          done=TRUE;
		    break;
		    /** system status; event data type = #snd_seq_result_t */
		case SND_SEQ_EVENT_SYSTEM:
		case SND_SEQ_EVENT_RESULT:
		    /** note on and off with duration; event data type = #snd_seq_ev_note_t */
		case SND_SEQ_EVENT_KEYPRESS:
		    /** controller; event data type = #snd_seq_ev_ctrl_t */
		    break;
		case SND_SEQ_EVENT_PGMCHANGE:
		case SND_SEQ_EVENT_CHANPRESS:
		case SND_SEQ_EVENT_PITCHBEND:
		case SND_SEQ_EVENT_CONTROL14:
		case SND_SEQ_EVENT_NONREGPARAM:
		case SND_SEQ_EVENT_REGPARAM:
		    /** SPP with LSB and MSB values; event data type = #snd_seq_ev_ctrl_t */
		case SND_SEQ_EVENT_SONGPOS://20
		case SND_SEQ_EVENT_SONGSEL:
		case SND_SEQ_EVENT_QFRAME:
		case SND_SEQ_EVENT_TIMESIGN:
		case SND_SEQ_EVENT_KEYSIGN:
		    /** MIDI Real Time Start message; event data type = #snd_seq_ev_queue_control_t */
		case SND_SEQ_EVENT_START://30
		case SND_SEQ_EVENT_CONTINUE:
		case SND_SEQ_EVENT_STOP:
		case SND_SEQ_EVENT_SETPOS_TICK:
		case SND_SEQ_EVENT_SETPOS_TIME:
		case SND_SEQ_EVENT_TEMPO:
		    break;
		case SND_SEQ_EVENT_CLOCK:
		    MIDI_MSG_SET_STATUS(msg, MIDI_SYS_TIMING_CLOCK);
		    //          done=-1;
		    //          printf("SND_SEQ_EVENT_TICK\n");
		    break;
		case SND_SEQ_EVENT_TICK:
		    break;
		case SND_SEQ_EVENT_QUEUE_SKEW:
		case SND_SEQ_EVENT_SYNC_POS:
		    /** Tune request; event data type = none */
		case SND_SEQ_EVENT_TUNE_REQUEST://40
		case SND_SEQ_EVENT_RESET:
		case SND_SEQ_EVENT_SENSING:
		    //          done = -1;
		    break;
		    /** Echo-back event; event data type = any type */
		case SND_SEQ_EVENT_ECHO://50
		case SND_SEQ_EVENT_OSS:
		    /** New client has connected; event data type = #snd_seq_addr_t */
		    break;
		case SND_SEQ_EVENT_CLIENT_START://60
		case SND_SEQ_EVENT_CLIENT_CHANGE:
		case SND_SEQ_EVENT_CLIENT_EXIT:
		case SND_SEQ_EVENT_PORT_START:
		case SND_SEQ_EVENT_PORT_CHANGE:
		case SND_SEQ_EVENT_PORT_EXIT:
//          printf("SND_SEQ_EVENT:%d:addr:%d:%d\n", ev->type, ev->dest.client, ev->dest.port);
		    break;
		    /** Ports connected; event data type = #snd_seq_connect_t */
		case SND_SEQ_EVENT_PORT_UNSUBSCRIBED:
		case SND_SEQ_EVENT_PORT_SUBSCRIBED:
//          printf("SND_SEQ_EVENT_PORT_(UN)SUBSCRIBED:sender:%d:%d, dest:%d:%d\n", ev->data.connect.sender.client, ev->data.connect.sender.port, ev->data.connect.dest.client, ev->data.connect.dest.port);
		    /** Sample select; event data type = #snd_seq_ev_sample_control_t */
//          printf("ev->dest.client:%d\n", ev->dest.client);
		    break;
		case SND_SEQ_EVENT_SAMPLE://70
		case SND_SEQ_EVENT_SAMPLE_CLUSTER:
		case SND_SEQ_EVENT_SAMPLE_START:
		case SND_SEQ_EVENT_SAMPLE_STOP:
		case SND_SEQ_EVENT_SAMPLE_FREQ:
		case SND_SEQ_EVENT_SAMPLE_VOLUME:
		case SND_SEQ_EVENT_SAMPLE_LOOP:
		case SND_SEQ_EVENT_SAMPLE_POSITION:
		case SND_SEQ_EVENT_SAMPLE_PRIVATE1:
		    /** user-defined event; event data type = any (fixed size) */
		case SND_SEQ_EVENT_USR0://90
		case SND_SEQ_EVENT_USR1:
		case SND_SEQ_EVENT_USR2:
		case SND_SEQ_EVENT_USR3:
		case SND_SEQ_EVENT_USR4:
		case SND_SEQ_EVENT_USR5:
		case SND_SEQ_EVENT_USR6:
		case SND_SEQ_EVENT_USR7:
		case SND_SEQ_EVENT_USR8:
		case SND_SEQ_EVENT_USR9:
		    /** begin of instrument management */
		case SND_SEQ_EVENT_INSTR_BEGIN://100
		    /** end of instrument management */
		case SND_SEQ_EVENT_INSTR_END:
		    /** query instrument interface info */
		case SND_SEQ_EVENT_INSTR_INFO:
		    /** result of instrument interface info */
		case SND_SEQ_EVENT_INSTR_INFO_RESULT:
		    /** query instrument format info */
		case SND_SEQ_EVENT_INSTR_FINFO:
		    /** result of instrument format info */
		case SND_SEQ_EVENT_INSTR_FINFO_RESULT:
		    /** reset instrument instrument memory */
		case SND_SEQ_EVENT_INSTR_RESET:
		    /** get instrument interface status */
		case SND_SEQ_EVENT_INSTR_STATUS:
		    /** result of instrument interface status */
		case SND_SEQ_EVENT_INSTR_STATUS_RESULT:
		    /** put an instrument to port */
		case SND_SEQ_EVENT_INSTR_PUT:
		    /** get an instrument from port */
		case SND_SEQ_EVENT_INSTR_GET:
		    /** result of instrument query */
		case SND_SEQ_EVENT_INSTR_GET_RESULT:
		    /** free instrument(s) */
		case SND_SEQ_EVENT_INSTR_FREE:
		    /** get instrument list */
		case SND_SEQ_EVENT_INSTR_LIST:
		    /** result of instrument list */
		case SND_SEQ_EVENT_INSTR_LIST_RESULT:
		    /** set cluster parameters */
		case SND_SEQ_EVENT_INSTR_CLUSTER:
		    /** get cluster parameters */
		case SND_SEQ_EVENT_INSTR_CLUSTER_GET:
		    /** result of cluster parameters */
		case SND_SEQ_EVENT_INSTR_CLUSTER_RESULT:
		    /** instrument change */
		case SND_SEQ_EVENT_INSTR_CHANGE:
		    /** system exclusive data (variable length);  event data type = #snd_seq_ev_ext_t */
		case SND_SEQ_EVENT_SYSEX://130
		case SND_SEQ_EVENT_BOUNCE:
		case SND_SEQ_EVENT_USR_VAR0://135
		case SND_SEQ_EVENT_USR_VAR1:
		case SND_SEQ_EVENT_USR_VAR2:
		case SND_SEQ_EVENT_USR_VAR3:
		case SND_SEQ_EVENT_USR_VAR4:
		    break;
		default:
		    msg->data[0]=0;
		    msg->timestamp=-1;
		    break;
		}
	    /*      if(done < 0)
		    {
		    //          printf("INTERNAL EVENT #%d\n", ev->type);
		    done=0;
		    }
	    */
	    snd_seq_free_event(ev);
	}

    msg->timestamp=alsa->timestamp++;

    return 0;
  error:
    if(ev)
	snd_seq_free_event(ev);
    return -1;
}


/* int */
/* query_sub_client(midi_alsa_t *alsa) */
/* { */
/*   int client=0; */
/*   snd_seq_query_subscribe_t *query_subscribe; */

/*   ck_err(snd_seq_query_subscribe_malloc(&query_subscribe) < 0); */
/*   ck_err(snd_seq_query_port_subscribers(alsa_seq, alsa->portid, query_subscribe) < 0); */
/*   ck_err((client = snd_seq_query_subscribe_get_client(query_subscribe)) < 0); */
/*   ck_err(snd_seq_query_subscribe_free(query_subscribe) < 0); */
/*   return client; */
/* } */


/* static int */
/* port_info_client(midi_alsa_t *alsa) */
/* { */
/*   int client=0; */
/*   snd_seq_port_info_t *port_info=0; */

/*   ck_err(snd_seq_port_info_malloc(&port_info) < 0); */
/*   ck_err(snd_seq_get_port_info(alsa_seq, alsa->portid, port_info) < 0); */
/*   ck_err((client = snd_seq_port_info_get_client(port_info)) < 0); */
/*   snd_seq_port_info_free(port_info); */
/*   return client; */
/*  error: */
/*   if(port_info) */
/*     snd_seq_port_info_free(port_info); */
/*   return -1; */
/* } */


static int
get_data (midi_alsa_t * alsa, unsigned char *data, int size)
{
    return 0;
}


static int
put_data (midi_alsa_t * alsa, unsigned char *data, int size)
{
    return 0;
}
#endif
