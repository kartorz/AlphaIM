##=======================================================
## Makefile for AlphaDict.                                
##                                                        
##
##=======================================================

##=== funtions ===##
# tree -d -L 1  -i

FUN_LOCAL_DIRS = $(shell ls $(1) -lA | grep ^d  | awk '{print $$9}')

include config.mak

DEPS :=

subdirs := tool  lib gui protocol
incs := $(foreach dir, $(subdirs), $(dir)/Makefile.inc)
include $(incs)

SRCS_ALPHAIM += main.cpp      \
			 Application.cpp  \
             Log.cpp          \
             MutexLock.cpp    \
             SpinLock.cpp     \
             ThdCond.cpp      \
             Thread.cpp       \
			 MessageQueue.cpp \
			 SysMessager.cpp  \
			 TaskManager.cpp  \
             Util.cpp         \
			 CharUtil.cpp     \
             TDFParser.cpp    \
             PY.cpp           \
			 IMPreedit.cpp    \
	         X11IMPreedit.cpp \
			 QtIMPreedit.cpp  \
             UsrPhrase.cpp \
             Configure.cpp \
             DBusDaemon.cpp


#ifeq ($(DEBUG), yes)
SRCS_ALPHAIM += Signal.cpp
#endif

SRCS_AIMTOOL += SpinLock.cpp  \
				ThdCond.cpp   \
				Log.cpp       \
                Util.cpp      \
				CharUtil.cpp  \
                TDFParser.cpp

SRCS_GUI-yes += Util.cpp

OBJS_TINYXML := $(SRCS_TINYXML:%.cpp=%.o)
OBJS_INDEXTREE := $(SRCS_INDEXTREE:%.cpp=%.o) 
OBJS_ALPHAIM := $(SRCS_ALPHAIM:%.cpp=%.o)

OBJS_ALPHAIM +=	$(OBJS_PROTOCOL) \
                lib/tinyxml2/libtinyxml2.a \
				lib/indextree/libindextree.a \
				lib/IMdkit/Ximd/Ximd.a

OBJS_AIMTOOL := $(SRCS_AIMTOOL:%.cpp=%.o)
OBJS_AIMTOOL += lib/tinyxml2/libtinyxml2.a \
			    lib/indextree/libindextree.a

OBJS_GUI := $(SRCS_GUI-yes:%.cpp=%.o)

DEPS += $(SRCS_ALPHAIM:%.cpp=%.d)
DEPS += $(SRCS_AIMTOOL:%.cpp=%.d)
DEPS += $(SRCS_GUI-yes:%.cpp=%.d)

INSTALL_TARGETS-yes = install-system install-AlphaIM install-AIMTool install-AIMGui

ifeq ($(QT5), yes)
INSTALL_TARGETS-yes += install-QtPlugin
endif

DIRS = .  \
    tool  \
	lib/indextree \
	lib/kary_tree \
	lib/sqlite3   \
	lib/tinyxml2  \
	gui/gtk       \
    gui/qt        \
	protocol	  \
	protocol/xim  \
	protocol/qt  \
	protocol/qt/plugin

ADDSUFFIXES     = $(foreach suf,$(1),$(addsuffix $(suf),$(2)))
ADD_ALL_DIRS    = $(call ADDSUFFIXES,$(1),$(DIRS))
ADD_ALL_EXESUFS = $(1) $(call ADDSUFFIXES,$(EXESUFS_ALL),$(1))

a11: IMdkit AlphaIM  AIMTool AIMGui QtPlugin

lib/tinyxml2/libtinyxml2.a: $(OBJS_TINYXML)
	$(AR) -r $@  $^

lib/indextree/libindextree.a: $(OBJS_INDEXTREE)
	$(AR) -r $@  $^

IMdkit:
	$(MAKE) -C lib/IMdkit

AlphaIM: $(OBJS_ALPHAIM)
	$(CXX) $^ $(LIBS) -lstdc++  -o $@

AIMTool: $(OBJS_AIMTOOL)
	$(CXX) $^ $(LIBS) -lstdc++ -o $@ 

AIMGui: $(OBJS_GUI)
	$(CXX) $^ $(LIBS) -lstdc++ -o $@ 

QtPlugin:
	$(MAKE) -C  protocol/qt/plugin

#$(QT_PLUGIN): protocol/qt/plugin/main.moc  $(OBJS_QIM)
#	$(CXX) $(OBJS_QIM) $(LIBS)  -lQt5DBus  -shared  -o $@

#%.d : %.cpp
#	$(CC) -MM $< -o $@

%.o : %.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS)  -c $< -o $@

%.o : %.c
	$(CC) $(CFLAGS)  -c $< -o $@

-include $(DEPS)



###### installation / clean / generic rules #######

install: $(INSTALL_TARGETS-yes)

install-dirs:
	$(INSTALL) -d $(BINDIR) $(DATADIR) $(DATADIR)/theme

install-%: % install-dirs
	$(INSTALL) -m 755 $(INSTALLSTRIP) $< $(BINDIR)

install-system: install-dirs
	install -m 0666  system/pinyin-utf8.imdb       "$(DATADIR)/"
	install -m 0666  system/phrase-utf8.imdb       "$(DATADIR)/"
	install -m 0666  system/han-utf8.imdb          "$(DATADIR)/"
	install -m 0755  system/app.png                "$(DATADIR)/"
	cp      -av      system/theme/*                "$(DATADIR)/theme/"
	install -m 0646  system/AlphaIM.desktop        "$(HOME)/.config/autostart/"

install-QtPlugin:
	$(INSTALL) -m 0644 protocol/qt/plugin/$(QT_PLUGIN)  "$(QTDIR)/plugins/platforminputcontexts/$(QT_PLUGIN)"

uninstall:
	rm -f $(BINDIR)/AlphaIM $(BINDIR)/AIMTool
	rm -f $(DATADIR)/theme -r

clean:
#	@find . -name "*.[oadi]" | xargs -I{} rm {}
#	-@rm protocol/qt/plugin/moc_*
#	-@rm protocol/qt/plugin/main.moc
	-rm -f $(call ADD_ALL_DIRS,/*.o /*.d /*.a /*.i /*~)
	-rm -f $(call ADD_ALL_EXESUFS, AlphaIM AIMTool)
	-$(MAKE) -C lib/IMdkit clean
	-$(MAKE) -C protocol/qt/plugin clean

distclean: clean
	-@rm -rf config.*

echo:
#	@echo $(DEPS)
	@echo $(INSTALL_TARGETS-yes)


.PHONY : IMdkit install uninstall clean distclean  echo
