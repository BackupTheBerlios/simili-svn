/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 c-file-style: "gnu" -*- */
/*
 *  Copyright (C) 2003 et droits d'auteur Frederic Motte
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef _MIDI_H
#define _MIDI_H
//http://www.gamasutra.com/resource_guide/20010515/patterson_02.htm

//LEX:
//midi interface: file | port | network | ...
//midi message: note on | note off | ctrl | system | ...
//midi data: data associated to a midi message: sysex | track name | ...

//TYPEDEFS
typedef struct obj_s obj_t;
typedef struct obj_class_s obj_c;
typedef struct midi_s midi_t;//General type for midi interface used by applications
typedef struct midi_class_s midi_c;
typedef struct midi_msg_s midi_msg_t;//General data handled by libmidi
typedef enum midi_flags_e midi_flags_t;//Flags used to defined midi interface properties

//FIXME: Change all get and put by read and write
//METHODES, use the macro instead (midi_*)
typedef int obj_destroy_t(obj_t*);
typedef int midi_get_msg_t (midi_t*, midi_msg_t*);//Method type used to write a message
typedef int midi_put_msg_t (midi_t*, midi_msg_t*);//Method type used to read a message
typedef int midi_get_data_t (midi_t*, unsigned char*, int);//Method type used to write data associated to a message
typedef int midi_put_data_t (midi_t*, unsigned char*, int);//Method type used to read data associated to a message
typedef int midi_timestamp_t (midi_t*, int);//Method type used to set the current timestamp of a midi interface when reading an asyncronious midi interface (typicaly files)


int dummy_int();


//ENUM
enum midi_flags_e
  {
    /* general */
    MIDI_FLAG_RT        = 1 << 8,//Syncronious reading and writing, imply locking read and write
    MIDI_FLAG_GET       = 1 << 9,//Set the interface read only
    MIDI_FLAG_PUT       = 1 << 10,//Set the interface write only
    //event 0xFF is META event instead of RESET
    MIDI_FLAG_GET_FILE  = 1 << 11,//What that ?
    MIDI_FLAG_PUT_FILE  = 1 << 12,//What that ?
    MIDI_FLAG_TB_LOCAL  = 1 << 13,//What that ?
    MIDI_FLAG_TB_UPDATE = 1 << 14,//What that ?
    MIDI_FLAG_NONBLOCK  = 1 << 15,//What that ?
    /* port */
    MIDI_FLAG_TICK = 1 << 16,
    /* file */
    MIDI_FLAG_MULTI_TRACK = 1 << 16,//format 0 or 1 of midi file, curently all are multi track files and can have only one track
    MIDI_FLAG_CREATE = 1 << 17,//Ask for creating the file and check it doesnt exist
  };


//STRUCT
struct obj_class_s
{
  const char *name;
  obj_destroy_t *destroy;
  obj_c *ancestor;
};

struct obj_s
{
  obj_c *class;
};

struct midi_class_s
{
  obj_c obj;
  midi_put_msg_t *put_msg;
  midi_get_msg_t *get_msg;
  midi_put_data_t *put_data;
  midi_get_data_t *get_data;
  midi_timestamp_t *timestamp;
};


struct midi_s
{
  obj_t obj;
  midi_flags_t flags;  
};


#define MIDI_MSG_SIZE 8


struct midi_msg_s
{
  unsigned char data[MIDI_MSG_SIZE];
  int timestamp;
};


#define MIDI_FLAG_DRIVER_MASK 0x0F
#define MIDI_FLAG_DRIVER(midi) ((midi)->flags & MIDI_FLAG_DRIVER_MASK)


//MIDI SPEC dependent
enum midi_msg_type_e//Type of midi message get with MIDI_MSG_TYPE(mmsg)
  {
    MIDI_NOTE_OFF   = 0x80,
    MIDI_NOTE_ON    = 0x90,
    MIDI_KEY_TOUCH  = 0xA0,
    MIDI_CONTROL    = 0xB0,
    MIDI_PROG_CHNG  = 0xC0,
    MIDI_CHAN_TOUCH = 0xD0,
    MIDI_PITCHBEND  = 0xE0,
    MIDI_SYSTEM     = 0xF0,//see enum midi_msg_sys_type_e
  };


enum midi_msg_sys_type_e//SubType of midi message of type MIDI_SYSTEM
  {
    /* -MIDI SysComon Messages */
    MIDI_SYS_EX = 0xF0,
    MIDI_SYS_EX0 = 0xF0,
    MIDI_SYS_SONG_POS_POINTER = 0xF2,
    MIDI_SYS_SONG_SELECT = 0xF3,
    MIDI_SYS_TUNE_REQUEST = 0xF6,
    MIDI_SYS_EX7 = 0xF7,
    MIDI_SYS_TIMING_CLOCK = 0xF8,
    MIDI_SYS_START = 0xFA,
    MIDI_SYS_CONTINUE = 0xFB,
    MIDI_SYS_STOP = 0xFC,
    MIDI_SYS_ACTIVE_SENSING = 0xFE,
    MIDI_SYS_RESET = 0xFF,	//On wire let say double 0xFF in a mmsg
    MIDI_SYS_META = 0xFF,	//On file: see enum midi_msg_sys_meta_type_e
  };


enum midi_msg_sys_meta_type_e//SubSubType of midi message of type MIDI_SYSTEM and SubType MIDI_SYS_META
  {
    MIDI_SYS_META_TEXT       = 0x00,
    MIDI_SYS_META_TEXTEVENT  = 0x01,
    MIDI_SYS_META_COPYRIGHT  = 0x02,
    MIDI_SYS_META_TRACKNAME  = 0x03,
    MIDI_SYS_META_INSTRNAME  = 0x04,
    MIDI_SYS_META_LIRIKS     = 0x05,
    MIDI_SYS_META_MARK       = 0x06,
    MIDI_SYS_META_SPECIAL    = 0x07,
    MIDI_SYS_META_MIDIPORT   = 0x21,
    MIDI_SYS_META_CHANNEL    = 0x20,
    MIDI_SYS_META_ENDOFTRACK = 0x2F,
    MIDI_SYS_META_TEMPO      = 0x51,
    MIDI_SYS_META_OFFSMPTE   = 0x54,
    MIDI_SYS_META_METRONOME  = 0x58,
    MIDI_SYS_META_KEY        = 0x59,
    MIDI_SYS_META_PROPRIO    = 0x7F,
    MIDI_SYS_META_RESET      = 0xFF,//Addition to have *reset* AND *meta*
  };


#define MIDI_STATUS_MASK 0x80
#define MIDI_TYPE_MASK   0xF0
#define MIDI_CHAN_MASK   0x0F
#define MIDI_DATA_MASK   0x7F

//Macro to set a midi message as a "note on"
#define MIDI_MSG_NOTE_ON(msg, chan, note, velo) \
  do { \
    MIDI_MSG_SET_TYPE((msg), MIDI_NOTE_ON); \
    MIDI_MSG_SET_CHAN((msg), (chan)); \
    MIDI_MSG_SET_NOTE((msg), (note)); \
    MIDI_MSG_SET_VELO((msg), (velo)); \
  } while(0)

//Macro to set a midi message as a "note off
#define MIDI_MSG_NOTE_OFF(msg, chan, note, velo) \
  do { \
    MIDI_MSG_SET_TYPE((msg), MIDI_NOTE_OFF); \
    MIDI_MSG_SET_CHAN((msg), (chan)); \
    MIDI_MSG_SET_NOTE((msg), (note)); \
    MIDI_MSG_SET_VELO((msg), (velo)); \
  } while(0)

//Macro to set a midi message as a "control"
#define MIDI_MSG_CONTROL(msg, chan, ctrl, value) \
  do { \
    MIDI_MSG_SET_TYPE((msg), MIDI_CTRL); \
    MIDI_MSG_SET_CHAN((msg), (chan)); \
    MIDI_MSG_SET_CTRL((msg), (ctrl)); \
    MIDI_MSG_SET_VALUE((msg), (value)); \
  } while(0)

//Miscelaneous macros to get midi messages informations
#define MIDI_MSG_STATUS(msg) ((msg)->data[0])
#define MIDI_MSG_TYPE(msg)   ((msg)->data[0] & MIDI_TYPE_MASK)
#define MIDI_MSG_CHAN(msg)   ((msg)->data[0] & MIDI_CHAN_MASK)
#define MIDI_MSG_NOTE(msg)   ((msg)->data[1])
#define MIDI_MSG_VELO(msg)   ((msg)->data[2])
#define MIDI_MSG_CTRL(msg)   ((msg)->data[1])
#define MIDI_MSG_PRGM(msg)   ((msg)->data[1])
#define MIDI_MSG_VALUE(msg)  ((msg)->data[2])
#define MIDI_MSG_DATA1(msg)  ((msg)->data[1])
#define MIDI_MSG_DATA2(msg)  ((msg)->data[2])
#define MIDI_MSG_PITCH(msg)  ((msg)->data[1] + ((msg)->data[2] << 7))

#define MIDI_MSG_SET_STATUS(msg, status) ((msg)->data[0] = status)

#define MIDI_MSG_SET_TYPE(msg,type)\
 ((msg)->data[0]=(MIDI_MSG_CHAN(msg) | ((type)&0xF0)))

#define MIDI_MSG_SET_CHAN(msg, chan)\
 ((msg)->data[0]=(MIDI_MSG_TYPE(msg) | ((chan)&0x0F)))

#define MIDI_MSG_SET_NOTE(msg, note) ((msg)->data[1] = ((note)&0x7F))
#define MIDI_MSG_SET_VELO(msg, velo) ((msg)->data[2] = ((velo)&0x7F))
#define MIDI_MSG_SET_CTRL(msg, ctrl) ((msg)->data[1] = ((ctrl)&0x7F))
#define MIDI_MSG_SET_PRGM(msg, prgm) ((msg)->data[1] = ((prgm)&0x7F))
#define MIDI_MSG_SET_VALUE(msg, value) ((msg)->data[2] = ((value)&0x7F))
#define MIDI_MSG_SET_DATA1(msg, data1) ((msg)->data[1] = ((data1)&0x7F))
#define MIDI_MSG_SET_DATA2(msg, data2) ((msg)->data[2] = ((data2)&0x7F))
#define MIDI_MSG_SET_PITCH(msg, pitch)\
do{\
  (msg)->data[1]=(pitch)&0x7F;\
  ((msg)->data[2]=((pitch)>>7)&0x7F);\
}while(0)

#define MIDI_MSG_NULL(msg)   ((msg)->data[0] == 0x00)
#define MIDI_MSG_EOF(msg)    ((msg)->data[0] == 0x00 && (msg)->timestamp == -1)
#define MIDI_MSG_EOT(msg)    ((msg)->data[0] == 0xFF && (msg)->data[1] == 0x2F)

#define MIDI_MSG_DATA(msg)   (((msg)->data[0] == 0xFF && (msg)->data[1] && ((msg)->data[1] < 8)) || ((msg)->data[0] == 0xF0))

#define MIDI_MSG_TEMPO(msg)  (((msg)->data[3] << 16) | ((msg)->data[4] << 8) | ((msg)->data[5]))

//Create midi objects
extern midi_t *midi_create(char const *url, midi_flags_t flags);
extern midi_t *midi_file_create (char const *file_name, midi_flags_t flags);//Constructor for a midi file interface
extern midi_t *midi_alsa_create (char const *port_name, midi_flags_t flags);//Constructor for a midi alsa interface
extern midi_t *midi_mux_create (int n_voices, midi_flags_t flags);//Constructor for a midi multiplexer interface
extern midi_t *midi_pipe_create (midi_flags_t flags);//Constructor for a midi pipe interface
//Not implemented ?
extern midi_t *midi_oss_create (char *file_name, midi_flags_t flags);//Constructor for a midi oss port interface
extern midi_t *midi_rtp_create (char *host, int rtp_potr, int rtcp_port, midi_flags_t flags);//Constructor for a midi rtp interface
extern midi_t *midi_fd_create (char *name, midi_flags_t flags);//nada

//generic functions
extern int midi_put_msg (midi_t *midi, midi_msg_t *msg);
#define midi_get_msg(midi, msg) (((midi_c*)midi->obj.class)->get_msg(midi, msg))
#define midi_put_msg(midi, msg) (((midi_c*)midi->obj.class)->put_msg(midi, msg))
//TEST
#define midi_timestamp(midi, ts) (((midi_c*)midi->obj.class)->timestamp(midi, ts))
#define midi_destroy(obj) (((obj_t*)obj)->class->destroy((obj_t*)obj))
//data transmitions are not RT
#define midi_get_data(midi, data, size) (((midi_c*)midi->obj.class)->get_data(midi, data, size))
#define midi_put_data(midi, data, size) (((midi_c*)midi->obj.class)->put_data(midi, data, size))

//extern int midi_close (midi_t * midi);
//global functions
extern int midi_set_ppqn (midi_t *, int);
extern int midi_get_ppqn (midi_t *);
extern int midi_set_tempo (midi_t *, int);
extern int midi_get_tempo (midi_t *);
//specialized
extern int midi_file_next_track(midi_t *file);
extern int midi_file_set_tpqn (midi_t * file, int tpqn);
extern int midi_file_get_tpqn (midi_t * file);
extern int midi_file_get_nb_tracks (midi_t * file);
extern midi_t *midi_mux_get_voice(midi_t *mux, int n_voices, midi_flags_t flags);


void midi_msg_print (midi_msg_t * msg);


#endif
