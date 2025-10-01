
#ifndef DATABASE_H
#define DATABASE_H

#include "common.h"

int deposit_to_account(int cust_id, double amount, double *new_balance);

void append_transaction_log(int cust_id, const char *type, double amount, int target_id);

int withdraw_from_account(int cust_id, double amount, double *new_balance);

int transfer_funds(int from_id, int to_id, double amount, double *new_balance);

#endif

