# aw64
nintendo 64 port of https://github.com/fabiensanglard/Another-World-Bytecode-Interpreter/

# important notes
You will need to build libsdtc++ with the no exceptions and no rtti flags, theres a toolchain script included to handle that.
There is also a small patch needed for libdragon that I forgot about. I will add that soon.

# to create a rom
you will need

- "game.bin" either from the github repo or one that you built yourself

- n64tool.exe and chksum64.exe found in libdragon repo, and the ROM header file you'll also find in libdragon repo

- data files for a PC copy of Another World / Out Of This World (MEMLIST.BIN and BANK0 files ranging from BANK01 to BANK0D), located in a subdirectory "data" under the directory containing "game.bin"

- copies of the 20 Code Wheels to get past the initial copy protection screens

then run the following
```
path/to/n64tool.exe -l 4M                              \
                -h path/to/header  \
                -o game.z64 -t "game" game.bin  \
                -s $(MEMLISTBIN_OFFSET) data/MEMLIST.BIN        \
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
path/to/chksum64.exe game.z64
```

where each file offset is the previous file offset + previous file size and MEMLISTBIN_OFFSET is always 1048576B (1MB)

for the particular set of data files I have, with the following sizes
```
total 1180
  4 drwxr-xr-x+ 1       0 Sep  6 11:48 .
 20 drwxr-xr-x+ 1       0 Sep 14 20:54 ..
208 -rwxr-xr-x  1  209250 Mar 15  1992 BANK01
 76 -rwxr-xr-x  1   77608 Jan 13  1992 BANK02
 96 -rwxr-xr-x  1   95348 Mar 15  1992 BANK03
 60 -rwxr-xr-x  1   58524 Jan 13  1992 BANK04
 16 -rwxr-xr-x  1   15100 Jan 13  1992 BANK05
 44 -rwxr-xr-x  1   44034 Jan 13  1992 BANK06
100 -rwxr-xr-x  1   98528 Jan 13  1992 BANK07
124 -rwxr-xr-x  1  123656 Jan 13  1992 BANK08
  8 -rwxr-xr-x  1    7396 Feb 10  1992 BANK09
196 -rwxr-xr-x  1  200120 Mar 15  1992 BANK0A
 48 -rwxr-xr-x  1   46296 Feb 10  1992 BANK0B
 20 -rwxr-xr-x  1   18864 Jan 13  1992 BANK0C
156 -rwxr-xr-x  1  157396 Mar 15  1992 BANK0D
  4 -rwxr-xr-x  1    2940 Mar 15  1992 MEMLIST.BIN
```
my command looks like:
```
path/to/n64tool.exe -l 4M                              \
                -h path/to/header  \
                -o game.z64 -t "game" game.bin  \
                -s 1048576B data/MEMLIST.BIN        \
                -s 1051516B data/BANK01 \
                -s 1260766B data/BANK02 \
                -s 1338374B data/BANK03 \
                -s 1433722B data/BANK04 \
                -s 1492246B data/BANK05 \
                -s 1507346B data/BANK06 \
                -s 1551380B data/BANK07 \
                -s 1649908B data/BANK08 \
                -s 1773564B data/BANK09 \
                -s 1780960B data/BANK0A \
                -s 1981080B data/BANK0B \
                -s 2027376B data/BANK0C \
                -s 2046240B data/BANK0D
path/to/chksum64.exe game.z64
```

this will produce a playale Another World / Out Of This World ROM

# controls
dpad up/down/left/right - movement (maps to arrow keys)

c button up - enter passcode (maps to 'c' key)

a button - "button" button (maps to 'return' key)

start button - pause (maps to 'p' key)
