#pragma once

/**
 * debuglog.h - Simple macro-based logging
 *
 * Usage:
 *   DEBUG_LOG("some message: " << someValue);
 *
 * Then compile with -DDEBUG_LOGGING to enable logs, or omit it to disable.
 */

// If we pass -DDEBUG_LOGGING to the compiler, logs are active.
#ifdef DEBUG_LOGGING
  #include <iostream>
  #define DEBUG_LOG(msg) do { std::cerr << msg << std::endl; } while(0)
#else
  // logs become no-ops
  #define DEBUG_LOG(msg) do {} while(0)
#endif
