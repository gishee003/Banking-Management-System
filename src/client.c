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
        while (1) {
        printf("\n----------- Customer Menu -----------\n");
        printf("1. View Account Balance\n");
	printf("2. Deposit Money\n");
	printf("3. Withdraw Money\n");
	printf("4. Transfer Funds\n");
        printf("5. Apply for a Loan\n");
	printf("6. Change Password\n");
        printf("7. Add Feedback\n");
        printf("8. View Transaction History\n");
        printf("9. Logout\n");
        printf("10. Exit\n");
        printf("Choice: ");

        char choice[10];
        fgets(choice, sizeof(choice), stdin);
        choice[strcspn(choice, "\n")] = '\0';

        /*if (strcmp(choice, "1") == 0) {
            send(sock, "BALANCE", 7, 0);
        } else if (strcmp(choice, "2") == 0) {
            send(sock, "EXIT", 4, 0);
            break;
        } else {
            printf("Invalid choice, try again.\n");
            continue;
        }*/

	int num = atoi(choice); 

	switch (num) {
    		case 1:
       			 send(sock, "BALANCE", 7, 0);
			 break;
	       	case 2:
        		send(sock, "DEPOSIT", 7, 0);
        		break;
		case 3:
                         send(sock, "WITHDRAW", 8, 0);
                         break;
                case 4:
                        send(sock, "TRANSFER", 8, 0);
                        break;
		case 5:
                         send(sock, "LOAN", 7, 0);
                         break; 
                case 6:
                        send(sock, "CHANGE PASSWORD", 15, 0);
                        break;
		case 7:
                         send(sock, "FEEDBACK", 8, 0);
                         break; 
                case 8:
                        send(sock, "TRANSACTION HISTORY", 19, 0);
                        break;

		case 9:
                         send(sock, "LOGOUT", 6, 0);
                         break; 
                case 10:
                        send(sock, "EXIT", 4, 0);
                        break;

    		default:
        		printf("Invalid choice, try again.\n");
        		continue;
	}


        // Receive server reply
        n = recv(sock, buffer, BUFFER_SIZE-1, 0);
        buffer[n] = '\0';
        printf("%s", buffer);

        if (strstr(buffer, "Goodbye")) break;
    }
	
   /* while ((n = recv(sock, buffer, BUFFER_SIZE-1, 0)) > 0) {
        buffer[n] = '\0';
        printf("%s", buffer);

        if (strstr(buffer, "Goodbye")) break;

        fgets(buffer, BUFFER_SIZE, stdin);
        send(sock, buffer, strlen(buffer), 0);
    }*/

    close(sock);
    return 0;
}

