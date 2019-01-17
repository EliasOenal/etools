/* 
 * Tests for emutex
 *
 * Written by Elias Oenal <emutex@eliasoenal.com>, released as public domain.
 */

#include "emutex.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#ifdef EMUTEX_THREAD_MULTI
#include <pthread.h>
#include <unistd.h>
void emt_test_mt(uint32_t count);
void* emt_mt_thread(void* m);
#endif

void emt_test_st(uint32_t count);


int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

#if defined(EMUTEX_THREAD_SINGLE)
    emt_test_st(31337);
#endif

#if defined(EMUTEX_THREAD_MULTI)
    emt_test_mt(13);
#endif

	return 0;
}

#ifdef EMUTEX_THREAD_MULTI

#define NUM_THREADS 20
#define COUNTER 30000

struct shared{
	emutex* m; uint64_t* i;
};
void emt_test_mt(uint32_t count)
{
	for(uint32_t i = 0; i < count; i++)
    {
		emutex m;
		uint64_t shared_counter = 0;
		struct shared s = {&m, &shared_counter};
        pthread_t threads[NUM_THREADS];

		emutex_init(&m);
		emutex_lock(&m);

		for(uint32_t t = 0; t < NUM_THREADS; t++)
		{
			if(pthread_create(&threads[t], NULL, emt_mt_thread, (void*)&s))
			{
				printf("Failed to spawn thread!\n");
				assert(false);
				return;
			}
		}

		emutex_unlock(&m);

		for(uint32_t t = 0; t < NUM_THREADS; t++)
		{
			pthread_join(threads[t], NULL);
		}
		assert(shared_counter == (NUM_THREADS * COUNTER));
    }

    pthread_exit(NULL);
}

void* emt_mt_thread(void* v)
{
	struct shared* s = (struct shared*)v;
	for(uint32_t i = 0; i < COUNTER; i++)
	{
		emutex_lock(s->m);
		(*(s->i))++;
		emutex_unlock(s->m);
	}

    pthread_exit((void*)true);    
}

#endif /* EMUTEX_THREAD_MULTI */

void emt_test_st(uint32_t count)
{
	emutex m;
	emutex_init(&m);

	for (uint32_t i = 0; i < count; i++)
	{
		emutex_lock(&m);
		emutex_unlock(&m);
		emutex_lock(&m);
		assert(!emutex_trylock(&m));
		assert(!emutex_trylock(&m));
		assert(!emutex_trylock(&m));
		emutex_unlock(&m);
		assert(emutex_trylock(&m));
		emutex_unlock(&m);
		assert(emutex_trylock(&m));
		emutex_unlock(&m);
	}
}
