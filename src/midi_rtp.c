#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>

#include <errno.h>

#include "midi_rtp.h"
#include "misc.h"


/* Net Stuffs */
int
midi_rtp_bind (midi_rtp_t * rtp, char *addr_name, int rtcp_port, int rtp_port)
{
  struct sockaddr_in addr;	/* for bind address   */

  ck_err ((rtp->fd = socket (AF_INET, SOCK_DGRAM, 0)) < 0);
  ck_err ((rtp->fdc = socket (AF_INET, SOCK_DGRAM, 0)) < 0);

  memset (&(addr.sin_zero), 0, 8);
  addr.sin_family = AF_INET;
  if (!addr_name)
    addr.sin_addr.s_addr = htonl (INADDR_ANY);
  else
    addr.sin_addr.s_addr = inet_addr (addr_name);
  addr.sin_port = htons (16112);	/* port 16112, from SDP */
  ck_err (bind (rtp->fd, (struct sockaddr *) &addr, sizeof (struct sockaddr))
	  < 0);
  /* ERROR_RETURN("Couldn't bind Internet RTP socket"); */

  memset (&(addr.sin_zero), 0, 8);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = addr.sin_addr.s_addr;
  addr.sin_port = htons (16113);	/* port 16113, from SDP */
  ck_err (bind (rtp->fdc, (struct sockaddr *) &addr, sizeof (struct sockaddr))
	  < 0);
  /* ERROR_RETURN("Couldn't bind Internet RTCP socket"); */

  return 0;
error:
  return -1;
}


int
midi_rtp_connect (midi_rtp_t * rtp, char *addr, int rtcp_port, int rtp_port)
{
  /* set RTP address, as coded in Figure 2's SDP */
  rtp->addr.sin_family = AF_INET;
  rtp->addr.sin_port = htons (rtp_port);
  rtp->addr.sin_addr.s_addr = inet_addr (addr);

  /* set RTCP address, as coded in Figure 2's SDP */
  rtp->addr.sin_family = AF_INET;
  rtp->addr.sin_port = htons (rtcp_port);	/* 5004 + 1 */
  rtp->addr.sin_addr.s_addr = rtp->addr.sin_addr.s_addr;

  /* Figure 6 -- Initializing destination addresses for RTP and RTCP. */
  return 0;
}


/* set non-blocking status, shield spurious ICMP errno */
int
sock_nb_ssie (int sock)
{
  int one = 1;

  ck_err (fcntl (sock, F_SETFL, O_NONBLOCK));
  /* ERROR_RETURN("Couldn't unblock Internet RTP socket"); */
  ck_err (setsockopt (sock, SOL_SOCKET, SO_BSDCOMPAT, &one, sizeof (one)));
  /* ERROR_RETURN("Couldn't shield RTP socket"); */
  return 0;
error:
  return -1;
}


int
midi_rtp_recv (midi_rtp_t * rtp)
{
  while ((rtp->rlen = recv (rtp->fd, rtp->rpacket, UDPMAXSIZE + 1, 0)) > 0)
    {
      /* process packet[], be cautious if (len == UDPMAXSIZE + 1) */
    }

  if ((rtp->rlen == 0) || (errno != EAGAIN))
    {
      /* while() may have exited in an unexpected way */
    }

  return 0;
}

/*  Algorithm for Sending a Packet: */
/*   1. Generate the RTP header for the new packet.  See Section 2.1 */
/*      of [1] for details. */
/*   2. Generate the MIDI command section for the new packet.  See */
/*      Section 3 of [1] for details. */
/*   3. Generate the recovery journal for the new packet.  We discuss */
/*      this process in Section 5.2.  The generation algorithm examines */
/*      the Recovery Journal Sending Structure (RJSS), a stateful */
/*      coding of a history of the stream. */
/*   4. Send the new packet to the receiver. */
/*   5. Update the RJSS to include the data coded in the MIDI command */
/*      section of the packet sent in step 4.  We discuss the update */
/*      procedure in Section 5.3. */
/*          Figure 8 -- A 5 step algorithm for sending a packet. */
int
midi_rtp_send (midi_rtp_t * rtp)
{
  /* first fill packet[] and set size ... then: */

  if (sendto
      (rtp->fd, rtp->spacket, rtp->slen, 0, (struct sockaddr *) &rtp->addr,
       sizeof (struct sockaddr)) == -1)
    {
      /* try again later if errno == EAGAIN or EINTR */
      /* other errno values --> an operational error */
    }

  /* Figure 7 -- Using sendto() to send an RTP packet. */

  return 0;
}


int
midi_rtp_get_msg (midi_rtp_t * rtp, midi_msg_t * msg)
{
  /* If the rpacket is totaly readed, recv the next */
  /* Read the timestamp of the next msg in the packet */
  /* eventualy return a generated tick */
  /* return the next packet */
  /* if no message, return null msg */
  return 0;
}


int
midi_rtp_put_msg (midi_rtp_t * rtp, midi_msg_t * msg, int ticks)
{
  /* Fill the spacket */
  /* If the first msg is too old, send the packet */
  return 0;
}


int
midi_rtp_spacket_init (midi_rtp_t * rtp)
{
  rtp->spacket[0] = 0;


  return 0;
}
