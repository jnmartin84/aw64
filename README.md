# aw64
nintendo 64 port of https://github.com/fabiensanglard/Another-World-Bytecode-Interpreter/

# how to build
grab the latest libdragon trunk, build the toolchain, build the library
get the data files (bank* and memlist.bin) from the 20th Anniversary release of OOTW on GOG
git clone this repo, cd to the repo dir
mkdir filesystem
copy the data files into filesystem directory
make
play it

# controls
dpad up/down/left/right - movement (maps to arrow keys)

c button up - enter passcode (maps to 'c' key)

a button - "button" button (maps to 'return' key)

start button - pause (maps to 'p' key)
