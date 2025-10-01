#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "../include/common.h"

void customer_menu(int sock) {
    int choice;

    while (1) {
        printf("\n===== Customer Menu =====\n");
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
        printf("=========================\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: send(sock, "VIEW_BALANCE", 12, 0); break;
            case 2: send(sock, "DEPOSIT", 7, 0); break;
            case 3: send(sock, "WITHDRAW", 8, 0); break;
            case 4: send(sock, "TRANSFER", 8, 0); break;
            case 5: send(sock, "APPLY_LOAN", 10, 0); break;
            case 6: send(sock, "CHANGE_PASS", 11, 0); break;
            case 7: send(sock, "FEEDBACK", 8, 0); break;
            case 8: send(sock, "TRANSACTIONS", 12, 0); break;
            case 9:
                send(sock, "LOGOUT", 6, 0);
                printf("Logging out...\n");
                return;
            case 10:
                printf("Exiting program...\n");
                close(sock);
                exit(0);
            default:
                printf("Invalid choice! Try again.\n");
        }
    }
}

