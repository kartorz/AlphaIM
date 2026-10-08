# Build AlphaIM's Qt platform-input-context plugin against WPS's private Qt.
# The headers are supplied in this directory; WPS libraries use the Kso names.
# -------------------------------------------
# https://github.com/kingsoft-wps/qtbase

ROOT := ../../../
QT_SOURCE := $(ROOT)/src/protocol/qt
BUILD_DIR := .
WPS_ROOT ?= /opt/kingsoft/wps-office/office6
QTMAKE ?= $(CURDIR)/bin/qmake
MOC ?= $(dir $(QTMAKE))moc
CXX ?= g++
INSTALL ?= install
PLUGIN := $(BUILD_DIR)/libalphaimplatforminputcontextplugin.so
WPS_PLUGIN_DIR ?= $(WPS_ROOT)/qt/plugins/platforminputcontexts

INCLUDE_ROOT := $(CURDIR)/include
QT_VERSION := 5.12.12
QT_INCLUDES := \
 -I$(INCLUDE_ROOT) \
 -I$(INCLUDE_ROOT)/QtCore/$(QT_VERSION) \
 -I$(INCLUDE_ROOT)/QtCore/$(QT_VERSION)/QtCore \
 -I$(INCLUDE_ROOT)/QtGui/$(QT_VERSION) \
 -I$(INCLUDE_ROOT)/QtGui/$(QT_VERSION)/QtGui \
 -I$(INCLUDE_ROOT)/QtDBus/$(QT_VERSION) \
 -I$(INCLUDE_ROOT)/QtDBus/$(QT_VERSION)/QtDBus \
 -I$(INCLUDE_ROOT)/QtCore \
 -I$(INCLUDE_ROOT)/QtGui \
 -I$(INCLUDE_ROOT)/QtDBus

DEFINES := -DAIM_WPS_QT -DQT_NAMESPACE=kso_qt -DQT_NO_TSLIB -DQT_NO_EXCEPTIONS -D_LARGEFILE64_SOURCE -D_LARGEFILE_SOURCE -DQT_NO_DEBUG -DQT_PLUGIN -D_REENTRANT
CXXFLAGS ?= -O2
CXXFLAGS += -fPIC -std=gnu++11 $(DEFINES) $(QT_INCLUDES) -I$(QT_SOURCE) -I$(BUILD_DIR)
LDFLAGS := -shared -Wl,-z,defs
WPS_LIBS := \
 -Wl,--no-as-needed \
 -Wl,-z,origin \
 -Wl,--disable-new-dtags \
 -Wl,-rpath,'$$ORIGIN/../../lib:$(WPS_ROOT)' \
 $(WPS_ROOT)/libQt5GuiKso.so.5.12.12 \
 $(WPS_ROOT)/libQt5DBusKso.so.5.12.12 \
 $(WPS_ROOT)/libQt5CoreKso.so.5.12.12 \
 -lGL -lpthread

SOURCES := main.cpp qaimplatforminputcontext.cpp qaiminputcontextproxy.cpp qaimproxy.cpp
OBJECTS := $(SOURCES:%.cpp=$(BUILD_DIR)/%.o)
MOC_SOURCES := $(BUILD_DIR)/main.moc $(BUILD_DIR)/moc_qaimplatforminputcontext.cpp $(BUILD_DIR)/moc_qaiminputcontextproxy.cpp $(BUILD_DIR)/moc_qaimproxy.cpp
MOC_OBJECTS := $(patsubst %.cpp,%.o,$(filter %.cpp,$(MOC_SOURCES)))

.PHONY: all clean install check

all: check $(PLUGIN)

check:
	@test -x "$(QTMAKE)" || { echo "Qt qmake is missing: $(QTMAKE)" >&2; exit 1; }
	@test -x "$(MOC)" || { echo "Qt moc is missing: $(MOC)" >&2; exit 1; }
	@test -d "$(WPS_ROOT)" || { echo "WPS directory is missing: $(WPS_ROOT)" >&2; exit 1; }
	@test -f "$(WPS_ROOT)/libQt5GuiKso.so.5.12.12" || { echo "WPS QtGui library is missing" >&2; exit 1; }
	@test -f "$(WPS_ROOT)/libQt5DBusKso.so.5.12.12" || { echo "WPS QtDBus library is missing" >&2; exit 1; }
	@test -f "$(WPS_ROOT)/libQt5CoreKso.so.5.12.12" || { echo "WPS QtCore library is missing" >&2; exit 1; }

$(BUILD_DIR):
	mkdir -p "$@"

$(BUILD_DIR)/%.o: $(QT_SOURCE)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c "$<" -o "$@"
$(BUILD_DIR)/moc_%.o: $(BUILD_DIR)/moc_%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c "$<" -o "$@"

$(BUILD_DIR)/main.o: $(BUILD_DIR)/main.moc


$(BUILD_DIR)/main.moc: $(QT_SOURCE)/main.cpp | $(BUILD_DIR)
	$(MOC) $(QT_INCLUDES) $(DEFINES) "$<" -o "$@"

$(BUILD_DIR)/moc_qaimplatforminputcontext.cpp: $(QT_SOURCE)/qaimplatforminputcontext.h | $(BUILD_DIR)
	$(MOC) $(QT_INCLUDES) $(DEFINES) "$<" -o "$@"

$(BUILD_DIR)/moc_qaiminputcontextproxy.cpp: $(QT_SOURCE)/qaiminputcontextproxy.h | $(BUILD_DIR)
	$(MOC) $(QT_INCLUDES) $(DEFINES) "$<" -o "$@"

$(BUILD_DIR)/moc_qaimproxy.cpp: $(QT_SOURCE)/qaimproxy.h | $(BUILD_DIR)
	$(MOC) $(QT_INCLUDES) $(DEFINES) "$<" -o "$@"

$(PLUGIN): $(OBJECTS) $(MOC_OBJECTS)
	$(CXX) $(LDFLAGS) -o "$@" $^ $(WPS_LIBS)

install: $(PLUGIN)
	$(INSTALL) -d "$(WPS_PLUGIN_DIR)"
	$(INSTALL) -m 0755 "$(PLUGIN)" "$(WPS_PLUGIN_DIR)/"

clean:
	rm -rf "$(BUILD_DIR)"
