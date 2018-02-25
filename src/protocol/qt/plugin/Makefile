QT_DEFINES= -DQT_NO_TSLIB -DQT_NO_EXCEPTIONS -D_LARGEFILE64_SOURCE -D_LARGEFILE_SOURCE -DQT_NO_DEBUG -DQT_PLUGIN
QT_CXXFLAGS= -O2 -fPIC $(QT_DEFINES)
#QT_PLUGIN= libalphaimplatforminputcontextplugin.so

#QTINCLUDES = -I/usr/include/x86_64-linux-gnu/qt5 -I/usr/include/x86_64-linux-gnu/qt5/QtWidgets -I/usr/include/x86_64-linux-gnu/qt5/QtGui  -I/usr/include/x86_64-linux-gnu/qt5/QtCore  -I/usr/include/x86_64-linux-gnu/qt5/QtDBus -I/usr/include/x86_64-linux-gnu/qt5/QtGui/5.5.1/QtGui 
#QTLIBS = -L/usr/lib/x86_64-linux-gnu -lQt5Core -lQt5Gui -lQt5Widgets -L/usr/lib/x86_64-linux-gnu -lQt5X11Extras  -lX11

#MOC="/usr/lib/x86_64-linux-gnu/qt5/bin/moc"

include ../../../config.mak

SRCS=

a11: $(QT_PLUGIN)

moc_qaiminputcontextproxy.cpp: qaiminputcontextproxy.h
	$(MOC) $(QTINCLUDES) qaiminputcontextproxy.h        -o moc_qaiminputcontextproxy.cpp

moc_qaimproxy.cpp: qaimproxy.h
	$(MOC) $(QTINCLUDES) qaimproxy.h  -o moc_qaimproxy.cpp

moc_qaimplatforminputcontext.cpp: qaimplatforminputcontext.h
	$(MOC)  $(QTINCLUDES) qaimplatforminputcontext.h       -o moc_qaimplatforminputcontext.cpp

main.moc: main.cpp
	$(MOC) $(QTINCLUDES)  main.cpp -o  main.moc

ifeq ($(QT5), yes)
SRCS := main.cpp \
	qaimplatforminputcontext.cpp \
	qaiminputcontextproxy.cpp \
	qaimproxy.cpp \
	moc_qaiminputcontextproxy.cpp \
	moc_qaimproxy.cpp \
	moc_qaimplatforminputcontext.cpp
endif

OBJS := $(SRCS:%.cpp=%.o)

DEPS += $(SRCS:%.cpp=%.d)

$(QT_PLUGIN): main.moc  $(OBJS)
	$(CXX) $(OBJS) $(QTLIBS) -lQt5DBus  -shared  -o $@

%.o : %.cpp
	$(CXX) $(QTINCLUDES) $(QT_CXXFLAGS)  -c $< -o $@

clean:
	-@rm $(QT_PLUGIN)
	-@rm moc_*
	-@rm main.moc
	-@find . -name "*.[oadi]" | xargs -I{} rm {}

.PHONY: clean
