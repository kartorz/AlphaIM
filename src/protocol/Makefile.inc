SRCS_PROTOCOL := protocol/IC.cpp \
            protocol/xim/XIMSrv.cpp \
            protocol/xim/XIC.cpp  \
            protocol/DBusICSrv.cpp

OBJS_PROTOCOL := $(SRCS_PROTOCOL:%.cpp=%.o)

DEPS += $(SRCS_PROTOCOL:%.cpp=%.d)
