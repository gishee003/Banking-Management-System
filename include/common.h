#ifndef COMMON_H
#define COMMON_H

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

#define ROLE_CUSTOMER 1
#define ROLE_EMPLOYEE 2
#define ROLE_MANAGER  3
#define ROLE_ADMIN    4

#define CUSTOMER_DB "data/customers.dat"

struct Customer {
    int id;
    char username[30];
    char password[30];
    float balance;
    int isActive;
    int isLoggedIn ;
};

#endif

