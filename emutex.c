/*
 * See emutex.h for further information.
 * 
 * Written by Elias Oenal <emutex@eliasoenal.com>, released as public domain.
 */

#include "emutex.h"

void emutex_init(emutex* const restrict mutex)
{
	emutex_unlock(mutex);
	return;
}

static inline bool emutex_trylock_private(emutex* const restrict mutex)
{
	if(atomic_flag_test_and_set(mutex))
		return false;
	return true;
}

bool emutex_trylock(emutex* const restrict mutex)
{
	return emutex_trylock_private(mutex);
}

void emutex_lock(emutex* const restrict mutex)
{
	while(!emutex_trylock_private(mutex))
	{
		// You may want to yield here
	}
	return;
}

void emutex_unlock(emutex* const restrict mutex)
{
	atomic_flag_clear(mutex);
	return;
}
