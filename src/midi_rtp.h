#ifndef _SIMILI_RTP_H
#define _SIMILI_RTP_H


#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "simili.h"
#include "misc.h"


#define UDPMAXSIZE 1472		/* based on Ethernet MTU of 1500 */


typedef struct midi_rtp_rjss_w_s midi_rtp_rjss_w_t;
typedef struct midi_rtp_rjss_n_s midi_rtp_rjss_n_t;
typedef struct midi_rtp_rjss_c_s midi_rtp_rjss_c_t;
typedef struct midi_rtp_rjss_p_s midi_rtp_rjss_p_t;
typedef struct midi_rtp_rjss_channel_s midi_rtp_rjss_channel_t;
typedef struct midi_rtp_rjss_s midi_rtp_rjss_t;
typedef struct midi_rtp_s midi_rtp_t;


/* leaf level of hierarchy: Chapter W, Appendix A.3 of [1] */
struct midi_rtp_rjss_w_s
{				/* Pitch Wheel (0xE)       */
  u8_t chapterw[2];		/* bitfield (Figure A.3.1, [1])   */
  u32_t seqnum;			/* extended sequence number, or 0 */
};

/* leaf level of hierarchy: Chapter N, Appendix A.4 of [1]   */
struct midi_rtp_rjss_n_s
{				/* Note commands (0x8, 0x9)       */
  u8_t chaptern[272];		/* bitfield (Figure A.4.1, [1])   */
  u16_t size;			/* actual size of chaptern[]      */
  u32_t seqnum;			/* extended sequence number, or 0 */
  u32_t note_seqnum[128];	/* most recent note seqnum, or 0  */
  u32_t note_tstamp[128];	/* NoteOn execution timestamp     */
  u8_t note_state[128];		/* NoteOn velocity, 0 -> NoteOff  */
};

/* leaf level of hierarchy: Chapter C, Appendix A.7 of [1] */
struct midi_rtp_rjss_c_s
{				/* Control Change (0xB) */
  u8_t chapterc[257];		/* bitfield (Figure A.7.1, [1])   */
  u16_t size;			/* actual size of chapterc[]      */
  u32_t seqnum;			/* extended sequence number, or 0 */
  u8_t control_state[128];	/* per-number control state */
  u32_t control_seqnum[128];	/* most recent seqnum, or 0 */
};

/* leaf level of hierarchy: Chapter P, Appendix A.2 of [1] */
struct midi_rtp_rjss_p_s
{				/* MIDI Program Change (0xC) */
  u8_t chapterp[3];		/* bitfield (Figure A.2.1, [1])   */
  u32_t seqnum;			/* extended sequence number, or 0 */
};

/* second-level of hierarchy, for channel journals */
struct midi_rtp_rjss_channel_s
{
  u8_t cheader[3];		/* header bitfield (Figure 8, [1]) */
  u32_t seqnum;			/* extended sequence number, or 0  */
  midi_rtp_rjss_p_t chapterp;	/* chapter P info  */
  midi_rtp_rjss_w_t chapterw;	/* chapter W info  */
  midi_rtp_rjss_n_t chaptern;	/* chapter N info  */
  midi_rtp_rjss_c_t chapterc;	/* chapter C info  */
};

/* top level of hierarchy, for recovery journal header */
struct midi_rtp_rjss_s
{
  u8_t jheader[3];		/* header bitfield (Figure 7, [1])  */
  /* Note: Empty journal has a header */
  u32_t seqnum;			/* extended sequence number, or 0   */
  /* seqnum = 0 codes empty journal   */
  midi_rtp_rjss_channel_t channels[16];	/* channel journal state */
  /* index is MIDI channel */
};


struct midi_rtp_s
{
  midi_flags_t flags;
  int fd, fdc;			//local sockets, rtp and rtcp
  struct sockaddr_in addr, addrc;	//peer addresse, rtp and rtcp
  unsigned char rpacket[UDPMAXSIZE + 1];	//recv packet buff
  unsigned char spacket[UDPMAXSIZE + 1];	//recv packet buff
  int rlen, slen;
  midi_rtp_rjss_t rjss;
};


//int midi_rtp_bind(midi_rtp_t *rtp, char *rtp_name, midi_flags_t flags);
int midi_rtp_get_msg (midi_rtp_t * rtp, midi_msg_t * msg);
int midi_rtp_put_msg (midi_rtp_t * rtp, midi_msg_t * msg, int ticks);
int midi_rtp_get_sysex (midi_rtp_t * rtp, void *buff, int *size);
int midi_rtp_close (midi_rtp_t * rtp);
int midi_rtp_get_data (midi_rtp_t * rtp, unsigned char *buffer, int size);
int midi_rtp_put_data (midi_rtp_t * rtp, unsigned char *buffer, int size);
int midi_rtp_get_ppqn (midi_rtp_t * rtp);
int midi_rtp_set_ppqn (midi_rtp_t * rtp, int ppqn);


#endif
