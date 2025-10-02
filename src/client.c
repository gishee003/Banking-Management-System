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

	int num = atoi(choice); 

	switch (num) {
    		case 1:
       			 send(sock, "BALANCE", 7, 0);
			 break;
	       	case 2:{
			char input[32];
        		double amt = 0.0;
		       	printf("Enter amount to deposit: ");
		    
			if(!fgets(input, sizeof(input), stdin) || sscanf(input, "%lf", &amt) != 1) {
				printf("Invalid input.\n");
				break;
		    	}
		    	char outbuf[BUFFER_SIZE];
		    	snprintf(outbuf, sizeof(outbuf), "DEPOSIT %.2f", amt);
		    	send(sock, outbuf, strlen(outbuf), 0);
			
		    	// wait for server response
			int n = recv(sock, outbuf, sizeof(outbuf) - 1, 0);
		    	if (n > 0) {
				outbuf[n] = '\0';
				printf("Server: %s\n", outbuf);
		    	}
		    	break;
		       }
		case 3:{
                         double amt = 0.0;
		     	 printf("Enter amount to withdraw: ");
		     	 if (scanf("%lf", &amt) != 1) {
			 	 int ch;
				 while ((ch = getchar()) != '\n' && ch != EOF);
			 	 printf("Invalid input.\n");
			 	 break;
		     	 }
		     	 char outbuf[BUFFER_SIZE];
		     	 snprintf(outbuf, sizeof(outbuf), "WITHDRAW %.2f", amt);
		     	 send(sock, outbuf, strlen(outbuf), 0);
			 
		     	 int n = recv(sock, outbuf, sizeof(outbuf) - 1, 0);
		     	 if (n > 0) {
			 	 outbuf[n] = '\0';
			 	 printf("Server: %s\n", outbuf);
		     	 }
		     	 break;
		       }
                case 4:{
		       char input[64];
		       int target_id;
		       double amt = 0.0;
		       
		       printf("Enter target customer ID and amount: ");
		       if (!fgets(input, sizeof(input), stdin) || 
			       	       sscanf(input, "%d %lf", &target_id, &amt) != 2) {
		       	       printf("Invalid input. Usage: <id> <amount>\n");
		       	       break;
		       }
		       
		       char outbuf[BUFFER_SIZE];
		       snprintf(outbuf, sizeof(outbuf), "TRANSFER %d %.2f", target_id, amt);
		       send(sock, outbuf, strlen(outbuf), 0);
		       
		       int n = recv(sock, outbuf, sizeof(outbuf) - 1, 0);
		       if (n > 0) {
		       	       outbuf[n] = '\0';
		       	       printf("Server: %s\n", outbuf);
		       }
		       break;
		       }
		case 5:{
                        char input[64];
                        double amt, interest;
                        int tenure;

                        printf("Enter loan amount, tenure (months), and interest rate: ");
                        if (!fgets(input, sizeof(input), stdin) ||
                                        sscanf(input, "%lf %d %lf", &amt, &tenure, &interest) != 3) {
                                printf("Invalid input. Usage: <amount> <tenure_months> <interest_rate>\n");
                                break;
                        }

                        char outbuf[BUFFER_SIZE];
                        snprintf(outbuf, sizeof(outbuf), "LOAN %.2f %d %.2f", amt, tenure, interest);
                        send(sock, outbuf, strlen(outbuf), 0);

                        int n = recv(sock, outbuf, sizeof(outbuf) - 1, 0);
                        if (n > 0) {
                                outbuf[n] = '\0';
                                printf("Server: %s\n", outbuf);
                        }
                        break;
                        }
                case 6:{
                        char new_pass[50];
                        printf("Enter new password: ");
                        if (!fgets(new_pass, sizeof(new_pass), stdin)) {
                                printf("Input error.\n");
                                break;
                        }
                        new_pass[strcspn(new_pass, "\n")] = '\0';

                        char outbuf[BUFFER_SIZE];
                        snprintf(outbuf, sizeof(outbuf), "CHANGE_PASSWORD %s", new_pass);
                        send(sock, outbuf, strlen(outbuf), 0);
			int n = recv(sock, outbuf, sizeof(outbuf) - 1, 0);
                        if (n > 0) {
                                outbuf[n] = '\0';
                                printf("Server: %s\n", outbuf);
                        }
                        break;
                       }
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
   	
	printf("\n");
    }

	
    close(sock);
    return 0;
}

