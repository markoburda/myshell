#include "operations/operations.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>
#include <string>

int writebuffer(int fd, const char* buffer,  ssize_t size, int* status) {
	ssize_t written_bytes = 0;
	while (written_bytes < size) {
		ssize_t written_now = write(fd, buffer + written_bytes, size - written_bytes);
		if (written_now == -1) {
			if (errno == EINTR) 
				continue;
			else {
				*status = errno;
				return -1;
			}

		}
		else 
			written_bytes += written_now;
	}
	return 0;
}


int readbuffer(int fd, char* buffer,  ssize_t size, int* status) {
	lseek(fd, 0, SEEK_SET);
	ssize_t read_bytes = 0;
	while (read_bytes < size) {
		ssize_t read_now = read(fd, buffer + read_bytes, size - read_bytes);
		if (read_now == -1) {
			if (errno == EINTR) 
				continue;
			else {
				*status = errno;
				return -1;
			}

		}
		else 
			read_bytes += read_now;
	}
	return 0;
}



int operations::mycat(std::vector<std::string> files, int hex){
	std::vector<int> fds;
	for (const auto& file : files) {
		int fd = open(const_cast<char*>(file.c_str()), O_RDONLY);
		if (fd == -1) {
			int errcode = errno;
			writebuffer(2, "Error, couldn't open file\n", 26, &errcode);
			return errcode;
		}
		fds.push_back(fd);
	}
	std::vector<std::pair<char *,size_t>> buffers;
	for (const auto& fd : fds) {
		int status = 0;
		size_t size = lseek(fd, 0, SEEK_END);
		char *buffer = new char[size];
		readbuffer(fd, buffer, size, &status);
		if (status) {
			int errcode = errno;
			writebuffer(2, "Error, couldn't read file\n", 26, &errcode);
			return errcode;
		}
		buffers.emplace_back(std::make_pair(buffer, size));
	}
	for (const auto& pair : buffers) {
		int status = 0;
		if (hex) {
			char* new_buffer = new char[pair.second * 4];
			size_t j = 0;
			for(size_t i = 0; i < pair.second; i++) {
	    		if ((!isprint(pair.first[i])) && (!isspace(pair.first[i]))) {
	    			 sprintf(&new_buffer[j], "\\x%02X", static_cast<unsigned char>(pair.first[i]));
	    			 j += 3;
	    		} else {
	    			new_buffer[j] = pair.first[i];
	    		}
	    		j++;
			}
			writebuffer(1, new_buffer, j, &status);
			delete [] new_buffer;
		} else 
			writebuffer(1, pair.first, pair.second, &status);
		if (status) {
			int errcode = errno;
			writebuffer(2, "Error, couldn't write in stdout\n", 32, &errcode);
			return errcode;
		}
		delete [] pair.first;
	}
	for (const auto& fd : fds) {
		int status = close(fd);
		if (status) {
			int errcode = errno;
			writebuffer(2, "Error, couldn't close file\n", 32, &errcode);
			return errcode;
		}
		
		
	}
	return 0;	
}
