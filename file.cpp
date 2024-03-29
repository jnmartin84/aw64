/* Raw - Another World Interpreter
 * Copyright (C) 2004 Gregory Montoir
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "file.h"
#include <libdragon.h>

struct File_impl {
	bool _ioErr;
	File_impl() : _ioErr(false) {}
	virtual ~File_impl() {}
	virtual bool open(const char *path, const char *mode) = 0;
	virtual void close() = 0;
	virtual void seek(int32_t off) = 0;
	virtual void read(void *ptr, uint32_t size) = 0;
	virtual void write(void *ptr, uint32_t size) = 0;
};

struct stdFile : File_impl {
	int _fp = -1;
	stdFile() : _fp(-1) {}
	bool open(const char *path, const char *mode) {
		_ioErr = false;
		_fp = dfs_open(path);//, mode);
		return (_fp != -1);
	}
	void close() {
		if (_fp != -1) {
			dfs_close(_fp);
			_fp = -1;
		}
	}
	void seek(int32_t off) {
		if (_fp != -1) {
			dfs_seek(_fp, off, SEEK_SET);
		}
	}
	void read(void *ptr, uint32_t size) {
		if (_fp !=	-1) {
			size_t r = dfs_read(ptr, size, 1, _fp);
			if (r != size) {
				_ioErr = true;
			}
		}
	}
	void write(void *ptr, uint32_t size) {
		if (_fp != -1) {
			size_t r = size;//0;//fwrite(ptr, 1, size, _fp);
			if (r != size) {
				_ioErr = true;
			}
		}
	}
};

File::File(bool gzipped) {
	_impl = new stdFile;
}

File::~File() {
	_impl->close();
	delete _impl;
}

bool File::open(const char *filename, const char *directory, const char *mode) {	
//	_impl->close();
//	char buf[512];
//	sprintf(buf, "%s", filename);
//	char *p = buf;
//	string_lower(p);
	bool opened = _impl->open(filename, mode);
//	if (!opened) { // let's try lowercase
//		string_lower(p);
//		opened = _impl->open(buf, mode);
//	}
	return opened;
}

void File::close() {
	_impl->close();
}

bool File::ioErr() const {
	return _impl->_ioErr;
}

void File::seek(int32_t off) {
	_impl->seek(off);
}

void File::read(void *ptr, uint32_t size) {
	_impl->read(ptr, size);
}

uint8_t File::readByte() {
	uint8_t b;
	read(&b, 1);
	return b;
}

uint16_t File::readUint16BE() {
	uint8_t hi = readByte();
	uint8_t lo = readByte();
	uint16_t rv = (hi << 8) | lo;
	return rv;
}

uint32_t File::readUint32BE() {
	uint16_t hi = readUint16BE();
	uint16_t lo = readUint16BE();
	uint32_t rv = (hi << 16) | lo;
	return rv;	
}

void File::write(void *ptr, uint32_t size) {
	_impl->write(ptr, size);
}

void File::writeByte(uint8_t b) {
	write(&b, 1);
}

void File::writeUint16BE(uint16_t n) {
	writeByte(n >> 8);
	writeByte(n & 0xFF);
}

void File::writeUint32BE(uint32_t n) {
	writeUint16BE(n >> 16);
	writeUint16BE(n & 0xFFFF);
}
