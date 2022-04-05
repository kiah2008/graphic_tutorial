#pragma once

#include <unistd.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/un.h>


int create_socket(const char *path);
int connect_socket(int sock, const char *path);

void write_fd(int sock, int fd, void *data, size_t data_len);
void read_fd(int sock, int *fd, void *data, size_t data_len);