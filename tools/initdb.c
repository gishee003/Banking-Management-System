#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "../include/common.h"

int main() {
    int fd = open(CUSTOMER_DB, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    struct Customer c = {1, "alice", "pass123", 5000.0, 1};
    write(fd, &c, sizeof(c));

    struct Customer d = {2, "bob", "secret", 3000.0, 1};
    write(fd, &d, sizeof(d));

    close(fd);
    return 0;
}

