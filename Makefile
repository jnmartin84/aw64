# usual mips toolchain/libdragon makefile defines
# where things live, tool names
ROOTDIR = $(N64_INST)
GCCN64PREFIX = $(ROOTDIR)/bin/mips64-elf-
CHKSUM64PATH = $(ROOTDIR)/bin/chksum64
MKDFSPATH = $(ROOTDIR)/bin/mkdfs
MKSPRITEPATH = $(ROOTDIR)/bin/mksprite
HEADERPATH = $(ROOTDIR)/mips64-elf/lib
N64TOOL = $(ROOTDIR)/bin/n64tool
HEADERNAME = header
CC = $(GCCN64PREFIX)gcc
CXX = $(GCCN64PREFIX)g++
AS = $(GCCN64PREFIX)as
LD = $(GCCN64PREFIX)ld
OBJCOPY = $(GCCN64PREFIX)objcopy
OBJDUMP = $(GCCN64PREFIX)objdump
LINK_FLAGS = -L$(ROOTDIR)/mips64-elf/lib -L$(ROOTDIR)/lib -ldragon -lc -lm -lstdc++ -ldragonsys -lc -lm -Tn64.ld
PROG_NAME = ootw

# N64_PCDOS is needed by endian
DEFINES = -DN64_PCDOS

CXXFLAGS := -g $(DEFINES) -std=c++11 -march=vr4300 -mtune=vr4300 -O2 -Wall -G4 -Wl,--defsym -Wl,__cxa_pure_virtual=0 -Wl,--defsym -Wl,__cxa_deleted_virtual=0 -Wundef -Wwrite-strings -Wnon-virtual-dtor -Wno-multichar -I$(ROOTDIR)/mips64-elf/include -fno-use-cxa-atexit
#-fno-rtti -fno-exceptions 

SRCS = bank.cpp file.cpp engine.cpp mixer.cpp resource.cpp parts.cpp vm.cpp \
	serializer.cpp sfxplayer.cpp staticres.cpp util.cpp video.cpp main.cpp sysImplementation.cpp

OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

$(PROG_NAME).z64: $(PROG_NAME).elf rom.dfs
	$(OBJCOPY) $(PROG_NAME).elf $(PROG_NAME).bin -O binary
	rm -f $(PROG_NAME).z64
	$(N64TOOL)  \
		-h $(HEADERPATH)/$(HEADERNAME) \
		-o $(PROG_NAME).z64 -t $(PROG_NAME) $(PROG_NAME).bin \
		-s 1M rom.dfs
	chmod ugo=rw $(PROG_NAME).z64
	$(CHKSUM64PATH) $(PROG_NAME).z64

$(PROG_NAME).elf: $(OBJS)
	$(CXX) -o $(PROG_NAME).elf $(OBJS) -Wl,--wrap -Wl,__do_global_ctors $(LINK_FLAGS)
#	$(OBJDUMP) -t $(PROG_NAME).elf > game_symbols.txt
#	cat game_symbols.txt | grep 'F .text' > game_functions.txt

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $*.o

clean:
	rm -f *.o *.d

-include $(DEPS)

rom.dfs:
	$(MKDFSPATH) rom.dfs ./filesystem/
