#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../include/common.h"
#include <termios.h>

int is_logged_in(int customerId);
void add_session(int customerId);
void remove_session(int customerId);
int authenticate_customer(const char *username, const char *password, struct Customer *out);
void logout_customer(int id);


void customer_menu(int sock, struct Customer c) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer),
             "Welcome, %s! (Balance: %.2f)\n1. View Balance\n2. Logout\nChoice: ",
             c.username, c.balance);
    send(sock, buffer, strlen(buffer), 0);

    int n = recv(sock, buffer, BUFFER_SIZE-1, 0);
    buffer[n] = '\0';

    if (buffer[0] == '1') {
        snprintf(buffer, sizeof(buffer), "Your balance: %.2f\n", c.balance);
        send(sock, buffer, strlen(buffer), 0);
    }

    logout_customer(c.id);
    send(sock, "You have been logged out.\n", 26, 0);
    exit(0);
}

void handle_client(int sock) {
  
    struct Customer c;

    char username[30];
    char password[30];

    int n ;
    n = recv(sock, username, sizeof(username)-1, 0);
    username[n] = '\0';

    n = recv(sock, password, sizeof(password)-1, 0);
    password[n] = '\0';

    if(authenticate_customer(username, password, &c)){
    	if(is_logged_in(c.id)){
		send(sock,"User already logged in elsewhere.\n",34,0);
	}
	else{
		add_session(c.id);
		customer_menu(sock,c);
		remove_session(c.id);
	}
    } 
    else{
	    send(sock,"Login failed.\n",14,0);
    }

    close(sock);
    exit(0);
}

int main() {
    int server_fd, client_sock;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) { perror("socket failed"); exit(1); }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed"); exit(1);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen failed"); exit(1);
    }

    printf("Server running on port %d...\n", PORT);

    while (1) {
        client_sock = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (client_sock < 0) { perror("accept failed"); continue; }

        if (fork() == 0) {
            close(server_fd);
            handle_client(client_sock);
        } else {
            close(client_sock);
        }
    }
    return 0;
}

