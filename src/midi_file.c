 /* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 8 c-file-style: "gnu" -*- */
#define DEBUG
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "midi_file.h"
#include "midi.h"
#include "misc.h"

//DEFINITIONS
typedef struct midi_file_s midi_file_t;
typedef struct midi_file_class_s midi_file_c;

static int destroy (midi_file_t*);
static int get_msg (midi_file_t*, midi_msg_t*);
static int put_msg (midi_file_t*, midi_msg_t*);
static int get_data (midi_file_t*, unsigned char*, int);
static int put_data (midi_file_t*, unsigned char*, int);

struct midi_file_class_s
{
  midi_c midi;
};

obj_c*
midi_file_class()
{
  static obj_c *class=0;

  if(!class)
    {
      create_class(midi_file, midi);
      class->destroy = (obj_destroy_t*)destroy;
      MIDI_CLASS(class)->put_msg = (midi_put_msg_t*)put_msg;
      MIDI_CLASS(class)->get_msg = (midi_get_msg_t*)get_msg;
      MIDI_CLASS(class)->put_data = (midi_put_data_t*)put_data;
      MIDI_CLASS(class)->get_data = (midi_get_data_t*)get_data;
    }

  return class;
}

struct midi_file_s
{
  midi_t midi;
  int fd;
  unsigned char status;//midi compress trick
  unsigned short format;//multitrack&async
  unsigned short nb_tracks;
  unsigned short tpqn;		//Ticks Per Quarter of Note
  int cur_track;
  int cur_track_off;//offset of the header of the current track
  int cur_track_size;//==-1 if closed
  int data_size;    //if data follow ex:-1:sysex/text, 
  int timestamp;
};

//FIXME: Buffurize file.
//MIDI_FLAG_GET_FILE, the META event will be readed

#define MIDI_MAGIC  0x4D546864
#define TRACK_MAGIC 0x4D54726B

#define msg_chan_size(msg) (_msg_chan_size[(msg->data[0]>>4)&0x7])
#define msg_sys_size(msg)  (_msg_sys_size[msg->data[0]&0xF])

static const int _msg_chan_size[] = { 3, 3, 3, 3, 2, 2, 3, 0 };
static const int _msg_sys_size[] =
  { -1, -1, 3, 2, -1, -1, 0, -1, 0, -1, 0, 0, 0, -1, 0, -1 };

static int close_track (midi_file_t * file);
static int open_track (midi_file_t * file);

typedef enum
  {
    MFGM_STATE_TICKS = 0,
    MFGM_STATE_STATUS,
    MFGM_STATE_MSG,
    MFGM_STATE_DATA,
    MFGM_STATE_SYSIZED,
    MFGM_STATE_SYS_META,
  }
mfgm_state_e;

//FUNCTIONS
midi_t*
midi_file_create (char const *file_name, midi_flags_t flags)
{
  midi_file_t * file=0;
  char buff[14];
  int open_flags;

  ck_err(flags & MIDI_FLAG_GET && flags & MIDI_FLAG_PUT);

  file = obj_create(midi_file);
  MIDI(file)->flags = flags;

  if (flags & MIDI_FLAG_GET)
    {
      open_flags = O_RDONLY;

      ck_err ((file->fd = open (file_name, open_flags)) < 0);
      ck_err (read (file->fd, buff, 14) != 14);

      file->format = ntohs (*(u16_t *) &buff[8]);
      file->nb_tracks = ntohs (*(u16_t *) &buff[10]);
      file->tpqn = ntohs (*(u16_t *) &buff[12]);
      ck_err (ntohl (*(u32_t *) &buff[0]) != MIDI_MAGIC);
      if(ntohl (*(u32_t *) &buff[4])-6)
        {
          //WARNING: unexpected size (size!=6)
          lseek(file->fd, ntohl (*(u32_t *) &buff[4])-6, SEEK_CUR);
        }
      ck_err (!ntohs (*(u16_t *) &buff[12]) & 0x80);

      //      printf ("format :\t%d\nnr tracks :\t%d\ntpqn :\t%d\n",
      //            file->format, file->nb_tracks, file->tpqn);
    }
  else
    {
      open_flags = O_WRONLY;

      if (flags & MIDI_FLAG_CREATE)
        {
          ck_err (!(flags & MIDI_FLAG_PUT));
          open_flags |= O_CREAT;
          if(0)
            open_flags |= O_EXCL;
          else
            open_flags |= O_TRUNC;
          //ck_err ((file->fd = open (file_name, open_flags)) < 0);
          ck_err ((file->fd = open (file_name, open_flags, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) < 0);
        }
      else
        {
          ck_err ((file->fd = open (file_name, open_flags)) < 0);
        }
      file->format = 1;		//(flags & MIDI_FLAG_MULTI_TRACK);
      file->nb_tracks = 0;
      file->tpqn = 480;
      //	open_track(file);
    }

  file->cur_track_off = 14;
  file->cur_track = 0;
  file->cur_track_size = -1;
  file->data_size = 0;

  return MIDI(file);
 error:
  return 0;
}

int
midi_file_next_track(midi_t *file)
{
  ck_err(!IS_MIDI_FILE(file));

  ck_err(close_track(MIDI_FILE(file)) < 0);
  if(MIDI(file)->flags & MIDI_FLAG_GET)
    return open_track(MIDI_FILE(file));
  else
    return 0;

 error:
  return -1;
}

int
midi_file_get_tpqn (midi_t * file)
{
  ck_err(!IS_MIDI_FILE(file));

  return (MIDI_FILE(file))->tpqn;
 error:
  return -1;
}

int
midi_file_set_tpqn (midi_t * file, int tpqn)
{
  ck_err(!IS_MIDI_FILE(file));

  return (MIDI_FILE(file))->tpqn = tpqn;
 error:
  return -1;
}

int
midi_file_get_nb_tracks(midi_t *file)
{
  ck_err(!IS_MIDI_FILE(file));

  return (MIDI_FILE(file))->nb_tracks;
 error:
  return -1;
}

//STATIC FUNCTIONS
static int
p2t (int pulses)
{
  return pulses;
}

static int
t2p (int ticks)
{
  return ticks;
}

static int
put_byte (int fd, unsigned char b)
{
  //printf("%02x ", b);
  return write (fd, &b, 1);
}

static int
get_byte (int fd, unsigned char *b)
{
  int r;

  r = read (fd, b, 1);
  //printf("%02x ", *b);
  return r;
}

static int
put_var (int fd, int var)
{
  /* write var lenght */

  if (var >= (1 << 7))
    {
      if (var >= (1 << 14))
        {
          if (var >= (1 << 21))
            {
              put_byte (fd, ((var & 0xFE00000) >> 21) | 0x80);
            }
          put_byte (fd, ((var & 0x1FC000) >> 14) | 0x80);
        }
      put_byte (fd, ((var & 0x3F80) >> 7) | 0x80);
    }
  put_byte (fd, var & 0x7F);
  /*    while(pulses<0x80) */
  /*      { */
  /*        write_byte((pulses & 0x7F) | 0x80); */
  /*        pulses >>=7; */
  /*      } */
  /*    write_byte(pulses & 0x7F); */

  return 0;
}

static int
open_track (midi_file_t * file)
{
  int off;
  unsigned char buffer[8];

  if(MIDI(file)->flags & MIDI_FLAG_GET && file->cur_track >= file->nb_tracks)
    return 0;

  if (MIDI(file)->flags & MIDI_FLAG_GET)
    {
      /* on se positionne */
      off = file->cur_track_off;
      ck_err (lseek (file->fd, off, SEEK_SET) != off);
      /* on lit l'entête */
      ck_err (read (file->fd, buffer, 8) != 8);
      ck_err (ntohl (*(int *) &buffer[0]) != TRACK_MAGIC);
      file->cur_track_size = ntohl (*(int *) &buffer[4]);
      file->cur_track_off = off + 8;
      file->cur_track++;
    }
  else
    {
      file->cur_track_size = 0;
      lseek (file->fd, file->cur_track_off + 8, SEEK_SET);
    }

  file->timestamp = 0;

  return 1;
 error:
  return -1;
}

static int
close_track (midi_file_t * file)
{
  unsigned char buffer[8];

  ck_err(!IS_MIDI_FILE(file));

  if(file->cur_track_size==-1)
    return 0;

  if (MIDI(file)->flags & MIDI_FLAG_PUT)
    {
      /* on écrit la taille de la track précédante */
      put_byte(file->fd, 0x00);//Right now
      put_byte(file->fd, 0xFF);//META EVENT
      put_byte(file->fd, 0x2F);//END OF TRACK
      put_byte(file->fd, 0x00);//0 BYTES LEFT
      file->cur_track_size =
        lseek (file->fd, 0, SEEK_CUR) - file->cur_track_off;
      lseek (file->fd, file->cur_track_off, SEEK_SET);
      *(int *) &buffer[0] = htonl (TRACK_MAGIC);
      *(int *) &buffer[4] = htonl (file->cur_track_size - 8);
      ck_err (write (file->fd, buffer, 8) != 8);
      file->cur_track++;
      file->nb_tracks++;
    }

  file->cur_track_off += file->cur_track_size;
  file->cur_track_size = -1;

  return 0;
 error:
  return -1;
}

//METHODES
static int
destroy (midi_file_t * file)
{
  char buff[14];

  ck_err(!IS_MIDI_FILE(file));

  if (MIDI(file)->flags & MIDI_FLAG_PUT)
    {
      //      puts("CLOSE");
      if (file->cur_track_size != -1)
        close_track (file);
      lseek (file->fd, 0, SEEK_SET);
      *(int *) &buff[0] = ntohl (MIDI_MAGIC);
      *(int *) &buff[4] = ntohl (6);
      *(short *) &buff[8] = htons (file->format);
      *(short *) &buff[10] = htons (file->nb_tracks);
      *(short *) &buff[12] = htons (file->tpqn);
      ck_err (write (file->fd, buff, 14) != 14);
      lseek (file->fd, 0, SEEK_END);
    }

  close (file->fd);

  return 0;
 error:
  return -1;
}

static int
get_msg (midi_file_t * file, midi_msg_t * msg)
{
  unsigned char b;
  int ticks = 0;
  int msg_index = 0;
  int msg_size = 0;
  int vle_index = 0;
  mfgm_state_e state;

  ck_err(!IS_MIDI_FILE(file));

  //printf("GET[%x] ", (int)lseek (file->fd, 0, SEEK_CUR));

  ck_err (!file || !(MIDI(file)->flags & MIDI_FLAG_GET) || !msg);

  get_data (file, 0, 0);

  state = MFGM_STATE_TICKS;

  for (b = 0; b < MIDI_MSG_SIZE; b++)
    msg->data[b] = 0;

  if (file->cur_track_size == -1 && file->cur_track >= file->nb_tracks)
    goto eof;

  if (file->cur_track_size == -1)
    ck_err (open_track (file) < 0);

  while (get_byte (file->fd, &b))
    {
      switch (state)
        {
        case MFGM_STATE_TICKS:
          ticks <<= 7;
          ticks += b & 0x7F;
          if (!(b & 0x80))
            {
              //	      printf("T[%x] \n", ticks);
              state = MFGM_STATE_STATUS;
              msg->timestamp+=ticks;
              vle_index = 0;
            }
          ck_err (vle_index > 4);
          break;
        case MFGM_STATE_STATUS:
          if (b & MIDI_STATUS_MASK)
            {
              ck_err (msg_index);	//KICK ME !
              file->status = b;
              msg->data[msg_index++] = b;
              switch (b & MIDI_TYPE_MASK)
                {
                case MIDI_SYSTEM:
                  switch (msg_size = msg_sys_size (msg))
                    {
                    case -1:	//unfixed size msg
                      switch (b)
                        {
                        case MIDI_SYS_EX:
                          msg->data[msg_index++] = b;
                          file->data_size = -1;
                          goto ret;
                          break;
                        case MIDI_SYS_META:	//FIXME On file
                          state = MFGM_STATE_SYS_META;
                          break;
                        }
                      break;
                    case 1:	//single byte msg
                      goto ret;
                    default:	//fixed size msg
                      state = MFGM_STATE_MSG;
                      break;
                    }
                  break;
                default:
                  ck_err (!(b & 0x80));
                  msg_size = msg_chan_size (msg);
                  ck_err (msg_size >= MIDI_MSG_SIZE);
                  state = MFGM_STATE_MSG;
                  break;
                }
            }
          else
            {
#if 0
              if (b == 0x7F)	//On file it could replace 0xF0 SYSEX
                {
                  ck_err (msg_index);
                  msg->data[0] = b;
                  file->data_size = -1;
                  goto ret;
                }
#endif
              msg->data[msg_index++] = file->status;
              msg->data[msg_index++] = b;
              msg_size = msg_chan_size (msg);
              ck_err (msg_size >= MIDI_MSG_SIZE);
              state = MFGM_STATE_MSG;
            }
          break;

        case MFGM_STATE_MSG:
          ck_err (msg_index >= MIDI_MSG_SIZE);
          msg->data[msg_index++] = b;
          if (msg_index >= msg_size)
            goto ret;
          break;

        case MFGM_STATE_SYS_META:
          msg->data[msg_index++] = b;
          switch (b)
            {
            case MIDI_SYS_META_TEXT:
            case MIDI_SYS_META_COPYRIGHT:
            case MIDI_SYS_META_TRACKNAME:
            case MIDI_SYS_META_INSTRNAME:
            case MIDI_SYS_META_LIRIKS:
            case MIDI_SYS_META_MARK:
            case MIDI_SYS_META_SPECIAL:
            case MIDI_SYS_META_PROPRIO:
            case MIDI_SYS_META_TEXTEVENT:
              state = MFGM_STATE_DATA;	//big data
              ck_err(file->data_size!=0);
              break;
            case MIDI_SYS_META_ENDOFTRACK:
            case MIDI_SYS_META_MIDIPORT:
            case MIDI_SYS_META_TEMPO:
            case MIDI_SYS_META_OFFSMPTE:
            case MIDI_SYS_META_METRONOME:
            case MIDI_SYS_META_KEY:
            default:
              state = MFGM_STATE_SYSIZED;	//max 8 bytes
              break;
            }
          break;
        case MFGM_STATE_SYSIZED:
          msg->data[msg_index++] = b;
          if (!b)
            goto ret;
          msg_size = 3 + b;

          state = MFGM_STATE_MSG;
          break;
        case MFGM_STATE_DATA:
          msg->data[msg_index++] = b;
          file->data_size += ((int) (b & 0x7F) << (7 * vle_index++));
          if (!(b & 0x80))
            {
              vle_index = 0;
              goto ret;
            }
          ck_err (vle_index > 4);
          break;
        }
    }
  //MIDI_SYS_META_OFFSMPTE
 eof:
  msg->timestamp = -1;
  msg->data[0] = 0;
  return 0;
 ret:
  //FIXME:
  msg->timestamp = (file->timestamp+=ticks);
  if(MIDI_MSG_EOT(msg))
    ck_err(close_track(file) < 0);
  return ticks;
 error:
  return -1;
}

static int
put_msg (midi_file_t * file, midi_msg_t * msg)
{
  int vle_index = 0, i;
  int pulses=0;

  ck_err(!IS_MIDI_FILE(file));

  ck_err (!(MIDI(file)->flags & MIDI_FLAG_PUT) || !file || !msg);
  if (file->cur_track_size == -1)
    ck_err (open_track (file) < 0);

  //pulses stuffs
  if(msg->timestamp<0)
    msg->timestamp=file->timestamp;
  ck_err(msg->timestamp < file->timestamp);
  pulses = msg->timestamp - file->timestamp;
  file->timestamp = msg->timestamp;
  //printf("timestamp: %d\n", file->timestamp);
  put_var (file->fd, p2t(pulses));

  if (file->data_size)
    ck_err (put_data (file, 0, 0) < 0);

  switch (MIDI_MSG_TYPE (msg))
    {
    case MIDI_SYSTEM:
      file->status = 0;
      if (MIDI_MSG_STATUS (msg) == MIDI_SYS_EX)
        {
          put_byte (file->fd, msg->data[0]);
          file->data_size = -1;
        }
      else if (MIDI_MSG_STATUS (msg) == MIDI_SYS_META)
        {
          if (MIDI_MSG_DATA1 (msg) && MIDI_MSG_DATA1 (msg) <= 7)
            {
              put_byte (file->fd, msg->data[0]);
              put_byte (file->fd, msg->data[1]);
              i = 2;
              do
                {
                  ck_err (vle_index > 4);
                  file->data_size +=
                    ((int) (msg->data[i] & 0x7F)) << (7 * vle_index++);
                  put_byte (file->fd, msg->data[i]);
                }
              while (msg->data[i++] & 0x80);
            }
          else if (MIDI_MSG_DATA1 (msg) == 0x7F)
            {
              put_byte (file->fd, msg->data[0]);
              put_byte (file->fd, msg->data[1]);
              file->data_size = -1;
            }
          else
            {
              //            printf("PUTS : SYS VAR : ");
              for (i = 0; i < msg->data[2] + 3; i++)
                {
                  //                printf("%x ", msg->data[i]);
                  put_byte (file->fd, msg->data[i]);
                }
              //            puts("");
              if (MIDI_MSG_EOT (msg))
                close_track (file);
            }
          goto end;
        }
      //MIDI_MSG_META
      else
        switch (msg_sys_size (msg))
          {
          case -1:
            ck_err ("Bad sys msg");
          case 0:
            break;
          }
      break;
    default:
      if (!MIDI_MSG_STATUS (msg))
        break;
      i = file->status == MIDI_MSG_STATUS (msg) ? 1 : 0;
      for (; i < msg_chan_size (msg); i++)
        put_byte (file->fd, msg->data[i]);
      file->status = MIDI_MSG_STATUS (msg);
      break;
    }

 end:
  //puts("");
  //why pulses ??
  return pulses;
 error:
  return -1;
}

static int
get_data (midi_file_t * file, unsigned char *buffer, int size)
{
  ck_err(!IS_MIDI_FILE(file));

  if (file->data_size)
    {
      //printf("GED\n");
      if (buffer && size > 0)
        {
          if (file->data_size > 0)
            {
              //DATA
              if (size < file->data_size)
                file->data_size -= size;
              else
                size = file->data_size;
              ck_err (read (file->fd, buffer, size) != size);
            }
          else
            {
              int i = 0;
              //SYSEX
              do
                {
                  ck_err (get_byte (file->fd, &buffer[i]) < 0);
                }
              while (i < size && buffer[i++] != 0xF7);
              size = i;
            }
        }
      else
        {
          unsigned char b;
          size = 0;
          if (file->data_size > 0)
            lseek (file->fd, file->data_size, SEEK_CUR);
          else
            while (get_byte (file->fd, &b))
              if (b == 0xF7)
                break;
        }
      file->data_size = 0;
      return size;
    }
  return 0;
 error:
  return -1;
}

static int
put_data (midi_file_t * file, unsigned char *buffer, int size)
{
  ck_err(!IS_MIDI_FILE(file));

  if (file->data_size)
    {
      //printf("PUD\n");
      if (buffer && size > 0)
        {
          if (file->data_size > 0)
            {
              //DATA
              if (size < file->data_size)
                file->data_size -= size;
              else
                size = file->data_size;
              ck_err (write (file->fd, buffer, size) != size);
            }
          else
            {
              int i = 0;
              //SYSEX
              while (i < size && buffer[i] != 0xF7)
                ck_err (put_byte (file->fd, buffer[i++]) < 0);
              ck_err (put_byte (file->fd, 0xF7) < 0);
            }
        }
      /*
      else
        {
          unsigned char b;
          size = 0;
          if (file->data_size > 0)
            lseek (file->fd, file->data_size, SEEK_CUR);
          else
            while (b == 0xF7)
              ck_err (put_byte (file->fd, b));
        }
      */
      file->data_size = 0;
      return size;
    }
 error:
  return -1;
}

