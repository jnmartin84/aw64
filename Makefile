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
LINK_FLAGS = -L$(ROOTDIR)/mips64-elf/lib -ldragon -lc -lm -lstdc++ -ldragonsys -lc -lm -Tn64.ld
PROG_NAME = aw

# filesystem constants
# used to generate #define values for n64_rom_fs.c
# and command line parameter values for n64tool

# this is the offset in bytes of the first file (MEMLIST.BIN)
# from the start of the ROM image
# this should not be changed
moffset = 1048576

# 2952794112 (0xB0001000) magic uint32_t constant, start of ROM image
mbase = $(shell echo $$(( 2952794112 + $(moffset) )))

# *size are the actual size in bytes of each file
# if you have different files, you can change these
# and all of the offsets will be generated correctly
# note that if you change these, you need to do a 
# "make clean" before doing anything else as these
# values are used during compilation of n64_rom_fs.c
msize = 2940
b1size = 209250
b2size = 77608
b3size = 95348
b4size = 58524
b5size = 15100
b6size = 44034
b7size = 98528
b8size = 123656
b9size = 7396
basize = 200120
bbsize = 46296
bcsize = 18864
bdsize = 157396

# *base are where in address space the files start
# dont change those lines
b1base=$(shell echo $$(( $(mbase) + $(msize) )))
b2base=$(shell echo $$(( $(b1base) + $(b1size) )))
b3base=$(shell echo $$(( $(b2base) + $(b2size) )))
b4base=$(shell echo $$(( $(b3base) + $(b3size) )))
b5base=$(shell echo $$(( $(b4base) + $(b4size) )))
b6base=$(shell echo $$(( $(b5base) + $(b5size) )))
b7base=$(shell echo $$(( $(b6base) + $(b6size) )))
b8base=$(shell echo $$(( $(b7base) + $(b7size) )))
b9base=$(shell echo $$(( $(b8base) + $(b8size) )))
babase=$(shell echo $$(( $(b9base) + $(b9size) )))
bbbase=$(shell echo $$(( $(babase) + $(basize) )))
bcbase=$(shell echo $$(( $(bbbase) + $(bbsize) )))
bdbase=$(shell echo $$(( $(bcbase) + $(bcsize) )))

# *off are zero-based instead of 0xB0001000-based offsets
# of each file from start of ROM image
b1off=$(shell echo $$(( $(moffset) + $(msize) )))
b2off=$(shell echo $$(( $(b1off) + $(b1size) )))
b3off=$(shell echo $$(( $(b2off) + $(b2size) )))
b4off=$(shell echo $$(( $(b3off) + $(b3size) )))
b5off=$(shell echo $$(( $(b4off) + $(b4size) )))
b6off=$(shell echo $$(( $(b5off) + $(b5size) )))
b7off=$(shell echo $$(( $(b6off) + $(b6size) )))
b8off=$(shell echo $$(( $(b7off) + $(b7size) )))
b9off=$(shell echo $$(( $(b8off) + $(b8size) )))
baoff=$(shell echo $$(( $(b9off) + $(b9size) )))
bboff=$(shell echo $$(( $(baoff) + $(basize) )))
bcoff=$(shell echo $$(( $(bboff) + $(bbsize) )))
bdoff=$(shell echo $$(( $(bcoff) + $(bcsize) )))

# *_OFFSET are needed by n64tool
MEMLISTBIN_OFFSET = $(moffset)B
BANK01_OFFSET = $(b1off)B
BANK02_OFFSET = $(b2off)B
BANK03_OFFSET = $(b3off)B
BANK04_OFFSET = $(b4off)B
BANK05_OFFSET = $(b5off)B
BANK06_OFFSET = $(b6off)B
BANK07_OFFSET = $(b7off)B
BANK08_OFFSET = $(b8off)B
BANK09_OFFSET = $(b9off)B
BANK0A_OFFSET = $(baoff)B
BANK0B_OFFSET = $(bboff)B
BANK0C_OFFSET = $(bcoff)B
BANK0D_OFFSET = $(bdoff)B

# N64_PCDOS is needed by endian.have
# *_ROM_base / *_ROM_size are needed by n64_rom_fs.c
DEFINES = -DN64_PCDOS -DMEMLIST_ROM_base=$(mbase) -DMEMLIST_size=$(msize) -DBANK01_ROM_base=$(b1base) -DBANK01_size=$(b1size)	\
	-DBANK02_ROM_base=$(b2base) -DBANK02_size=$(b2size) -DBANK03_ROM_base=$(b3base) -DBANK03_size=$(b3size)	\
	-DBANK04_ROM_base=$(b4base) -DBANK04_size=$(b4size) -DBANK05_ROM_base=$(b5base) -DBANK05_size=$(b5size)	\
	-DBANK06_ROM_base=$(b6base) -DBANK06_size=$(b6size) -DBANK07_ROM_base=$(b7base) -DBANK07_size=$(b7size)	\
	-DBANK08_ROM_base=$(b8base) -DBANK08_size=$(b8size) -DBANK09_ROM_base=$(b9base) -DBANK09_size=$(b9size)	\
	-DBANK0A_ROM_base=$(babase) -DBANK0A_size=$(basize) -DBANK0B_ROM_base=$(bbbase) -DBANK0B_size=$(bbsize)	\
	-DBANK0C_ROM_base=$(bcbase) -DBANK0C_size=$(bcsize) -DBANK0D_ROM_base=$(bdbase) -DBANK0D_size=$(bdsize)

CXXFLAGS := $(DEFINES) -std=c++11 -fno-rtti -fno-exceptions -march=vr4300 -mtune=vr4300 -O3 -Wall -G0 -Wl,--defsym -Wl,__cxa_pure_virtual=0 -Wl,--defsym -Wl,__cxa_deleted_virtual=0 -Wundef -Wwrite-strings -Wnon-virtual-dtor -Wno-multichar -I$(ROOTDIR)/include -I$(ROOTDIR)/mips64-elf/include -fno-use-cxa-atexit

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
## uncomment these and copy to the "game.elf" target after the LD line 
## if you want to run in debugger and have references for symbols
## output all symbols
#	$(OBJDUMP) -t game.elf > game_symbols.txt
## filter and output just functions
#	cat game_symbols.txt | grep 'F .text' > game_functions.txt

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $*.o

n64_memcpy.o: n64_memcpy.S
	$(CC) -c $< -o $@

n64_memset.o: n64_memset.S
	$(CC) -c $< -o $@

clean:
	rm -f *.o *.d

-include $(DEPS)
