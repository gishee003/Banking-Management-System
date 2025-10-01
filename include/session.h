#ifndef SESSION_H
#define SESSION_H

#define SESSION_FILE "data/sessions.dat"

int is_logged_in(int customerId);
void add_session(int customerId);
void remove_session(int customerId);

#endif
