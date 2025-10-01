#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../include/common.h"
#include <termios.h>
#include "../include/customer_menu.h"


void get_hidden_input(char *buffer, size_t size) {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO; 
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    fgets(buffer, size, stdin);
    buffer[strcspn(buffer, "\n")] = '\0';

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); 
    printf("\n");
}

int main() {
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connection failed");
        exit(1);
    }
	
    char username[30], password[30];
    printf("Enter username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';
    send(sock, username, strlen(username), 0);

    printf("Enter password: ");
    get_hidden_input(password, sizeof(password));
    send(sock, password, strlen(password), 0);

    int n;
    while ((n = recv(sock, buffer, BUFFER_SIZE-1, 0)) > 0) {
        buffer[n] = '\0';
        printf("%s", buffer);

        if (strstr(buffer, "Goodbye")) break;

        fgets(buffer, BUFFER_SIZE, stdin);
        send(sock, buffer, strlen(buffer), 0);
    }

    close(sock);
    return 0;
}

