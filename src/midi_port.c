#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "midi_port.h"
#include "misc.h"


int
midi_port_open (midi_port_t * port, char *file_name, midi_flags_t flags)
{
  ck_err ((port->fd = open (file_name, O_RDONLY)) < 0);

  return 0;
error:
  return -1;
}

/*
 *  3               2      1 3 22    2
 *  9        r      1     -1   rr    r
 *  -t---t---t---t---t---t---t---t---t---t->
 */

int
midi_port_wait_and_read_byte (int fd, unsigned char *b, int timeout)
{
  fd_set fs;
  //  struct timeval tv;

  FD_SET (fd, &fs);

/*    if(ticks>=0) */
/*      if(port->flags & MIDI_FLAG_TICK) */
/*        TVSetDelay(tv, ticks * port->notpqof); */
/*      else */
/*        TVSetDelay(tv, ticks * port->notpqof * 2); */

  select (fd + 1, &fs, 0, 0, 0);
  if (b)
    ck_err (read (fd, b, 1) <= 0);

  return 0;
error:
  return -1;
}


int
midi_port_get_msg (midi_port_t * port, midi_msg_t * msg)
{
  int ticks;

  ck_err (!(port->flags & MIDI_FLAG_GET) || !port || !msg);

  return ticks;
error:
  return -1;
}


int
midi_port_put_msg (midi_port_t * port, midi_msg_t * msg, int ticks)
{
  ck_err (!(port->flags & MIDI_FLAG_PUT) || !port || !msg);

  return ticks;
error:
  return -1;
}


int
midi_port_close (midi_port_t * port)
{
  close (port->fd);

  return 0;
}
