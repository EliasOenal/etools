/*
 * See emutex.h for further information.
 * 
 * Written by Elias Oenal <emutex@eliasoenal.com>, released as public domain.
 */

#include "emutex.h"

#if defined(EM_ASSERT)
#include <assert.h>
#else
#define NDEBUG
#define assert(x)
#endif

void emutex_init(emutex* const restrict mutex)
{
	assert(mutex);
	emutex_unlock(mutex);
	return;
}

static inline bool emutex_trylock_private(emutex* const restrict mutex)
{
	if(atomic_flag_test_and_set_explicit(mutex, memory_order_acquire))
		return false;
	return true;
}

bool emutex_trylock(emutex* const restrict mutex)
{
	assert(mutex);
	return emutex_trylock_private(mutex);
}

void emutex_lock(emutex* const restrict mutex)
{
	assert(mutex);
	while(!emutex_trylock_private(mutex))
	{
		// You may want to yield here
	}
	return;
}

void emutex_unlock(emutex* const restrict mutex)
{
	assert(mutex);
	atomic_flag_clear_explicit(mutex, memory_order_release);
	return;
}
