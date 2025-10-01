#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "../include/common.h"


int authenticate_customer(const char *username, const char *password, struct Customer *out) {
    int fd = open(CUSTOMER_DB, O_RDONLY);
    if (fd < 0) return 0;

    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    fcntl(fd, F_SETLKW, &lock);

    struct Customer c;
    while (read(fd, &c, sizeof(c)) == sizeof(c)) {
        if (strcmp(c.username, username) == 0 && strcmp(c.password, password) == 0 && c.isActive) {
            *out = c;
            
	    lock.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &lock);

            close(fd);
            return 1;
        }
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
    return 0;
}

