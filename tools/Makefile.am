bin_PROGRAMS=midi2text text2midi

midi2text_SOURCE=midi2text.c debug.h misc.h
text2midi_SOURCES=text2midi.c text2midi.h parse.y lex.l debug.h misc.h

AM_YFLAGS =  -d

midi2text_LDADD=../src/libsimili.la
text2midi_LDADD=../src/libsimili.la

#INCLUDES=-I$(top_srcdir)/include
#AM_CFLAGS=-static -Wall -pipe -g
