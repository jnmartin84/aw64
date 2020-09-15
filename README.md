# aw64
nintendo 64 port of https://github.com/fabiensanglard/Another-World-Bytecode-Interpreter/

# important notes
If you want to build this from source, you will need to build libsdtc++ with the no exceptions and no rtti flags, theres a toolchain script included to handle that. It creates the full exact toolchain I used, gcc binutils There are also two small patches needed for libdragon, the timer code. See below for diff info
```
$ git diff include/timer.h
diff --git a/include/timer.h b/include/timer.h
index 2eeed3d..36990a1 100644
--- a/include/timer.h
+++ b/include/timer.h
@@ -24,8 +24,11 @@ typedef struct timer_link
     int ovfl;
     /** @brief Timer flags.  See #TF_ONE_SHOT and #TF_CONTINUOUS */
     int flags;
-    /** @brief Callback function to call when timer fires */
-    void (*callback)(int ovfl);
+    int param1;
+    int param2;
+    int param3;
+/** @brief Callback function to call when timer fires */
+    void (*callback)(int ovfl,int p1, int p2, int p3);
     /** @brief Link to next timer */
     struct timer_link *next;
 } timer_link_t;
@@ -82,9 +85,9 @@ extern "C" {
 /* initialize timer subsystem */
 void timer_init(void);
 /* create a new timer and add to list */
-timer_link_t *new_timer(int ticks, int flags, void (*callback)(int ovfl));
+timer_link_t *new_timer(int ticks, int flags, int param1, int param2, int param3, void (*callback)(int ovfl, int param1, int param2, int param3));
 /* start a timer not currently in the list */
-void start_timer(timer_link_t *timer, int ticks, int flags, void (*callback)(int ovfl));
+void start_timer(timer_link_t *timer, int ticks, int flags, int param1, int param2, int param3, void (*callback)(int ovfl, int param1, int param2, int param3));
 /* remove a timer from the list */
 void stop_timer(timer_link_t *timer);
 /* remove a timer from the list and delete it */
```
```
$ git diff src/timer.c
diff --git a/src/timer.c b/src/timer.c
index aad4861..7ead106 100644
--- a/src/timer.c
+++ b/src/timer.c
@@ -94,7 +94,7 @@ static int __proc_timers(timer_link_t * head)
                        /* yes - timed out, do callback */
                        head->ovfl = head->left;
                        if (head->callback)
-                               head->callback(head->ovfl);
+                               head->callback(head->ovfl,head->param1,head->param2,head->param3);

                        /* reset ticks if continuous */
                        if (head->flags & TF_CONTINUOUS)
@@ -184,7 +184,7 @@ void timer_init(void)
  *
  * @return A pointer to the timer structure created
  */
-timer_link_t *new_timer(int ticks, int flags, void (*callback)(int ovfl))
+timer_link_t *new_timer(int ticks, int flags, int param1, int param2, int param3, void (*callback)(int ovfl, int param1, int param2, int param3))
 {
        timer_link_t *timer = malloc(sizeof(timer_link_t));
        if (timer)
@@ -193,6 +193,9 @@ timer_link_t *new_timer(int ticks, int flags, void (*callback)(int ovfl))
                timer->set = ticks;
                timer->flags = flags;
                timer->callback = callback;
+timer->param1 = param1;
+timer->param2 = param2;
+timer->param3 = param3;

                disable_interrupts();

@@ -224,7 +227,7 @@ timer_link_t *new_timer(int ticks, int flags, void (*callback)(int ovfl))
  * @param[in] callback
  *            Callback function to call when the timer expires
  */
-void start_timer(timer_link_t *timer, int ticks, int flags, void (*callback)(int ovfl))
+void start_timer(timer_link_t *timer, int ticks, int flags, int param1, int param2, int param3, void (*callback)(int ovfl,int param1,int param2, int param3))
 {
        if (timer)
        {
@@ -232,7 +235,9 @@ void start_timer(timer_link_t *timer, int ticks, int flags, void (*callback)(int
                timer->set = ticks;
                timer->flags = flags;
                timer->callback = callback;
-
+timer->param1 = param1;
+timer->param2 = param2;
+timer->param3 = param3;
                disable_interrupts();

                timer->next = TI_timers;

```

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
