#ifndef SESSION_H
#define SESSION_H

#include "common.h"

int  session_load(Session *out);
int  session_save(const Session *s);
void session_clear(void);

#endif /* SESSION_H */
