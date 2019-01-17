/*
 * emutex implements a minimalistic mutex in standard C11. While written for
 * embedded systems it is fully portable and can be used on any system
 * supporting C11. (e.g. on POSIX/x86 as the included tests do.)
 * 
 * emutex_trylock returns false if it fails to allocate the mutex, while
 * emutex_lock implements a spin lock and blocks until the mutex is aquired.
 *
 * WARNING: As of GCC 8.2.0 the code generated for ARMv6-m (e.g. Cortex-M0)
 * 			does not look safe, since the architecture doesn't support the
 * 			LDREX/STREX instructions and GCC doesn't generate code to protect
 * 			against interrupts. Though ARMv7-m (e.g. Cortex-M3 and onwards)
 * 			has been tested and is fully supported.
 * 
 * Written by Elias Oenal <emutex@eliasoenal.com>, released as public domain.
 */

#ifndef EMUTEX_H
#define EMUTEX_H

#include <stdbool.h>
#include <stdatomic.h>
typedef atomic_flag emutex;

void emutex_init(emutex* const restrict mutex);
bool emutex_trylock(emutex* const restrict mutex);
void emutex_lock(emutex* const restrict mutex);
void emutex_unlock(emutex* const restrict mutex);

#endif /* EMUTEX_H */
