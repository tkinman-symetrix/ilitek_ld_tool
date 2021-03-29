DAEMON := ilitek_ld

CXX := gcc

CXXFLAGS += -static

CXXLIB += -lusb
CXXLIB += -lm

sources += ILITek_Main.c
sources += API/ILITek_Frequency.c
sources += API/ILITek_RawData.c
sources += API/ILITek_SensorTest.c
sources += API/ILITek_Upgrade.c
sources += ILITek_Device.c
sources += ILITek_Protocol_3X.c
sources += ILITek_Protocol_6X.c
sources += ILITek_DebugTool_3X.c

.PHONY: all
all:
	$(CXX) $(CXXFLAGS) $(sources) -o $(DAEMON) $(CXXLIB)
	@chmod 777 $(DAEMON)

.PHONY: clean
clean:
	@rm -rf $(DAEMON)
