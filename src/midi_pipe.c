/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 c-file-style: "gnu" -*- */
//#define MIDI_PIPE_NONBLOCK
#ifdef MIDI_PIPE_NONBLOCK
#include <sys/poll.h>
#include <unistd.h>
#define PIPE_IN  0
#define PIPE_OUT 1
#endif
#include "midi.h"
#include "misc.h"
#include "list.h"
#include "obj.h"
/* as pipe, msg are removed when get it */


//DEFINITIONS
typedef struct midi_pipe_s midi_pipe_t;
typedef struct midi_pipe_class_s midi_pipe_c;
typedef struct voice_s voice_t;
typedef int timestamp_t;


static int destroy (midi_pipe_t*);
static int get_msg (midi_pipe_t*, midi_msg_t*);
static int put_msg (midi_pipe_t*, midi_msg_t*);
static int get_data (midi_pipe_t*, unsigned char*, int);
static int put_data (midi_pipe_t*, unsigned char*, int);
static int sort(midi_pipe_t*);
static int timestamp(midi_pipe_t*, int);


struct midi_pipe_s
{
  midi_t midi;
  midi_flags_t flags;
  list_t msgs;
  int timestamp_put;
  int timestamp_get;
  int timestamp;
  int caos;
#ifdef MIDI_PIPE_NONBLOCK
  int pipe_fd[2];
#endif
};


struct midi_pipe_class_s
{
  midi_c midi;
};


obj_c*
midi_pipe_class()
{
  static obj_c *class=0;

  if(!class)
    {
      create_class(midi_pipe, midi);
      class->destroy = (obj_destroy_t*)destroy;
      MIDI_CLASS(class)->put_msg = (midi_put_msg_t*)put_msg;
      MIDI_CLASS(class)->get_msg = (midi_get_msg_t*)get_msg;
      MIDI_CLASS(class)->put_data = (midi_put_data_t*)put_data;
      MIDI_CLASS(class)->get_data = (midi_get_data_t*)get_data;
      MIDI_CLASS(class)->timestamp = (midi_timestamp_t*)timestamp;
    }

  return class;
}

static int evaluator(void* mmsg)
{
  return ((midi_msg_t*)mmsg)->timestamp;
}

midi_t*
midi_pipe_create (midi_flags_t flags)
{
  midi_pipe_t * p=0;

  ck_err(!(p = obj_create(midi_pipe)));
  MIDI(p)->flags = flags;

#ifdef MIDI_PIPE_NONBLOCK
  if(!MIDI(p)->flags & MIDI_FLAG_NONBLOCK)
    ck_err(pipe(p->pipe_fd) < 0);
#endif

  return MIDI(p);
 error:
  return 0;
}


static int
timestamp(midi_pipe_t * pipe, int timestamp)
{
  //  printf("midi_pipe: timestamp: %d\n", timestamp);
  return pipe->timestamp = timestamp;
}

static int
destroy (midi_pipe_t * pipe)
{
  node_t *node;

  node = pipe->msgs.first;

  while (node)
    {
      node_t *tmp = node;
      node = tmp->next;
      list_remove (&pipe->msgs, tmp);
      free (tmp);
    }

  return 0;
}


/* remove and return the oldest message
 * if MIDI_FLAG_RT is not set, the message must have a timestamp anterior to the global_timestamp
 * if MIDI_FLAG_EDIT is set, messages are not removed
 * add pulse argument ? manage timestamp at a global level or local ? */
static int
get_msg (midi_pipe_t * pipe, midi_msg_t * msg)
{
  int /* timestamp,  */pulses;
  node_t *node;
  ck_err (!pipe || !msg);

  if(pipe->caos)
    ck_err(sort(pipe) < 0);

  /*   if(pipe->flags & MIDI_FLAG_TB_LOCAL) */
  /*     pipe->timestamp_get += pulses; */

  //  printf("midi_pipe_get_msg:pipe->timestamp_get:%d\n", pipe->timestamp_get);

  
#ifdef MIDI_PIPE_NONBLOCK
  while(!pipe->msgs.first && !(MIDI(pipe)->flags&MIDI_FLAG_NONBLOCK))
    {
      struct pollfd popol;
      popol.fd = pipe->pipe_fd[PIPE_OUT];
      popol.events = POLLIN;
      poll(&popol, 1, -1);
      while(read(pipe->pipe_fd[PIPE_OUT], &popol, sizeof(popol))==sizeof(popol));//just empty the fifo
    }
#endif

  if (pipe->msgs.first)
    {
      node = pipe->msgs.first;
      //      printf("GET msg timestamp %s: %d, %d\n", MIDI(pipe)->flags&MIDI_FLAG_RT?"*RT*":"", ((midi_msg_t *) node->data)->timestamp, pipe->timestamp);
      if((MIDI(pipe)->flags&MIDI_FLAG_RT) || ((midi_msg_t *) node->data)->timestamp <= pipe->timestamp)
	{
	  *msg = *((midi_msg_t*) node->data);
	  //if(pipe->flags & MIDI_FLAG_PIPE)
	  list_remove (&pipe->msgs, node);
	  free(node);

	  pulses = msg->timestamp - pipe->timestamp_get;
          //          printf("GET msg timestamp : %d\n", msg->timestamp);
	  ck_err(pulses < 0);
	  pipe->timestamp_get = msg->timestamp;
	  return pulses;
	}
    }

  msg->data[0]=0;
  msg->timestamp=-1;
  return 0;

error:
  return -1;
}


static int
put_msg (midi_pipe_t * pipe, midi_msg_t * msg)
{
  node_t *node;

  ck_err (!msg || !pipe);

  node = node_create(Talloc(midi_msg_t));
  *((midi_msg_t *) node->data) = *msg;

   ck_err(list_insort(&pipe->msgs, node, evaluator) < 0);
  //  printf("midi_pipe_put_msg: ((midi_msg_t *) node->data)->timestamp: %d\n", ((midi_msg_t *) node->data)->timestamp);

#ifdef MIDI_PIPE_NONBLOCK
  if(!MIDI(p)->flags & MIDI_FLAG_NONBLOCK && pipe->msgs.first == pipe->msgs.last)
    write(pipe->pipe_fd[PIPE_IN], &pipe, 1);
#endif

  return 0;
error:
  return -1;
}


static int
get_data (midi_pipe_t * pipe, unsigned char *data, int size)
{
  return -1;
}


static int
put_data (midi_pipe_t * pipe, unsigned char *data, int size)
{
  return -1;
}


static int
sort(midi_pipe_t *pipe)
{
  printf("midi_pipe: sorting events: ");
  fflush(stdout);
  ck_err(list_sort(&pipe->msgs, evaluator) < 0);
  pipe->caos=0;
  printf("done\n");
  return 0;
 error:
  return -1;
}
