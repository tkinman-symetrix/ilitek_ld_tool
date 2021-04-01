#
# Makefile for ilitek_ld_tool
#

program := ilitek_ld
objects := ILITek_Main.c \
	   ILITek_Device.c \
	   ILITek_Protocol_3X.c \
	   ILITek_Protocol_6X.c \
	   ILITek_DebugTool_3X.c \
	   API/ILITek_Frequency.c \
	   API/ILITek_RawData.c \
	   API/ILITek_SensorTest.c \
	   API/ILITek_Upgrade.c

libraries := usb m

CXX ?= gcc

CXXFLAGS = -Wall -ansi -O3 -g
CXXFLAGS += -D__ENABLE_DEBUG__
CXXFLAGS += -D__ENABLE_OUTBUF_DEBUG__
CXXFLAGS += -D__ENABLE_INBUF_DEBUG__
CXXFLAGS += -D__ENABLE_LOG_FILE_DEBUG__

CXXFLAGS += -static

LIB_FLAGS += $(addprefix -l, $(libraries))


.PHONY: all
all: $(objects)
	$(CXX) $^ $(CXXFLAGS) $(LIB_FLAGS) -o $(program)
	@chmod 777 $(program)

.PHONY: clean
clean:
	@rm -rf $(program)
