/* Tests for ecbuff */

#include "ecbuff.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#ifdef ECB_THREAD_MULTI
#include <pthread.h>
#include <unistd.h>
#endif

#define ECBT_ELEM_CNT ((ECBT_BUFF_SIZ / ECBT_ELEM_SIZ) - 1)

ecbuff* ecbt_new(ECB_UINT_T total_size, ECB_UINT_T element_size);
void ecbt_delete(ecbuff* buff);
void ecbt_verify_stats(ecbuff* buff, ECB_UINT_T used_elements);
uint8_t ecbt_val_next(uint8_t lastval);
void ecbt_write(ecbuff* buff, uint8_t* write_count, ECB_UINT_T num);
void ecbt_write_over(ecbuff* buff, uint8_t* write_count, uint8_t* read_count, ECB_UINT_T num);
void ecbt_write_drop(ecbuff* buff, uint8_t* write_count, ECB_UINT_T num);
void ecbt_read(ecbuff* buff, uint8_t* read_count, ECB_UINT_T num);
void ecbt_read_empty(ecbuff* buff, ECB_UINT_T num);
void ecbt_test_st_basic(ECB_UINT_T count);
void ecbt_test_st_rand(ECB_UINT_T count);
void ecbt_test_st_rand_over(ECB_UINT_T count);
void ecbt_test_st_rand_drop(ECB_UINT_T count);
void ecbt_test_mt_basic(ECB_UINT_T count);
void* ecbt_mt_source_basic(void* buff);
void* ecbt_mt_sink_basic(void* buff);
void ecbt_test_mt_drop(ECB_UINT_T count);
void* ecbt_mt_source_drop(void* buff);
void* ecbt_mt_sink_drop(void* buff);

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
#if defined(ECB_THREAD_SINGLE)
    ecbt_test_st_basic(1337);
    ecbt_test_st_rand(1337);
#endif
#if defined(ECB_THREAD_SINGLE) && defined(ECB_WRITE_OVERWRITE)
    ecbt_test_st_rand_over(1337);
#endif
#if defined(ECB_THREAD_SINGLE) && defined(ECB_WRITE_DROP)
    ecbt_test_st_rand_drop(1337);
#endif

#if defined(ECB_THREAD_MULTI) && !defined(ECB_WRITE_DROP)
    ecbt_test_mt_basic(13);
#endif
#if defined(ECB_THREAD_MULTI) && defined(ECB_WRITE_DROP)
    ecbt_test_mt_drop(13);
#endif
    return 0;
}

#if defined(ECB_THREAD_MULTI)
void ecbt_test_mt_basic(ECB_UINT_T count)
{
    ecbuff* buff = ecbt_new(ECBT_BUFF_SIZ, ECBT_ELEM_SIZ);

    for(ECB_UINT_T i = 0; i < count; i++)
    {
        pthread_t threads[2];
        int ret[2] = {1, 1};
        if(pthread_create(&threads[0], NULL, ecbt_mt_source_basic, (void*)buff))
        {
            printf("Failed to spawn source thread!\n");
            assert(false);
            return;
        }

        if(pthread_create(&threads[1], NULL, ecbt_mt_sink_basic, (void*)buff))
        {
            printf("Failed to spawn sink thread!\n");
            assert(false);
            return;
        }

        pthread_join(threads[0], (void*)&ret[0]);
        pthread_join(threads[1], (void*)&ret[1]);
        if(!ret[0] || !ret[1])
        {
            assert(false);
            return;
        }
    }

    pthread_exit(NULL);
    ecbt_delete(buff);
}

void ecbt_test_mt_drop(ECB_UINT_T count)
{
    ecbuff* buff = ecbt_new(ECBT_BUFF_SIZ, ECBT_ELEM_SIZ);

    for(ECB_UINT_T i = 0; i < count; i++)
    {
        pthread_t threads[2];
        int ret[2] = {1, 1};
        ecbuff_init(buff, ECBT_BUFF_SIZ, ECBT_ELEM_SIZ);
        if(pthread_create(&threads[0], NULL, ecbt_mt_source_drop, (void*)buff))
        {
            printf("Failed to spawn source thread!\n");
            assert(false);
            return;
        }

        if(pthread_create(&threads[1], NULL, ecbt_mt_sink_drop, (void*)buff))
        {
            printf("Failed to spawn sink thread!\n");
            assert(false);
            return;
        }

        pthread_join(threads[0], (void*)&ret[0]);
        pthread_join(threads[1], (void*)&ret[1]);
        if(!ret[0] || !ret[1])
        {
            assert(false);
            return;
        }
    }

    pthread_exit(NULL);

    ecbt_delete(buff);
}

void* ecbt_mt_source_basic(void* buff)
{
    uint8_t write_value[ECBT_ELEM_SIZ];
    static uint8_t write_count;
    memset(write_value, 0, ECBT_ELEM_SIZ);

    ecbt_verify_stats(buff, 0);

    for(unsigned int i = 0; i < (ECBT_ELEM_CNT * 10); i++)
    {
        if(ecbuff_is_full(buff))
            while(ecbuff_is_full(buff))
                usleep(1000);
        memcpy(write_value, &write_count, sizeof(write_count));
        ecbuff_write(buff, &write_value);
        write_count = ecbt_val_next(write_count);
    }

    pthread_exit((void*)true);
}

void* ecbt_mt_source_drop(void* buff)
{
    uint8_t write_value[ECBT_ELEM_SIZ];
    uint8_t write_count = 0;
    memset(write_value, 0, ECBT_ELEM_SIZ);

    ecbt_verify_stats(buff, 0);

    for(unsigned int i = 0; i < (ECBT_ELEM_CNT * 10); i++)
    {
        memcpy(write_value, &write_count, sizeof(write_count));
        ecbuff_write(buff, &write_value);
        write_count = ecbt_val_next(write_count);
    }

    pthread_exit((void*)true);
}

void* ecbt_mt_sink_basic(void* buff)
{
    uint8_t read_value[ECBT_ELEM_SIZ];
    static uint8_t expected_read_value;

    for(unsigned int i = 0; i < (ECBT_ELEM_CNT * 10); i++)
    {
        while(ecbuff_is_empty(buff))
            usleep(1000);
        ecbuff_read(buff, read_value);
        if(memcmp(read_value, &expected_read_value, sizeof(expected_read_value)))
        {
            printf("Read unexpected value! (%hhu instead of %hhu)\n", (uint8_t)*read_value, (uint8_t)expected_read_value);
            assert(false);
            pthread_exit((void*)false);
        }
        expected_read_value = ecbt_val_next(expected_read_value);
    }

    pthread_exit((void*)true);    
}

void* ecbt_mt_sink_drop(void* buff)
{
    uint8_t read_value[ECBT_ELEM_SIZ];
    uint8_t expected_read_value = 0;

    for(unsigned int i = 0; i < ECBT_ELEM_CNT; i++)
    {
        while(ecbuff_is_empty(buff))
            usleep(1000);
        ecbuff_read(buff, read_value);
        if(memcmp(read_value, &expected_read_value, sizeof(expected_read_value)))
        {
            printf("Read unexpected value! (%hhu instead of %hhu)\n", (uint8_t)*read_value, (uint8_t)expected_read_value);
            assert(false);
            pthread_exit((void*)false);
        }
        expected_read_value = ecbt_val_next(expected_read_value);
    }

    pthread_exit((void*)true);    
}
#endif

void ecbt_test_st_basic(ECB_UINT_T count)
{
    uint8_t write_count = ecbt_val_next(0);
    uint8_t read_count = write_count;

    ecbuff* buff = ecbt_new(ECBT_BUFF_SIZ, ECBT_ELEM_SIZ);
    for(ECB_UINT_T i = 0; i < count; i++)
    {
        ecbt_write(buff, &write_count, ECBT_ELEM_CNT);
        ecbt_read(buff, &read_count, ECBT_ELEM_CNT);
#if defined(ECB_EXTRA_CHECKS)
        ecbt_read_empty(buff, 1);
#endif
        ecbt_write(buff, &write_count, 1);
        ecbt_write(buff, &write_count, ECBT_ELEM_CNT - 1);
        ecbt_read(buff, &read_count, ECBT_ELEM_CNT);
#if defined(ECB_EXTRA_CHECKS)
        ecbt_read_empty(buff, ECBT_ELEM_CNT);
#endif
        ecbt_write(buff, &write_count, ECBT_ELEM_CNT);
        ecbt_read(buff, &read_count, 1);
        ecbt_read(buff, &read_count, ecbuff_used(buff));
#if defined(ECB_EXTRA_CHECKS)
        ecbt_read_empty(buff, 1);
#endif
    }
    ecbt_delete(buff);
}

void ecbt_test_st_rand(ECB_UINT_T count)
{
    srand(time(NULL));
    uint8_t write_count = ecbt_val_next((ECB_UINT_T)rand());
    uint8_t read_count = write_count;

    ecbuff* buff = ecbt_new(ECBT_BUFF_SIZ, ECBT_ELEM_SIZ);
    for(ECB_UINT_T i = 0; i < count; i++)
    {
        ECB_UINT_T rnd = rand() % (ecbuff_unused(buff) + 1);
        ecbt_write(buff, &write_count, rnd);
        rnd = rand() % (ecbuff_used(buff) + 1);
        ecbt_read(buff, &read_count, rnd);
    }
    ecbt_read(buff, &read_count, ecbuff_used(buff));
#if defined(ECB_EXTRA_CHECKS)
    ecbt_read_empty(buff, 1);
#endif
    ecbt_delete(buff);
}

void ecbt_test_st_rand_over(ECB_UINT_T count)
{
    srand(time(NULL));
    uint8_t write_count = ecbt_val_next((ECB_UINT_T)rand());
    uint8_t read_count = write_count;

    ecbuff* buff = ecbt_new(ECBT_BUFF_SIZ, ECBT_ELEM_SIZ);
    for(ECB_UINT_T i = 0; i < count; i++)
    {
        ECB_UINT_T rnd = rand() % (ECBT_ELEM_CNT * 10);
        ecbt_write_over(buff, &write_count, &read_count, rnd);
        rnd = rand() % (ecbuff_used(buff) + 1);
        ecbt_read(buff, &read_count, rnd);
    }
    ecbt_read(buff, &read_count, ecbuff_used(buff));
    ecbt_delete(buff);
}

void ecbt_test_st_rand_drop(ECB_UINT_T count)
{
    srand(time(NULL));
    uint8_t write_count = ecbt_val_next((ECB_UINT_T)rand());
    uint8_t read_count = write_count;

    ecbuff* buff = ecbt_new(ECBT_BUFF_SIZ, ECBT_ELEM_SIZ);
    for(ECB_UINT_T i = 0; i < count; i++)
    {
        ECB_UINT_T rnd = rand() % (ECBT_ELEM_CNT * 10);
        ecbt_write_drop(buff, &write_count, rnd);
        rnd = rand() % (ecbuff_used(buff) + 1);
        ecbt_read(buff, &read_count, rnd);
        ecbt_read(buff, &read_count, ecbuff_used(buff));
        read_count = write_count;
    }
    ecbt_delete(buff);
}

uint8_t ecbt_val_next(uint8_t lastval)
{
    return (lastval + 31337);
}

ecbuff* ecbt_new(ECB_UINT_T total_size, ECB_UINT_T element_size)
{
    ecbuff* buff = malloc(sizeof(ecbuff) + total_size);
    assert(buff);

    ecbuff_init(buff, total_size, element_size);
    ecbt_verify_stats(buff, 0);
    return buff;
}

void ecbt_delete(ecbuff* buff)
{
    assert(buff);
    free(buff);
}

void ecbt_verify_stats(ecbuff* buff, ECB_UINT_T used_elements)
{
    ECB_UINT_T ret = ecbuff_unused(buff);
    if(ret != ECBT_ELEM_CNT - used_elements)
    {
        printf("ecbuff_unused() reports wrong value! (%u expected %u)\n",
                ret, (unsigned int)(ECBT_ELEM_CNT - used_elements));
        assert(false);
        return;
    }

    ret = ecbuff_used(buff);
    if(ret != used_elements)
    {
        printf("ecbuff_used() reports wrong value! (%u expected %u)\n",
                ret, used_elements);
        assert(false);
        return;
    }

    ret = ecbuff_is_empty(buff);
    if(ret != (used_elements == 0))
    {
        printf("ecbuff_is_empty() reports wrong value! (%u expected %u)\n",
                ret, (used_elements == 0));
        assert(false);
        return;
    }

    ret = ecbuff_is_full(buff);
    if(ret != (used_elements == ECBT_ELEM_CNT))
    {
        printf("ecbuff_is_full() reports wrong value! (%u expected %u)\n",
                ret, (used_elements == ECBT_ELEM_CNT));
        assert(false);
        return;
    }
}

void ecbt_write(ecbuff* buff, uint8_t* write_count, ECB_UINT_T num)
{
    uint8_t write_value[ECBT_ELEM_SIZ];
    memset(write_value, 0, ECBT_ELEM_SIZ);

    ECB_UINT_T oldlevel = ecbuff_used(buff);
    for(ECB_UINT_T i = 0; i < num; i++)
    {
        assert(!ecbuff_is_full(buff));
        memcpy(write_value, write_count, sizeof(*write_count));
#if defined(ECB_EXTRA_CHECKS) && !defined(ECB_DIRECT_ACCESS)
        assert(ecbuff_write(buff, write_value));
#elif !defined(ECB_EXTRA_CHECKS) && !defined(ECB_DIRECT_ACCESS)
        ecbuff_write(buff, write_value);
#elif defined(ECB_DIRECT_ACCESS)
        void* ptr = ecbuff_write_alloc(buff);
        assert(ptr);
        memcpy(ptr, write_value, ECBT_ELEM_SIZ);
#if defined(ECB_EXTRA_CHECKS)
        assert(ecbuff_write_enqueue(buff));
#else
        ecbuff_write_enqueue(buff);
#endif
#endif
        *write_count = ecbt_val_next(*write_count);
    }
    ecbt_verify_stats(buff, oldlevel + num);
}

void ecbt_read(ecbuff* buff, uint8_t* read_count, ECB_UINT_T num)
{
    uint8_t read_value[ECBT_ELEM_SIZ];
    ECB_UINT_T oldlevel = ecbuff_used(buff);

    for(ECB_UINT_T i = 0; i < num; i++)
    {
        assert(!ecbuff_is_empty(buff));
#if defined(ECB_EXTRA_CHECKS) && !defined(ECB_DIRECT_ACCESS)
        assert(ecbuff_read(buff, read_value));
#elif !defined(ECB_EXTRA_CHECKS) && !defined(ECB_DIRECT_ACCESS)
        ecbuff_read(buff, read_value);
#elif defined(ECB_DIRECT_ACCESS)
        void* ptr = ecbuff_read_dequeue(buff);
        assert(ptr);
        memcpy(read_value, ptr, ECBT_ELEM_SIZ);
#if defined(ECB_EXTRA_CHECKS)
        assert(ecbuff_read_free(buff));
#else
        ecbuff_read_free(buff);
#endif
#endif
        if(memcmp(read_value, read_count, sizeof(*read_count)))
        {
            printf("Read unexpected value! (%hhu instead of %hhu)\n", *read_value, *read_count);
            assert(false);
            return;
        }
        *read_count = ecbt_val_next(*read_count);
    }
    ecbt_verify_stats(buff, oldlevel - num);
}

void ecbt_read_empty(ecbuff* buff, ECB_UINT_T num)
{
    ecbt_verify_stats(buff, 0);

    for(ECB_UINT_T i = 0; i < num; i++)
    {
        assert(ecbuff_is_empty(buff));
#if defined(ECB_EXTRA_CHECKS) && !defined(ECB_DIRECT_ACCESS)
        uint8_t read_value[ECBT_ELEM_SIZ];
        assert(!ecbuff_read(buff, read_value));
#elif defined(ECB_EXTRA_CHECKS) && defined(ECB_DIRECT_ACCESS)
        void* ptr = ecbuff_read_dequeue(buff);
        assert(!ptr);
        assert(!ecbuff_read_free(buff));
#else
          printf("Empty read test isn't supported without ECB_EXTRA_CHECKS!\n");
          assert(false);
          return;
#endif
    }
    ecbt_verify_stats(buff, 0);
}

void ecbt_write_over(ecbuff* buff, uint8_t* write_count, uint8_t* read_count, ECB_UINT_T num)
{
    uint8_t write_value[ECBT_ELEM_SIZ];
    memset(write_value, 0, ECBT_ELEM_SIZ);

    if(!ecbuff_is_full(buff))
        ecbt_write(buff, write_count, ecbuff_unused(buff));

    for(ECB_UINT_T i = 0; i < num; i++)
    {
        memcpy(write_value, write_count, sizeof(*write_count));
#if defined(ECB_EXTRA_CHECKS) && !defined(ECB_DIRECT_ACCESS)
        assert(!ecbuff_write(buff, write_value));
#elif !defined(ECB_EXTRA_CHECKS) && !defined(ECB_DIRECT_ACCESS)
        ecbuff_write(buff, write_value);
#elif defined(ECB_DIRECT_ACCESS)
        void* ptr = ecbuff_write_alloc(buff);
        assert(ptr);
        memcpy(ptr, write_value, ECBT_ELEM_SIZ);
#if defined(ECB_EXTRA_CHECKS)
        assert(!ecbuff_write_enqueue(buff));
#else
        ecbuff_write_enqueue(buff);
#endif
#endif
        ecbt_verify_stats(buff, ECBT_ELEM_CNT);
        *write_count = ecbt_val_next(*write_count);
        *read_count = ecbt_val_next(*read_count);
    }
}

void ecbt_write_drop(ecbuff* buff, uint8_t* write_count, ECB_UINT_T num)
{
    uint8_t write_value[ECBT_ELEM_SIZ];
    memset(write_value, 0, ECBT_ELEM_SIZ);

    if(!ecbuff_is_full(buff))
        ecbt_write(buff, write_count, ecbuff_unused(buff));

    for(ECB_UINT_T i = 0; i < num; i++)
    {
        memcpy(write_value, write_count, sizeof(*write_count));
#if defined(ECB_EXTRA_CHECKS) && !defined(ECB_DIRECT_ACCESS)
        assert(!ecbuff_write(buff, write_value));
#elif !defined(ECB_EXTRA_CHECKS) && !defined(ECB_DIRECT_ACCESS)
        ecbuff_write(buff, write_value);
#elif defined(ECB_DIRECT_ACCESS)
        void* ptr = ecbuff_write_alloc(buff);
        assert(ptr);
        memcpy(ptr, write_value, ECBT_ELEM_SIZ);
#if defined(ECB_EXTRA_CHECKS)
        assert(!ecbuff_write_enqueue(buff));
#else
        ecbuff_write_enqueue(buff);
#endif
#endif
        ecbt_verify_stats(buff, ECBT_ELEM_CNT);
        *write_count = ecbt_val_next(*write_count);
    }
}