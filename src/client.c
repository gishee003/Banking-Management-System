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

ssize_t recv_and_print(int sock, char *buf, size_t bufsz) {
    ssize_t n = recv(sock, buf, bufsz - 1, 0);
    if (n <= 0) return n;
    buf[n] = '\0';
    printf("%s", buf);
    return n;
}

void get_hidden_input(char *buffer, size_t size) {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    if (!fgets(buffer, size, stdin)) buffer[0] = '\0';
    buffer[strcspn(buffer, "\n")] = '\0';

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n");
}

int main() {
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); exit(1); }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connection failed");
        exit(1);
    }

    char username[30], password[30];
    printf("Enter username: ");
    if (!fgets(username, sizeof(username), stdin)) { close(sock); exit(1); }
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

        char choice[16];
        if (!fgets(choice, sizeof(choice), stdin)) break;
        choice[strcspn(choice, "\n")] = '\0';
        int num = atoi(choice);

        char outbuf[BUFFER_SIZE];
        memset(outbuf, 0, sizeof(outbuf));

        switch (num) {
            case 1:
                send(sock, "BALANCE", strlen("BALANCE"), 0);
                break;

            case 2: {
                char input[64];
                double amt = 0.0;
                printf("Enter amount to deposit: ");
                if (!fgets(input, sizeof(input), stdin) || sscanf(input, "%lf", &amt) != 1) {
                    printf("Invalid input.\n");
                    continue;
                }
                snprintf(outbuf, sizeof(outbuf), "DEPOSIT %.2f", amt);
                send(sock, outbuf, strlen(outbuf), 0);
                break;
            }

            case 3: {
                char input[64];
                double amt = 0.0;
                printf("Enter amount to withdraw: ");
                if (!fgets(input, sizeof(input), stdin) || sscanf(input, "%lf", &amt) != 1) {
                    printf("Invalid input.\n");
                    continue;
                }
                snprintf(outbuf, sizeof(outbuf), "WITHDRAW %.2f", amt);
                send(sock, outbuf, strlen(outbuf), 0);
                break;
            }

            case 4: {
                char input[128];
                int target_id = 0;
                double amt = 0.0;
                printf("Enter target customer ID and amount: ");
                if (!fgets(input, sizeof(input), stdin) ||
                    sscanf(input, "%d %lf", &target_id, &amt) != 2) {
                    printf("Invalid input. Usage: <id> <amount>\n");
                    continue;
                }
                snprintf(outbuf, sizeof(outbuf), "TRANSFER %d %.2f", target_id, amt);
                send(sock, outbuf, strlen(outbuf), 0);
                break;
            }

            case 5: {
                char input[128];
                double amt = 0.0, interest = 0.0;
                int tenure = 0;
                printf("Enter loan amount, tenure (months), and interest rate: ");
                if (!fgets(input, sizeof(input), stdin) ||
                    sscanf(input, "%lf %d %lf", &amt, &tenure, &interest) != 3) {
                    printf("Invalid input. Usage: <amount> <tenure_months> <interest_rate>\n");
                    continue;
                }
                snprintf(outbuf, sizeof(outbuf), "LOAN %.2f %d %.2f", amt, tenure, interest);
                send(sock, outbuf, strlen(outbuf), 0);
                break;
            }

            case 6: {
                char new_pass[64];
                printf("Enter new password: ");
                if (!fgets(new_pass, sizeof(new_pass), stdin)) { printf("Input error.\n"); continue; }
                new_pass[strcspn(new_pass, "\n")] = '\0';
                snprintf(outbuf, sizeof(outbuf), "CHANGE_PASSWORD %s", new_pass);
                send(sock, outbuf, strlen(outbuf), 0);
                break;
            }

            case 7:
                send(sock, "FEEDBACK", strlen("FEEDBACK"), 0);
                break;

            case 8:
                // send the exact token server expects
                send(sock, "TRANSACTIONS", strlen("TRANSACTIONS"), 0);
                break;

            case 9:
                send(sock, "LOGOUT", strlen("LOGOUT"), 0);
                break;

            case 10:
                // send exact token without newline
                send(sock, "EXIT", strlen("EXIT"), 0);
                printf("Exiting client...\n");
                // wait for server reply then close
                n = recv(sock, buffer, sizeof(buffer) - 1, 0);
                if (n > 0) {
                    buffer[n] = '\0';
                    printf("%s", buffer);
                }
                close(sock);
                exit(0);
                break;

            default:
                printf("Invalid choice, try again.\n");
                continue;
        }

        /* Receive server reply (single read) */
        n = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (n <= 0) {
            printf("Server closed connection.\n");
            break;
        }
        buffer[n] = '\0';
        printf("%s", buffer);

        /* Feedback path: server asked for feedback: */
        if (strstr(buffer, "Enter your feedback") != NULL) {
            char feedback[BUFFER_SIZE];
            if (!fgets(feedback, sizeof(feedback), stdin)) feedback[0] = '\0';
            feedback[strcspn(feedback, "\n")] = '\0';
            send(sock, feedback, strlen(feedback), 0);

            n = recv_and_print(sock, buffer, sizeof(buffer));
            if (n <= 0) break;
        }

        // /* Logged out: break client-side loop so user can login again or exit */
        // if (strstr(buffer, "logged out") != NULL) {
        //     printf("You were logged out by server.\n");
        //     break;
        // }

        /* Server says goodbye -> exit */
        if (strstr(buffer, "Goodbye") != NULL) {
            close(sock);
            exit(0);
        }

        printf("\n");
    }

    close(sock);
    return 0;
}
