#ifndef CUSTOMER_OPS_H
#define CUSTOMER_OPS_H

#include "common.h" 

void handle_view_balance(int sock, struct Customer *c);
void handle_deposit(int sock, struct Customer *c, const char* cmd);
void handle_withdraw(int sock, struct Customer *c,const char* cmd);
void handle_transfer(int sock, struct Customer *c,const char* cmd);
void handle_apply_loan(int sock, struct Customer *c,const char* cmd);
void handle_change_password(int sock, struct Customer *c,const char* cmd);
void handle_feedback(int sock, struct Customer *c);
void handle_view_transactions(int sock, struct Customer *c);

#endif

