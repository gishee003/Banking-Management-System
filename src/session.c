#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "session.h"

struct Session {
    int customerId;
};

void logout_customer(int customerId) {
    remove_session(customerId);
}

// Check if customer already logged in
int is_logged_in(int customerId) {
    int fd = open(SESSION_FILE, O_RDONLY);
    if (fd < 0) return 0; // no session file yet

    struct Session s;
    while (read(fd, &s, sizeof(s)) == sizeof(s)) {
        if (s.customerId == customerId) {
            close(fd);
            return 1;
        }
    }
    close(fd);
    return 0;
}

// Add a new session
void add_session(int customerId) {
    int fd = open(SESSION_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) return;

    struct Session s = { .customerId = customerId };
    write(fd, &s, sizeof(s));
    close(fd);
}

// Remove session (logout)
void remove_session(int customerId) {
    int fd = open(SESSION_FILE, O_RDWR);
    if (fd < 0) return;

    struct Session s;
    off_t pos = 0;
    while (read(fd, &s, sizeof(s)) == sizeof(s)) {
        if (s.customerId == customerId) {
            // overwrite with dummy (or compact file later)
            s.customerId = -1;
            lseek(fd, pos, SEEK_SET);
            write(fd, &s, sizeof(s));
            break;
        }
        pos += sizeof(s);
    }
    close(fd);
}

