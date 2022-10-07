#include "debug.h"

#include <stdbool.h>

// Convenience

static enum debug_levels {
    DEBUG_INFO,
    DEBUG_VERBOSE,
    DEBUG_DEBUG,
    DEBUG_NONE
} global_debug_level;

void debug_setlevel_INFO(void) { global_debug_level = DEBUG_INFO; }
void debug_setlevel_VERBOSE(void) { global_debug_level = DEBUG_VERBOSE; }
void debug_setlevel_DEBUG(void) { global_debug_level = DEBUG_DEBUG; }
void debug_setlevel_NONE(void) { global_debug_level = DEBUG_NONE; }

bool DBGLVL_INFO(void) { return global_debug_level == DEBUG_INFO; }
bool DBGLVL_VERBOSE(void) { return global_debug_level == DEBUG_VERBOSE; }
bool DBGLVL_DEBUG(void) { return global_debug_level == DEBUG_DEBUG; }
bool DBGLVL_NONE(void) { return global_debug_level == DEBUG_NONE; }
