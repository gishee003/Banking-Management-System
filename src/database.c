#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "../include/common.h"
#include "../include/database.h"

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

int deposit_to_account(int cust_id, double amount, double *new_balance) {
    if (amount <= 0) return 0;

    int fd = open(CUSTOMER_DB, O_RDWR);
    if (fd < 0) return 0;

    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;      
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;            

    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        close(fd);
        return 0;
    }

    struct Customer c;
    off_t pos = 0;
    int success = 0;
    while (read(fd, &c, sizeof(c)) == sizeof(c)) {
        if (c.id == cust_id) {
            c.balance += amount;
            if (lseek(fd, pos, SEEK_SET) == -1) break;
            if (write(fd, &c, sizeof(c)) != sizeof(c)) break;
            *new_balance = c.balance;
            success = 1;
            break;
        }
        pos += sizeof(c);
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
    return success;
}

void append_transaction_log(int cust_id, const char *type, double amount, int target_id) {
    int fd = open("data/transactions.dat", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) return;

    time_t t = time(NULL);
    struct tm tm;
    localtime_r(&t, &tm);
    char timestr[64];
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", &tm);

    char line[256];
 
    int len = snprintf(line, sizeof(line), "%d,%s,%.2f,%d,%s\n",
                       cust_id, type, amount, target_id, timestr);
    if (len > 0) write(fd, line, (size_t)len);

    close(fd);
}

int withdraw_from_account(int cust_id, double amount, double *new_balance) {
    if (amount <= 0) return 0;

    int fd = open(CUSTOMER_DB, O_RDWR);
    if (fd < 0) return 0;

    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        close(fd);
        return 0;
    }

    struct Customer c;
    off_t pos = 0;
    int success = 0;
    while (read(fd, &c, sizeof(c)) == sizeof(c)) {
        if (c.id == cust_id) {
            if (c.balance < amount) {
                success = -1;
                break;
            }
            c.balance -= amount;
            if (lseek(fd, pos, SEEK_SET) == -1) break;
            if (write(fd, &c, sizeof(c)) != sizeof(c)) break;
            *new_balance = c.balance;
            success = 1;
            break;
        }
        pos += sizeof(c);
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
    return success;
}

int transfer_funds(int from_id, int to_id, double amount, double *new_balance) {
    if (amount <= 0) return 0;

    int fd = open(CUSTOMER_DB, O_RDWR);
    if (fd < 0) return 0;

    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        close(fd);
        return 0;
    }

    struct Customer c;
    off_t pos = 0;
    int from_found = 0, to_found = 0;
    double from_balance = 0.0;

    // Read all customers into memory (small system assumption)
    // For bigger DBs, we’d use two passes.
    while (read(fd, &c, sizeof(c)) == sizeof(c)) {
        if (c.id == from_id) {
            if (c.balance < amount) {
                lock.l_type = F_UNLCK;
                fcntl(fd, F_SETLK, &lock);
                close(fd);
                return -1; // insufficient funds
            }
            c.balance -= amount;
            from_balance = c.balance;
            if (lseek(fd, pos, SEEK_SET) == -1) break;
            if (write(fd, &c, sizeof(c)) != sizeof(c)) break;
            from_found = 1;
        } else if (c.id == to_id) {
            c.balance += amount;
            if (lseek(fd, pos, SEEK_SET) == -1) break;
            if (write(fd, &c, sizeof(c)) != sizeof(c)) break;
            to_found = 1;
        }
        pos += sizeof(c);
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);

    if (from_found && to_found) {
        *new_balance = from_balance;
        return 1; // success
    }
    return 0; // failure
}

//---------------- LOAN FUNCTIONS --------------------

int generateLoanId() {
    int max_id = 0;
    Loan loan;

    int fd = open("loans.dat", O_RDONLY);
    if (fd < 0) return 1; // first loan

    while (read(fd, &loan, sizeof(Loan)) == sizeof(Loan)) {
        if (loan.loan_id > max_id)
            max_id = loan.loan_id;
    }
    close(fd);

    return max_id + 1;
}

void addLoanRequest(int customer_id, double amount, int tenure, double interest_rate) {
    Loan loan;
    loan.loan_id = generateLoanId();
    loan.customer_id = customer_id;
    loan.amount = amount;
    loan.tenure_months = tenure;
    loan.interest_rate = interest_rate;
    strcpy(loan.status, "Pending");

    int fd = open("loans.dat", O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (fd < 0) {
        perror("Error opening loans.dat");
        return;
    }

    if (write(fd, &loan, sizeof(Loan)) != sizeof(Loan)) {
        perror("Error writing loan record");
    }

    close(fd);
}

int change_password(int cust_id, const char *new_password) {
    int fd = open(CUSTOMER_DB, O_RDWR);
    if (fd < 0) return 0;

    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        close(fd);
        return 0;
    }

    struct Customer c;
    off_t pos = 0;
    int found = 0;

    while (read(fd, &c, sizeof(c)) == sizeof(c)) {
        if (c.id == cust_id) {
            strncpy(c.password, new_password, sizeof(c.password) - 1);
            c.password[sizeof(c.password) - 1] = '\0';

            if (lseek(fd, pos, SEEK_SET) == -1) break;
            if (write(fd, &c, sizeof(c)) != sizeof(c)) break;

	    fsync(fd);
            found = 1;
            break;
        }
        pos += sizeof(c);
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);

    return found;
}

