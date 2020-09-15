ROOTDIR = $(N64_INST)
GCCN64PREFIX = $(ROOTDIR)/bin/mips64-elf-
CHKSUM64PATH = $(ROOTDIR)/bin/chksum64
MKDFSPATH = $(ROOTDIR)/bin/mkdfs
MKSPRITEPATH = $(ROOTDIR)/bin/mksprite
HEADERPATH = $(ROOTDIR)/mips64-elf/lib
N64TOOL = $(ROOTDIR)/bin/n64tool
HEADERNAME = header

LINK_FLAGS = -L$(ROOTDIR)/mips64-elf/lib -ldragon -lc -lm -lstdc++ -ldragonsys -lc -lm -Tn64.ld
PROG_NAME = aw

CFLAGS_ASM = -g -std=gnu99 -march=vr4300 -mtune=vr4300 -Wall -G4 -Os -I$(ROOTDIR)/include -I$(ROOTDIR)/mips64-elf/include

CC = $(GCCN64PREFIX)gcc
AS = $(GCCN64PREFIX)as
LD = $(GCCN64PREFIX)ld
OBJCOPY = $(GCCN64PREFIX)objcopy
OBJDUMP = $(GCCN64PREFIX)objdump

# this works for combination of MIPS R4300 and DOS data file endian-ness differences
DEFINES = -DSYS_LITTLE_ENDIAN

CXX = $(GCCN64PREFIX)g++
CXXFLAGS := $(DEFINES) -std=c++11 -fno-rtti -fno-exceptions -march=vr4300 -mtune=vr4300 -Os -Wall -G0 -Wl,--defsym -Wl,__cxa_pure_virtual=0 -Wl,--defsym -Wl,__cxa_deleted_virtual=0 -Wundef -Wwrite-strings -Wnon-virtual-dtor -Wno-multichar -I$(ROOTDIR)/include -I$(ROOTDIR)/mips64-elf/include -fno-use-cxa-atexit

MEMLISTBIN_OFFSET = 1048576B
BANK01_OFFSET = 1051516B
BANK02_OFFSET = 1260766B
BANK03_OFFSET = 1338374B
BANK04_OFFSET = 1433722B
BANK05_OFFSET = 1492246B
BANK06_OFFSET = 1507346B
BANK07_OFFSET = 1551380B
BANK08_OFFSET = 1649908B
BANK09_OFFSET = 1773564B
BANK0A_OFFSET = 1780960B
BANK0B_OFFSET = 1981080B
BANK0C_OFFSET = 2027376B
BANK0D_OFFSET = 2046240B

SRCS = bank.cpp file.cpp engine.cpp mixer.cpp resource.cpp parts.cpp vm.cpp \
	serializer.cpp sfxplayer.cpp staticres.cpp util.cpp video.cpp main.cpp sysImplementation.cpp \
	exception.cpp n64_rom_fs.cpp

OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

game.z64: game.elf
	$(OBJCOPY) game.elf game.bin -O binary
	rm -f game.z64
	$(N64TOOL) -l 4M \
		-h $(HEADERPATH)/$(HEADERNAME) \
		-o game.z64 -t "game" game.bin \
		-s $(MEMLISTBIN_OFFSET) data/MEMLIST.BIN \
		-s $(BANK01_OFFSET) data/BANK01 \
		-s $(BANK02_OFFSET) data/BANK02 \
		-s $(BANK03_OFFSET) data/BANK03 \
		-s $(BANK04_OFFSET) data/BANK04 \
		-s $(BANK05_OFFSET) data/BANK05 \
		-s $(BANK06_OFFSET) data/BANK06 \
		-s $(BANK07_OFFSET) data/BANK07 \
		-s $(BANK08_OFFSET) data/BANK08 \
		-s $(BANK09_OFFSET) data/BANK09 \
		-s $(BANK0A_OFFSET) data/BANK0A \
		-s $(BANK0B_OFFSET) data/BANK0B \
		-s $(BANK0C_OFFSET) data/BANK0C \
		-s $(BANK0D_OFFSET) data/BANK0D
	$(CHKSUM64PATH) game.z64

game.elf: $(OBJS)
	$(LD) -o game.elf n64_memcpy.o n64_memset.o $(OBJS) $(LINK_FLAGS)
# uncomment these if you want to run in debugger and have references for symbols
# all symbols
#	$(OBJDUMP) -t game.elf > game_symbols.txt
# just functions
#	cat game_symbols.txt | grep 'F .text' > game_functions.txt

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $*.o

n64_memcpy.o: n64_memcpy.S
	$(CC) $(CFLAGS_ASM) -c $< -o $@

n64_memset.o: n64_memset.S
	$(CC) $(CFLAGS_ASM) -c $< -o $@

clean:
	rm -f *.o *.d

-include $(DEPS)
