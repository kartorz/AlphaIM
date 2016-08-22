# Query the default goal.

ifeq ($(SharedLibXimd),)
SharedLibXimd := HasSharedLibraries
endif

#ifeq ($(NormalLibXimd),)
#NormalLibXimd := (!SharedLibXimd | ForceNormalLib)
#endif

ifeq ($(DebugLibXimd),)
DebugLibXimd :=	NO	/* debugged Ximd library */
endif

ifeq ($(ProfileLibXimd),)
ProfileLibXimd := NO	/* profiled Ximd library */
endif

ifeq ($(SharedLibXimd),)
REQUIREDLIBS := $(XLIBONLY)
endif

DoNormalLib := $(NormalLibXimd)
DoSharedLib := $(SharedLibXimd)
DoDebugLib := $(DebugLibXimd)
DoProfileLib := $(ProfileLibXimd)
LibName := Ximd
SoRev := SOXIMDREV
IncSubdir := Ximd

BuildXi18n := YES
BuildXimp := YES

SOXIMDREV = 1.0
CONN_DEFINES = ConnectionFlags
EXTRA_DEFINES = -DUse_Ximp -DNeedVarargsPrototypes=1
XTRANS_XIM_DEFINES = -DXIM_t -DTRANS_SERVER
#if HasBSD44Sockets
#   SOCK_DEFINES = -DBSD44SOCKETS
#endif
EXTRA_INCLUDES :=  -IIMCore -IXi18n -IXimp -I/usr/include/X11/Xtrans
CFLAGS := $(ALCFLAGS) $(XTRANS_XIM_DEFINES) -fPIC $(EXTRA_DEFINES)  $(EXTRA_INCLUDES)

X11_INC_DIR := /usr/include/X11
ADDPREFIX   = $(foreach pre,$(1),$(addprefix $(pre)/, $(2)))

COREHDRS = IMdkit.h

CORESRCS = \
   IMConn.c \
   IMValues.c \
   IMMethod.c

COREOBJS = \
   IMConn.o \
   IMValues.o \
   IMMethod.o

COREHDRS := $(call ADDPREFIX, IMCore, $(COREHDRS))
CORESRCS := $(call ADDPREFIX, IMCore, $(CORESRCS))
COREOBJS := $(call ADDPREFIX, IMCore, $(COREOBJS))

ifeq ($(BuildXi18n), YES)

I18NHDRS = Xi18n.h

I18NSRCS = \
   FrameMgr.c \
   i18nIMProto.c \
   i18nAttr.c \
   i18nMethod.c \
   i18nUtil.c \
   i18nPtHdr.c \
   i18nIc.c \
   i18nClbk.c \
   i18nX.c \
   i18nTr.c \
   i18ntrans.c

I18NOBJS = \
   FrameMgr.o \
   i18nIMProto.o \
   i18nAttr.o \
   i18nMethod.o \
   i18nUtil.o \
   i18nPtHdr.o \
   i18nIc.o \
   i18nClbk.o \
   i18nX.o \
   i18nTr.o \
   i18ntrans.o

I18NHDRS := $(call ADDPREFIX, Xi18n, $(I18NHDRS))
I18NSRCS := $(call ADDPREFIX, Xi18n, $(I18NSRCS))
I18NOBJS := $(call ADDPREFIX, Xi18n, $(I18NOBJS))

endif

ifeq ($(BuildXimp), YES)

IMPHDRS = XimpData.h

IMPSRCS = \
	XimpMethods.c \
	XimpHandler.c \
	XimpProc.c \
	XimpAttr.c \
	XimpFlt.c \
	XimpClbk.c \
	XimpUtls.c

IMPOBJS = \
	XimpMethods.o \
	XimpHandler.o \
	XimpProc.o \
	XimpAttr.o \
	XimpFlt.o \
	XimpClbk.o \
	XimpUtls.o

IMPHDRS := $(call ADDPREFIX, Ximp, $(IMPHDRS))
IMPSRCS := $(call ADDPREFIX, Ximp, $(IMPSRCS))
IMPOBJS := $(call ADDPREFIX, Ximp, $(IMPOBJS))
endif

HEADERS := $(COREHDRS) $(I18NHDRS) $(IMPHDRS)
SRCS := $(CORESRCS) $(I18NSRCS) $(IMPSRCS)
OBJS := $(I18NOBJS) $(IMPOBJS) $(COREOBJS)

$(LibName).a : $(OBJS)
	$(AR) -r $@  $^
	@if [ ! -d ../Ximd ] ; then mkdir ../Ximd; fi

	install -m 0666  IMCore/*.h             "../Ximd/"
	install -m 0666  Xi18n/*.h              "../Ximd/"
	install -m 0666  Ximp/*.h               "../Ximd/"
	install -m 0666  Ximd.a                 "../Ximd/Ximd.a"

#Xi18n/i18ntrans.o : Xi18n/i18ntrans.c
#	$(CC) $(CFLAGS) $(XTRANS_XIM_DEFINES) -c $< -o $@

#Xi18n/i18ntrans.c :
#	ln -s ${X11_INC_DIR}/Xtrans/transport.c  Xi18n/i18ntrans.c
#	ln -s ${X11_INC_DIR}/Xtrans/Xtransint.h  Xi18n/Xtransint.h
#	ln -s ${X11_INC_DIR}/Xtrans/Xtrans.h     Xi18n/Xtrans.h

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@if [ -L Xi18n/Xtransint.h ] ; then unlink Xi18n/Xtransint.h; fi
	@if [ -L Xi18n/Xtrans.h    ] ; then unlink Xi18n/Xtrans.h; fi
	@if [ -L Xi18n/i18ntrans.c ] ; then unlink Xi18n/i18ntrans.c; fi

all: $(libName).a clean

.PHONY: clean
