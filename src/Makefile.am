lib_LTLIBRARIES = libsimili.la

libsimiliincludedir = $(includedir)
libsimiliinclude_HEADERS=simili.h

libsimili_la_SOURCES=list.c list.h midi.c midi.h \
           midi_alsa.c midi_alsa.h midi_buffer.c midi_buffer.h \
           midi_file.c midi_file.h midi_mux.c midi_mux.h \
	   midi_pipe.c midi_pipe.h midi_port.c midi_port.h \
	   midi_rtp.c midi_rtp.h misc.h obj.c obj.h

libsimili_la_LDFLAGS = -no-undefined -release $(LT_RELEASE) \
	@ALSA_LIBS@

libsimili_la_CFLAGS = @ALSA_CFLAGS@
