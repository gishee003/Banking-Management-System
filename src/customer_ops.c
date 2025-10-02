#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <time.h>
#include "../include/common.h"
#include "../include/customer_ops.h"
#include "../include/database.h"   


void handle_view_balance(int sock, struct Customer *c) {
    char resp[BUFFER_SIZE];
    snprintf(resp, sizeof(resp), "Your balance: %.2f\n", c->balance);
    send(sock, resp, strlen(resp), 0);
}

void handle_deposit(int sock, struct Customer *c, const char* cmd) {
   // send(sock, "Deposit selected (not implemented yet)\n", 40, 0);
    double amount = 0.0;
    if (sscanf(cmd, "DEPOSIT %lf", &amount) != 1) {
        send(sock, "Usage: DEPOSIT <amount>\n", 24, 0);
        return;
    }

    if (amount <= 0.0) {
        send(sock, "Invalid amount. Must be positive.\n", 34, 0);
        return;
    }

    double new_balance = 0.0;
    if (!deposit_to_account(c->id, amount, &new_balance)) {
        send(sock, "Deposit failed due to server error.\n", 36, 0);
        return;
    }

    // update in-memory struct for this session
    c->balance = new_balance;

    // log the transaction (best-effort)
    append_transaction_log(c->id, "DEPOSIT", amount, -1);

    char resp[BUFFER_SIZE];
    snprintf(resp, sizeof(resp), "Deposit successful. New balance: %.2f\n", new_balance);
    send(sock, resp, strlen(resp), 0);
}

void handle_withdraw(int sock, struct Customer *c, const char *cmd) {
	double amount = 0.0;
    if (sscanf(cmd, "WITHDRAW %lf", &amount) != 1) {
        send(sock, "Usage: WITHDRAW <amount>\n", 25, 0);
        return;
    }

    if (amount <= 0.0) {
        send(sock, "Invalid amount. Must be positive.\n", 34, 0);
        return;
    }

    double new_balance = 0.0;
    int res = withdraw_from_account(c->id, amount, &new_balance);
    if (res == 0) {
        send(sock, "Withdraw failed due to server error.\n", 37, 0);
        return;
    } else if (res == -1) {
        send(sock, "Insufficient balance.\n", 22, 0);
        return;
    }

    // update in-memory session copy
    c->balance = new_balance;

    // log the transaction
    append_transaction_log(c->id, "WITHDRAW", amount, -1);

    char resp[BUFFER_SIZE];
    snprintf(resp, sizeof(resp), "Withdrawal successful. New balance: %.2f\n", new_balance);
    send(sock, resp, strlen(resp), 0);
}

void handle_transfer(int sock, struct Customer *c,const char *cmd) {
   // send(sock, "Transfer selected (not implemented yet)\n", 41, 0
 
    int target_id;
    double amount;

    if (sscanf(cmd, "TRANSFER %d %lf", &target_id, &amount) != 2) {
        send(sock, "Usage: TRANSFER <target_id> <amount>\n", 38, 0);
        return;
    }

    if (target_id == c->id) {
        send(sock, "Cannot transfer to yourself.\n", 29, 0);
        return;
    }

    double new_balance = 0.0;
    int res = transfer_funds(c->id, target_id, amount, &new_balance);

    if (res == -1) {
        send(sock, "Insufficient balance.\n", 22, 0);
        return;
    } else if (res == 0) {
        send(sock, "Transfer failed.\n", 17, 0);
        return;
    }

    // update session copy
    c->balance = new_balance;

    // log transaction
    append_transaction_log(c->id, "TRANSFER", amount, target_id);

    char resp[BUFFER_SIZE];
    snprintf(resp, sizeof(resp),
             "Transfer successful. Sent %.2f to Customer %d. New balance: %.2f\n",
             amount, target_id, new_balance);
    send(sock, resp, strlen(resp), 0);
}

void handle_apply_loan(int sock, struct Customer *c, const char *cmd) {
    double amount, interest;
    int tenure;

    if (sscanf(cmd, "LOAN %lf %d %lf", &amount, &tenure, &interest) != 3) {
        send(sock, "Usage: LOAN <amount> <tenure_months> <interest_rate>\n", 52, 0);
        return;
    }

    addLoanRequest(c->id, amount, tenure, interest);

    char resp[BUFFER_SIZE];
    snprintf(resp, sizeof(resp), 
             "Loan request submitted. Amount: %.2f, Tenure: %d months, Interest: %.2f%%\n",
             amount, tenure, interest);
    send(sock, resp, strlen(resp), 0);
}


void handle_change_password(int sock, struct Customer *c, const char *cmd){
    char new_pass[50];
    if (sscanf(cmd, "CHANGE_PASSWORD %49s", new_pass) != 1) {
        send(sock, "Usage: CHANGE_PASSWORD <new_password>\n", 38, 0);
        return;
    }

    if (!change_password(c->id, new_pass)) {
        send(sock, "Password change failed.\n", 24, 0);
        return;
    }

    strncpy(c->password, new_pass, sizeof(c->password) - 1);

    send(sock, "Password changed successfully.\n", 31, 0);
}

void handle_feedback(int sock, struct Customer *c) {
    char buffer[BUFFER_SIZE];
    int n;

    // Ask for feedback
    send(sock, "Enter your feedback: ", 21, 0);

    // Receive the actual feedback
    n = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (n <= 0) return;
    buffer[n] = '\0';

    FILE *fp = fopen(FEEDBACK_FILE, "a");
    if (!fp) {
        send(sock, "Error saving feedback.\n", 24, 0);
        return;
    }

    time_t now = time(NULL);
    char *timestamp = ctime(&now);
    timestamp[strlen(timestamp) - 1] = '\0'; // remove newline

    fprintf(fp, "CustomerID: %d | User: %s | Time: %s\nFeedback: %s\n\n",
            c->id, c->username, timestamp, buffer);
    fclose(fp);

    send(sock, "Thank you for your feedback!\n", 30, 0);
}

void handle_view_transactions(int sock, struct Customer *c) {
    send(sock, "Transaction history selected (not implemented yet)\n", 51, 0);
}

