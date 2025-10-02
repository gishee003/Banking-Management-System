
#ifndef DATABASE_H
#define DATABASE_H

#include "common.h"

typedef struct {
    int loan_id;
    int customer_id;
    double amount;
    int tenure_months;
    double interest_rate;
    char status[20]; // "Pending", "Approved", "Rejected"
} Loan;

int deposit_to_account(int cust_id, double amount, double *new_balance);

void append_transaction_log(int cust_id, const char *type, double amount, int target_id);

int withdraw_from_account(int cust_id, double amount, double *new_balance);

int transfer_funds(int from_id, int to_id, double amount, double *new_balance);


// LOAN FUNCTIONS 

int generateLoanId();

void addLoanRequest(int customer_id, double amount, int tenure, double interest_rate);


//change password

int change_password(int cust_id, const char *new_password);

#endif

