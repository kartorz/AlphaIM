IMDLIB =  -lX11
DEPLIBS = $(DEPXONLYLIB)
INCLUDES = -I.. -I../Ximd
LOCAL_LIBRARIES = $(IMDLIB) $(XONLYLIB)
SRCS = sampleIM.c IC.c
OBJS = sampleIM.o IC.o
CDEBUGFLAGS = -g -DDEBUG
CC = gcc

PROGRAM : sampleIM i18n_input

sampleIM: $(OBJS) ../Ximd/Ximd.a
	$(CC) $(CDEBUGFLAGS) $^  $(LOCAL_LIBRARIES)  -o $@

%.o: %.c
	$(CC) $(CDEBUGFLAGS) $(INCLUDES) -c $<  -o $@

i18n_input: i18n_input.c
	$(CC) -g  -o  $@  i18n_input.c   libX11-xcb.a libX11.a  -lxcb
#	$(CC)  -o $@ i18n_input.c   -lX11

