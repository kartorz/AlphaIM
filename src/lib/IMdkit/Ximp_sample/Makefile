DEPLIBS := -lX11
INCLUDES := -I.. -I../Ximd
SRCS := IC.c sampleIM.c 
OBJS := IC.o sampleIM.o
CDEBUGFLAGS := -g
PROGRAM := sampleIM

sampleIM: $(OBJS) ../Ximd/Ximd.a
	$(CC) $(CDEBUGFLAGS) $^  $(DEPLIBS)  -o $@

%.o: %.c
	$(CC) $(CDEBUGFLAGS) $(INCLUDES) -c $<  -o $@
