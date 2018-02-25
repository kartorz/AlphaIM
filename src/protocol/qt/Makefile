empty :=
space := $(empty) $(empty)
.RECIPEPREFIX := $(space)

QT_DEFINES= -DQT_NO_TSLIB -DQT_NO_EXCEPTIONS -D_LARGEFILE64_SOURCE -D_LARGEFILE_SOURCE -DQT_NO_DEBUG -DQT_PLUGIN
QT_CXXFLAGS= -O2 -fPIC $(QT_DEFINES)
QT_PLUGIN=libalphaimplatforminputcontextplugin.so
QT_MAJORS := 6 5
QT_FOUND=
QT_PLUGINS=

-include ../../config.mak

COMMON_SRCS=main.cpp \
    qaimplatforminputcontext.cpp \
    qaiminputcontextproxy.cpp \
    qaimproxy.cpp

MOC_SRCS=moc_qaiminputcontextproxy.cpp \
    moc_qaimproxy.cpp \
    moc_qaimplatforminputcontext.cpp

define qt_pkg
Qt$1Core Qt$1Gui Qt$1Widgets Qt$1DBus
endef

define qt_version
$(shell pkg-config --modversion Qt$1Core 2>/dev/null)
endef

define qt_includes
$(shell pkg-config --cflags $(call qt_pkg,$1) 2>/dev/null)
endef

define qt_libs
$(shell pkg-config --libs $(call qt_pkg,$1) 2>/dev/null)
endef

define qt_include_dir
$(shell pkg-config --variable=includedir Qt$1Core 2>/dev/null)
endef

define qt_private_includes
$(shell \
    include_dir="$(call qt_include_dir,$1)"; \
    version="$(call qt_version,$1)"; \
    private_includes=""; \
    if test -n "$$include_dir" -a -n "$$version"; then \
        for private_dir in \
            "$$include_dir/QtGui/$$version" \
            "$$include_dir/QtGui/$$version/QtGui" \
            "$$include_dir/QtCore/$$version" \
            "$$include_dir/QtCore/$$version/QtCore"; do \
            test -d "$$private_dir" && \
                private_includes="$$private_includes -I$$private_dir"; \
        done; \
    fi; \
    echo "$$private_includes")
endef

define qt_qmake
$(shell \
for q in qmake$1 qmake-qt$1 qmake ; do \
    if command -v $$q >/dev/null 2>&1 ; then \
        v=`$$q -query QT_VERSION 2>/dev/null`; \
        if test "$${v%%.*}" = "$1" ; then \
            echo $$q; \
            exit; \
        fi; \
    fi; \
done)
endef

define qt_tool
$(shell \
q="$(call qt_qmake,$1)"; \
if test -n "$$q"; then \
    d=`$$q -query QT_INSTALL_LIBEXECS 2>/dev/null`; \
    test -x "$$d/$2" && echo "$$d/$2" && exit; \
    d=`$$q -query QT_INSTALL_BINS 2>/dev/null`; \
    test -x "$$d/$2" && echo "$$d/$2" && exit; \
fi; \
command -v $2$1 2>/dev/null || command -v $2 2>/dev/null)
endef

define qt_plugin_dir
$(shell \
q="$(call qt_qmake,$1)"; \
test -n "$$q" && $$q -query QT_INSTALL_PLUGINS 2>/dev/null)
endef

$(foreach qt, $(QT_MAJORS), \
  $(if $(filter yes,$(shell pkg-config --exists $(call qt_pkg,$(qt)) && echo yes)), \
    $(eval QT$(qt)_VERSION := $(call qt_version,$(qt))) \
    $(eval QT$(qt)_INCLUDES := $(call qt_includes,$(qt))) \
    $(eval QT$(qt)_INCLUDES += $(call qt_private_includes,$(qt))) \
    $(eval QT$(qt)_LIBS := $(call qt_libs,$(qt))) \
    $(eval QT$(qt)_MOC := $(call qt_tool,$(qt),moc)) \
    $(eval QT$(qt)_UIC := $(call qt_tool,$(qt),uic)) \
    $(eval QT$(qt)_RCC := $(call qt_tool,$(qt),rcc)) \
    $(eval QT$(qt)_PLUGIN_DIR := $(call qt_plugin_dir,$(qt))) \
    $(eval QT$(qt)_BUILDDIR := build/qt$(qt)) \
    $(eval QT$(qt)_PLUGIN := $$(QT$(qt)_BUILDDIR)/$$(QT_PLUGIN)) \
    $(eval QT_PLUGINS += $(QT$(qt)_PLUGIN)) \
    $(eval QT_FOUND += $(qt)) \
  ) \
)

$(foreach qt,$(QT_FOUND), \
    $(eval QT$(qt)_MOC_SRCS := \
        $(addprefix $$(QT$(qt)_BUILDDIR)/,$(MOC_SRCS))) \
    $(eval QT$(qt)_OBJS := \
        $(COMMON_SRCS:%.cpp=$$(QT$(qt)_BUILDDIR)/%.o) \
        $$(QT$(qt)_MOC_SRCS:%.cpp=%.o)) \
)

$(info Qt versions found: $(QT_FOUND), build: $(QT_PLUGINS))
#$(foreach v,$(QT_FOUND), \
#  $(info Qt$(v): version=$(QT$(v)_VERSION)) \
#  $(info Qt$(v): moc=$(QT$(v)_MOC)) \
#  $(info Qt$(v): uic=$(QT$(v)_UIC)) \
#  $(info Qt$(v): rcc=$(QT$(v)_RCC)) \
#  $(info Qt$(v): plugin_dir=$(QT$(v)_PLUGIN_DIR)) \
#)

a11: $(QT_PLUGINS)

$(QT5_BUILDDIR) $(QT6_BUILDDIR):
    @mkdir -p $@

$(QT5_BUILDDIR)/main.moc: main.cpp | $(QT5_BUILDDIR)
    $(QT5_MOC) $(QT5_INCLUDES) main.cpp -o $@
$(QT5_BUILDDIR)/moc_qaiminputcontextproxy.cpp: qaiminputcontextproxy.h | $(QT5_BUILDDIR)
    $(QT5_MOC) $(QT5_INCLUDES) qaiminputcontextproxy.h -o $@
$(QT5_BUILDDIR)/moc_qaimproxy.cpp: qaimproxy.h | $(QT5_BUILDDIR)
    $(QT5_MOC) $(QT5_INCLUDES) qaimproxy.h -o $@
$(QT5_BUILDDIR)/moc_qaimplatforminputcontext.cpp: qaimplatforminputcontext.h | $(QT5_BUILDDIR)
    $(QT5_MOC) $(QT5_INCLUDES) qaimplatforminputcontext.h -o $@
$(QT5_BUILDDIR)/%.o: %.cpp $(QT5_BUILDDIR)/main.moc | $(QT5_BUILDDIR)
    $(CXX) $(QT5_INCLUDES) -I$(QT5_BUILDDIR) $(QT_CXXFLAGS) -c $< -o $@
$(QT5_BUILDDIR)/%.o: $(QT5_BUILDDIR)/%.cpp | $(QT5_BUILDDIR)
    $(CXX) $(QT5_INCLUDES) -I$(QT5_BUILDDIR) $(QT_CXXFLAGS) -c $< -o $@
$(QT5_PLUGIN): $(QT5_OBJS)
    $(CXX) $(QT5_OBJS) $(QT5LIBS) -shared -o $@

$(QT6_BUILDDIR)/main.moc: main.cpp | $(QT6_BUILDDIR)
    $(QT6_MOC) $(QT6_INCLUDES) main.cpp -o $@
$(QT6_BUILDDIR)/moc_qaiminputcontextproxy.cpp: qaiminputcontextproxy.h | $(QT6_BUILDDIR)
    $(QT6_MOC) $(QT6_INCLUDES) qaiminputcontextproxy.h -o $@
$(QT6_BUILDDIR)/moc_qaimproxy.cpp: qaimproxy.h | $(QT6_BUILDDIR)
    $(QT6_MOC) $(QT6_INCLUDES) qaimproxy.h -o $@
$(QT6_BUILDDIR)/moc_qaimplatforminputcontext.cpp: qaimplatforminputcontext.h | $(QT6_BUILDDIR)
    $(QT6_MOC) $(QT6_INCLUDES) qaimplatforminputcontext.h -o $@
$(QT6_BUILDDIR)/%.o: %.cpp $(QT6_BUILDDIR)/main.moc | $(QT6_BUILDDIR)
    $(CXX) $(QT6_INCLUDES) -I$(QT6_BUILDDIR) $(QT_CXXFLAGS) -c $< -o $@
$(QT6_BUILDDIR)/%.o: $(QT6_BUILDDIR)/%.cpp | $(QT6_BUILDDIR)
    $(CXX) $(QT6_INCLUDES) -I$(QT6_BUILDDIR) $(QT_CXXFLAGS) -c $< -o $@
$(QT6_PLUGIN): $(QT6_OBJS)
    $(CXX) $(QT6_OBJS) $(QT6LIBS) -shared -o $@

clean:
    -@rm -rf build
    -@rm -f $(QT_PLUGIN) moc_* main.moc
    -@find . -name "*.[oadi]" -exec rm -f {} +

install: a11
    $(foreach qt,$(QT_FOUND), \
        $(INSTALL) -d "$(DESTDIR)$(QT$(qt)_PLUGIN_DIR)/platforminputcontexts"; \
        $(INSTALL) -m 0644 "$(QT$(qt)_PLUGIN)" \
            "$(DESTDIR)$(QT$(qt)_PLUGIN_DIR)/platforminputcontexts/$(QT_PLUGIN)"; \
    )

uninstall:
    $(foreach qt,$(QT_FOUND), \
        rm -f "$(DESTDIR)$(QT$(qt)_PLUGIN_DIR)/platforminputcontexts/$(QT_PLUGIN)"; \
    )

.PHONY: a11 clean install uninstall
