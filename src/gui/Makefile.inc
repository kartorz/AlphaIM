
SRCS_GUI-$(GTK3) := gui/gtk/AimApp.cpp \
                    gui/gtk/AimWin.cpp \
                    gui/gtk/IcWin.cpp  \
                    gui/gtk/HelpWin.cpp \
                    gui/gtk/GuiMessager.cpp



SRCS_GUI-$(QT5) := gui/qt/AimApp.cpp \
                   gui/qt/GuiMessager.cpp \
                   gui/qt/moc_GuiMessager.cpp \
                   gui/qt/aimwin.cpp  \
                   gui/qt/moc_aimwin.cpp \
                   gui/qt/icwin.cpp \
                   gui/qt/moc_icwin.cpp \
                   gui/qt/qrc_default.cpp

OBJS_GUI := $(SRCS_GUI-yes:%.cpp=%.o)

