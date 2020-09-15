#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <libdragon.h>

extern "C" void *__n64_memcpy_ASM(void *d, const void *s, size_t n);

#define MEMLIST_ROM_base	(0xB0101000)
#define MEMLIST_size		2940

#define BANK01_ROM_base		(MEMLIST_ROM_base + MEMLIST_size)
#define BANK01_size			209250

#define BANK02_ROM_base		(BANK01_ROM_base + BANK01_size)
#define BANK02_size			77608

#define BANK03_ROM_base		(BANK02_ROM_base + BANK02_size)
#define BANK03_size			95348

#define BANK04_ROM_base		(BANK03_ROM_base + BANK03_size)
#define BANK04_size			58524

#define BANK05_ROM_base		(BANK04_ROM_base + BANK04_size)
#define BANK05_size			15100

#define BANK06_ROM_base		(BANK05_ROM_base + BANK05_size)
#define BANK06_size			44034

#define BANK07_ROM_base		(BANK06_ROM_base + BANK06_size)
#define BANK07_size			98528

#define BANK08_ROM_base		(BANK07_ROM_base + BANK07_size)
#define BANK08_size			123656

#define BANK09_ROM_base		(BANK08_ROM_base + BANK08_size)
#define BANK09_size			7396

#define BANK0A_ROM_base		(BANK09_ROM_base + BANK09_size)
#define BANK0A_size			200120

#define BANK0B_ROM_base		(BANK0A_ROM_base + BANK0A_size)
#define BANK0B_size			46296

#define BANK0C_ROM_base		(BANK0B_ROM_base + BANK0B_size)
#define BANK0C_size			18864

#define BANK0D_ROM_base		(BANK0C_ROM_base + BANK0C_size)
#define BANK0D_size			157396

#define MAX_FILES               16

const int MEMLIST_FILE = MEMLIST_ROM_base;
const int MEMLIST_FILESIZE = MEMLIST_size;

const int BANK01_FILE = BANK01_ROM_base;
const int BANK01_FILESIZE = BANK01_size;

const int BANK02_FILE = BANK02_ROM_base;
const int BANK02_FILESIZE = BANK02_size;

const int BANK03_FILE = BANK03_ROM_base;
const int BANK03_FILESIZE = BANK03_size;

const int BANK04_FILE = BANK04_ROM_base;
const int BANK04_FILESIZE = BANK04_size;

const int BANK05_FILE = BANK05_ROM_base;
const int BANK05_FILESIZE = BANK05_size;

const int BANK06_FILE = BANK06_ROM_base;
const int BANK06_FILESIZE = BANK06_size;

const int BANK07_FILE = BANK07_ROM_base;
const int BANK07_FILESIZE = BANK07_size;

const int BANK08_FILE = BANK08_ROM_base;
const int BANK08_FILESIZE = BANK08_size;

const int BANK09_FILE = BANK09_ROM_base;
const int BANK09_FILESIZE = BANK09_size;

const int BANK0A_FILE = BANK0A_ROM_base;
const int BANK0A_FILESIZE = BANK0A_size;

const int BANK0B_FILE = BANK0B_ROM_base;
const int BANK0B_FILESIZE = BANK0B_size;

const int BANK0C_FILE = BANK0C_ROM_base;
const int BANK0C_FILESIZE = BANK0C_size;

const int BANK0D_FILE = BANK0D_ROM_base;
const int BANK0D_FILESIZE = BANK0D_size;

typedef struct rom_file_info_s {
    int32_t fd;
    uint32_t rom_base;
    uint32_t size;
    uint32_t seek;
}
rom_file_info_t;

static rom_file_info_t __attribute__((aligned(8))) files[MAX_FILES];
static uint8_t __attribute__((aligned(8))) file_opened[MAX_FILES] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static uint8_t __attribute__((aligned(8))) dmaBuf[65536];

/*__attribute__((noinline)) */void dma_and_copy(void *buf, int count, int ROM_base_address, int current_ROM_seek) {
    data_cache_hit_writeback_invalidate(dmaBuf, (count + 3) & ~3);
    dma_read((void *)((uint32_t)dmaBuf & 0x1FFFFFFF), ROM_base_address + (current_ROM_seek & ~1), (count + 3) & ~3);
    data_cache_hit_invalidate(dmaBuf, (count + 3) & ~3);
    __n64_memcpy_ASM(buf, dmaBuf + (current_ROM_seek & 1), count);
}

long rom_tell(int fd) {
    if ((fd < 0) || (fd > MAX_FILES)) {
        return -1;
    }
	
    return files[fd].seek;
}

int rom_lseek(int fd, off_t offset, int whence) {
    if ((fd < 0) || (fd > MAX_FILES)) {
        return -1;
    }
	
    switch (whence) {
        case SEEK_SET: {
            files[fd].seek = offset;
            break;
        }
        case SEEK_CUR: {
            files[fd].seek += offset;
            break;
        }
        case SEEK_END: {
            files[fd].seek = files[fd].size + offset;
            break;
        }
        default: {
            return -1;
            break;
        }
    }

    return files[fd].seek;
}

int rom_open(const char *name, const char *mode) {
	int FILE_START;
	int size;
	
    int had_open_file = 0;
    int i;

	if('M' == name[0]) {
		FILE_START = MEMLIST_FILE;
		size = MEMLIST_FILESIZE;
	}
	else if('1' == name[1]) {
		FILE_START = BANK01_FILE;
		size = BANK01_FILESIZE;
	}	
 	else if('2' == name[1]) {
		FILE_START = BANK02_FILE;
		size = BANK02_FILESIZE;
	}
	else if('3' == name[1]) {
		FILE_START = BANK03_FILE;
		size = BANK03_FILESIZE;
	}	
	else if('4' == name[1]) {
		FILE_START = BANK04_FILE;
		size = BANK04_FILESIZE;
	}
	else if('5' == name[1]) {
		FILE_START = BANK05_FILE;
		size = BANK05_FILESIZE;
	}	
 	else if('6' == name[1]) {
		FILE_START = BANK06_FILE;
		size = BANK06_FILESIZE;
	}
	else if('7' == name[1]) {
		FILE_START = BANK07_FILE;
		size = BANK07_FILESIZE;
	}	
	else if('8' == name[1]) {
		FILE_START = BANK08_FILE;
		size = BANK08_FILESIZE;
	}  	
	else if('9' == name[1]) {
		FILE_START = BANK09_FILE;
		size = BANK09_FILESIZE;
	}	
 	else if('A' == name[1]) {
		FILE_START = BANK0A_FILE;
		size = BANK0A_FILESIZE;
	}
	else if('B' == name[1]) {
		FILE_START = BANK0B_FILE;
		size = BANK0B_FILESIZE;
	}	
	else if('C' == name[1]) {
		FILE_START = BANK0C_FILE;
		size = BANK0C_FILESIZE;
	}	
	else if('D' == name[1]) {
		FILE_START = BANK0D_FILE;
		size = BANK0D_FILESIZE;
	}
	else {
		return -1;
	}
	
    for (i=0;i<MAX_FILES;i++) {
        if (!file_opened[i]) {
            had_open_file = 1;
            break;
        }
    }

    if (!had_open_file) {
        return -1;
    }
	else {
		files[i].fd       = i;
		files[i].rom_base = FILE_START;
		files[i].size     = size;
		files[i].seek     = 0;

		file_opened[i]    = 1;
		return i;
	}
}

int rom_close(int fd) {
    if ((fd < 0) || (fd > MAX_FILES)) {
        return -1;
    }

    files[fd].fd = -1;
    files[fd].rom_base = 0;
    files[fd].size = 0;
    files[fd].seek = 0;

    file_opened[fd] = 0;
    return 0;
}

int rom_read(int fd, void *buf, size_t nbyte) {
    int ROM_base_address = 0;
    int current_ROM_seek = 0;
    int count            = 0;

    if ((fd < 0) || (fd > MAX_FILES)) {
        return -1;
    }
	
	ROM_base_address = files[fd].rom_base;
    current_ROM_seek = files[fd].seek;
    count            = nbyte;

    if (count <= 32768) {
        dma_and_copy(buf, count, ROM_base_address, current_ROM_seek);
    }
    else {
        int tmp_seek = current_ROM_seek;
        int count_32K_blocks = count / 32768;
        int count_bytes = count % 32768;
        int actual_count = 0;

        while (count_32K_blocks > 0) {
            actual_count = 32768;

            dma_and_copy(buf, actual_count, ROM_base_address, tmp_seek);

            tmp_seek += 32768;
            buf += 32768;
            count_32K_blocks -= 1;
        }

        actual_count = count_bytes;

        dma_and_copy(buf, actual_count, ROM_base_address, tmp_seek);
    }

    files[fd].seek += count;

    return count;
}