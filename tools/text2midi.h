#ifndef _TEXT2MIDI_H_
# define _TEXT2MIDI_H_

extern int line;
struct T2M_s;
typedef struct T2M_s T2M_t;

extern T2M_t *T2M;

//PARSE
int T2M_mmsg_new(T2M_t*, char*, int);
int T2M_mmsg_param_id(T2M_t*, char*, char*);
int T2M_mmsg_param_digit(T2M_t*, char*, int);
int T2M_mmsg_param_datastr(T2M_t*, char*, char*);
int T2M_mmsg_flush(T2M_t*);
int T2M_header_flush(T2M_t*);
int T2M_track_flush(T2M_t*);
int text2timestamp(T2M_t*, char*);

#endif
