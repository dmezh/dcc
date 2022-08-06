#ifndef DEBUG_H
#define DEBUG_H

#include <stdbool.h>

void debug_setlevel_INFO();
void debug_setlevel_VERBOSE();
void debug_setlevel_DEBUG();
void debug_setlevel_NONE();

bool DBGLVL_INFO();
bool DBGLVL_VERBOSE();
bool DBGLVL_DEBUG();
bool DBGLVL_NONE();

#endif
