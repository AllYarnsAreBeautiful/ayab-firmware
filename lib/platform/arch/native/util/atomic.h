#ifndef NATIVE_ATOMIC_H
#define NATIVE_ATOMIC_H

#include <stdint.h>

// Native tests run single-threaded, so an atomic block only needs to preserve
// the production code's control-flow shape.
#define ATOMIC_BLOCK(type) \
  for (type, __ToDo = 1; __ToDo; __ToDo = 0)

#define ATOMIC_RESTORESTATE \
  uint8_t __atomic_restore_state __attribute__((unused)) = 0

#endif  // NATIVE_ATOMIC_H
