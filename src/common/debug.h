#ifndef DEBUG_H
#define DEBUG_H

#include <stdbool.h>

void debug_setlevel_INFO(void);
void debug_setlevel_VERBOSE(void);
void debug_setlevel_DEBUG(void);
void debug_setlevel_NONE(void);

bool DBGLVL_INFO(void);
bool DBGLVL_VERBOSE(void);
bool DBGLVL_DEBUG(void);
bool DBGLVL_NONE(void);

#endif
