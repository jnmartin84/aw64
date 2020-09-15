# aw64
nintendo 64 port of https://github.com/fabiensanglard/Another-World-Bytecode-Interpreter/

# to create a rom
you will need

- "game.bin" either from the github repo or one that you built yourself

- n64tool.exe and chksum64.exe found in libdragon repo, and the ROM header file you'll also find in libdragon repo

- data files for a PC copy of Another World / Out Of This World (MEMLIST.BIN and BANK0 files ranging from BANK01 to BANK0D)

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

which will produce a playale Another World / Out Of This World ROM


#controls
dpad up/down/left/right - movement (maps to arrow keys)
c button up - enter passcode (maps to 'c' key)
a button - "button" button (maps to 'return' key)
start button - pause (maps to 'p' key)
