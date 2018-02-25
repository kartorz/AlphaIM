##=======================================================
## Makefile for AlphaDict.
##
##=======================================================

empty :=
space := $(empty) $(empty)
.RECIPEPREFIX := $(space)

##=== funtions ===##
# tree -d -L 1  -i

FUN_LOCAL_DIRS = $(shell ls $(1) -lA | grep ^d | awk '{print $$9}')

include config.mak

DEPS :=

SYSTEMDIR := ../system
DESKTOP_FILE := ../debian/alphaim.desktop
AUTOSTARTDIR := $(DESTDIR)/etc/xdg/autostart
IM_CONFIG_DATA_DIR := $(DESTDIR)/usr/share/im-config/data

subdirs := tool lib gui protocol
incs := $(foreach dir,$(subdirs),$(dir)/Makefile.inc)
include $(incs)

SRCS_ALPHAIMD += main.cpp \
    Application.cpp \
    Log.cpp \
    MutexLock.cpp \
    SpinLock.cpp \
    ThdCond.cpp \
    Thread.cpp \
    MessageQueue.cpp \
    SysMessager.cpp \
    TaskManager.cpp \
    Util.cpp \
    CharUtil.cpp \
    TDFParser.cpp \
    PY.cpp \
    IMPreedit.cpp \
    X11IMPreedit.cpp \
    QtIMPreedit.cpp \
    GtkIMPreedit.cpp \
    UsrPhrase.cpp \
    Configure.cpp \
    DBusDaemon.cpp

#ifeq ($(DEBUG), yes)
SRCS_ALPHAIMD += Signal.cpp
#endif

SRCS_AIMCTL += SpinLock.cpp \
    ThdCond.cpp \
    Log.cpp \
    Util.cpp \
    CharUtil.cpp \
    TDFParser.cpp

SRCS_GUI-$(GUI) += Util.cpp

OBJS_TINYXML := $(SRCS_TINYXML:%.cpp=%.o)
OBJS_INDEXTREE := $(SRCS_INDEXTREE:%.cpp=%.o)
OBJS_ALPHAIMD := $(SRCS_ALPHAIMD:%.cpp=%.o)

OBJS_ALPHAIMD += $(OBJS_PROTOCOL) \
    lib/tinyxml2/libtinyxml2.a \
    lib/indextree/libindextree.a \
    lib/IMdkit/Ximd/Ximd.a

OBJS_AIMCTL := $(SRCS_AIMCTL:%.cpp=%.o)
OBJS_AIMCTL += lib/tinyxml2/libtinyxml2.a \
    lib/indextree/libindextree.a

OBJS_GUI_CPP := $(SRCS_GUI-$(GUI):%.c=%.cpp)
OBJS_GUI := $(OBJS_GUI_CPP:%.cpp=%.o)

DEPS += $(SRCS_ALPHAIMD:%.cpp=%.d)
DEPS += $(SRCS_AIMCTL:%.cpp=%.d)
DEPS += $(OBJS_GUI_CPP:%.cpp=%.d)

INSTALL_TARGETS-yes = install-alphaimd install-aimctl install-alphaim-ui install-system
INSTALL_TARGETS-yes += install-aim-qt install-aim-gtk

DIRS = . \
    tool \
    lib/indextree \
    lib/kary_tree \
    lib/sqlite3 \
    lib/tinyxml2 \
    gui/gtk \
    gui/qt \
    protocol \
    protocol/xim \
    protocol/qt \
    protocol/gtk

ADDSUFFIXES = $(foreach suf,$(1),$(addsuffix $(suf),$(2)))
ADD_ALL_DIRS = $(call ADDSUFFIXES,$(1),$(DIRS))

a11: IMdkit alphaimd aimctl alphaim-ui aim-gtk aim-qt

lib/tinyxml2/libtinyxml2.a: $(OBJS_TINYXML)
 $(AR) -r $@ $^

lib/indextree/libindextree.a: $(OBJS_INDEXTREE)
 $(AR) -r $@ $^

IMdkit:
 $(MAKE) -C lib/IMdkit

alphaimd: $(OBJS_ALPHAIMD)
 $(CXX) $^ $(LIBS) -lstdc++ -o $@

aimctl: $(OBJS_AIMCTL)
 $(CXX) $^ $(LIBS) -lstdc++ -o $@

alphaim-ui: $(OBJS_GUI)
 $(CXX) $^ $(LIBS) -lstdc++ -o $@

aim-qt:
 $(MAKE) -C protocol/qt

aim-gtk:
 $(MAKE) -C protocol/gtk

gui/gtk/resource.c: gui/gtk/aimapp.gresource.xml gui/gtk/menu.ui
 glib-compile-resources gui/gtk/aimapp.gresource.xml --target=gui/gtk/resource.c --generate-source

#$(QT_PLUGIN): protocol/qt/main.moc $(OBJS_QIM)
# $(CXX) $(OBJS_QIM) $(LIBS) -lQt5DBus -shared -o $@

%.o: %.cpp
 $(CXX) $(CFLAGS) $(CXXFLAGS) -c $< -o $@

%.o: %.c
 $(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

###### installation / clean / generic rules #######

install: $(INSTALL_TARGETS-yes)

install-dirs:
 $(INSTALL) -d $(BINDIR) $(DATADIR) $(DATADIR)/theme

install-%: % install-dirs
 $(INSTALL) -m 755 $(INSTALLSTRIP) $< $(BINDIR)

install-system: install-dirs
 $(INSTALL) -m 0755 "$(SYSTEMDIR)/run-alphaim.sh"   "$(BINDIR)/"
 $(INSTALL) -m 0644 "$(SYSTEMDIR)/pinyin-utf8.imdb" "$(DATADIR)/"
 $(INSTALL) -m 0644 "$(SYSTEMDIR)/phrase-utf8.imdb" "$(DATADIR)/"
 $(INSTALL) -m 0644 "$(SYSTEMDIR)/han-utf8.imdb"    "$(DATADIR)/"
 $(INSTALL) -m 0644 "$(SYSTEMDIR)/app.png"          "$(DATADIR)/"
 $(INSTALL) -m 0644 "$(SYSTEMDIR)/configure.xml.in" "$(DATADIR)/"
 $(INSTALL) -m 0644 "$(SYSTEMDIR)/alphaim.desktop"  "$(AUTOSTARTDIR)/alphaim.desktop"
 $(INSTALL) -m 0644 "$(SYSTEMDIR)/im-config/28_alphaim.conf" "$(IM_CONFIG_DATA_DIR)/28_alphaim.conf"
 $(INSTALL) -m 0644 "$(SYSTEMDIR)/im-config/28_alphaim.rc" "$(IM_CONFIG_DATA_DIR)/28_alphaim.rc"
 cp -a "$(SYSTEMDIR)/theme/." "$(DATADIR)/theme/"

install-aim-qt: aim-qt
 $(MAKE) -C protocol/qt install DESTDIR=$(DESTDIR)

install-aim-gtk: aim-gtk
 $(MAKE) -C protocol/gtk install DESTDIR=$(DESTDIR)

uninstall:
 rm -f $(BINDIR)/alphaimd $(BINDIR)/aimctl $(BINDIR)/alphaim-ui
 rm -f "$(AUTOSTARTDIR)/alphaim.desktop"
 rm -f "$(IM_CONFIG_DATA_DIR)/28_alphaim.conf" "$(IM_CONFIG_DATA_DIR)/28_alphaim.rc"
 rm -rf "$(DATADIR)"
 $(MAKE) -C protocol/gtk uninstall DESTDIR=$(DESTDIR)
 $(MAKE) -C protocol/qt uninstall DESTDIR=$(DESTDIR)

clean:
 -rm -f $(call ADD_ALL_DIRS,/*.o /*.d /*.a /*.i /*~)
 -rm -f  alphaimd aimctl alphaim-ui
 -$(MAKE) -C lib/IMdkit clean
 -$(MAKE) -C protocol/qt clean
 -$(MAKE) -C protocol/gtk clean

distclean: clean
 -@rm -rf config.*

.PHONY: IMdkit aim-qt aim-gtk install install-dirs install-system install-aim-qt install-aim-gtk uninstall clean distclean
