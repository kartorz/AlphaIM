SRCS_GUI := gui/gtk/AimApp.cpp \
            gui/gtk/AimWin.cpp \
            gui/gtk/IcWin.cpp  \
            gui/gtk/HelpWin.cpp \
            gui/gtk/GuiMessager.cpp


OBJS_GUI := $(SRCS_GUI:%.cpp=%.o)

