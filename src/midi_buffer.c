/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 c-file-style: "gnu" -*- */
#include "midi_buffer.h"
#include "misc.h"
#include "list.h"
/* as pipe, msg are removed when get it */


int
midi_pipe_open (midi_pipe_t * pipe, char *name, midi_flags_t flags)
{
  return 0;
}


int
midi_pipe_close (midi_pipe_t * pipe)
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
 * if MIDI_FLAG_RT is set, the message must have a timestamp anterior to the global_timestamp
 * if MIDI_FLAG_EDIT is set, messages are not removed
 * add pulse argument ? manage timestamp at a global level or local ? */
int
midi_pipe_get_msg (midi_pipe_t * pipe, midi_msg_t * msg)
{
  int /* timestamp,  */pulses;
  node_t *node;
  ck_err (!pipe || !msg);

/*   if(pipe->flags & MIDI_FLAG_TB_LOCAL) */
/*     pipe->timestamp_get += pulses; */

  //  printf("midi_pipe_get_msg:pipe->timestamp_get:%d\n", pipe->timestamp_get);

  if (pipe->msgs.first)
    {
/*       if((!(pipe->flags & MIDI_FLAG_RT)) || */
/* 	 ((midi_msg_t *) node->data)->timestamp <= global_timestamp) */
	{
	  node = pipe->msgs.first;
	  *msg = *((midi_msg_t *) node->data);
	  //if(pipe->flags & MIDI_FLAG_PIPE)
	  list_remove (&pipe->msgs, node);
	  free(node);

	  pulses = msg->timestamp - pipe->timestamp_get;
	  printf("GET msg timestamp : %d\n", msg->timestamp);
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


int
midi_pipe_put_msg (midi_pipe_t * pipe, midi_msg_t * msg, int pulses)
{
  node_t *node;

  ck_err (!msg || !pipe);

  ck_err (pulses < 0);

  node = Malloc (sizeof (node_t) + sizeof (midi_msg_t));
  node->data = ((void *) node) + sizeof (node_t);
  *((midi_msg_t *) node->data) = *msg;
  ((midi_msg_t *) node->data)->timestamp = (pipe->timestamp_put += pulses);
  list_add_end (&pipe->msgs, node);

  printf("midi_pipe_put_msg:pipe->timestamp_put:%d\n", pipe->timestamp_put);

  return pulses;
error:
  return -1;
}


int
midi_pipe_get_data (midi_pipe_t * pipe, unsigned char *data, int size)
{
  return 0;
}


int
midi_pipe_put_data (midi_pipe_t * pipe, unsigned char *data, int size)
{
  return 0;
}
