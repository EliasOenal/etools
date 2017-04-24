/* See ecbuff.h and ecbuff_cfg.h for further information */

#include "ecbuff.h"

#if defined(ECB_ASSERT)
#include <assert.h>
#include <limits.h>
#else
#define NDEBUG
#define assert(x)
#endif

#if defined(ECB_WRITE_DROP)
#include <stddef.h>
#endif

/* Turn static assertions into runtime checks on older compilers */
#if !defined(static_assert)
#define static_assert(x, y) assert(x)
#endif

/* Verify configuration */
#if defined ECB_THREAD_SINGLE
#if defined(ECB_THREAD_MULTI) || defined(ECB_THREAD_BARRIER) || defined(ECB_THREAD_VOLATILE)
#error ECB_THREAD_SINGLE and other ECB_THREAD options are mutually exclusive!
#endif
#else /* !ECB_THREAD_SINGLE */
#if defined(ECB_THREAD_MULTI)
#if !(defined(ECB_THREAD_BARRIER) ^ defined(ECB_THREAD_VOLATILE))
#error ECB_THREAD_MULTI requires ECB_THREAD_BARRIER or ECB_THREAD_VOLATILE
#endif
#else /* !ECB_THREAD_MULTI */
#error ECB_THREAD_SINGLE or ECB_THREAD_MULTI required!
#endif /* ECB_THREAD_MULTI */
#endif /* ECB_THREAD_SINGLE */

#if defined(ECB_WRITE_OVERWRITE)
#if defined(ECB_THREAD_MULTI)
#error ECB_WRITE_OVERWRITE is not thread-safe and thus mutually exclusive with ECB_THREAD_MULTI!
#endif /* ECB_THREAD_MULTI */
#if defined(ECB_WRITE_DROP)
#error ECB_WRITE_OVERWRITE and ECB_WRITE_DROP are mutually exclusive!
#endif /* ECB_WRITE_DROP */
#endif /* ECB_WRITE_OVERWRITE */

#if defined(ECB_EXTRA_CHECKS) && !(defined(ECB_WRITE_OVERWRITE) ^ defined(ECB_WRITE_DROP))
#error ECB_EXTRA_CHECKS requires ECB_WRITE_DROP or ECB_WRITE_OVERWRITE to be defined!
#endif

#if defined(ECB_THREAD_BARRIER)
/* TODO: Implement C11 barrier */
#if defined(__GNUC__) || defined(__clang__)
#define FULL_MEMORY_BARRIER() __sync_synchronize()
#else
#error ECB_THREAD_BARRIER does not support this compiler!
#endif /* Compiler support */
#else /* ECB_THREAD_SINGLE or ECB_THREAD_VOLATILE */
#define FULL_MEMORY_BARRIER()
#endif /* ECB_THREAD_BARRIER */

#define ECB_MODULUS(x, y) (x % y)
#define ECB_CHECK_ALIGN(ptr, req) (((uintptr_t)ptr) % req == 0)


void ecbuff_init(ecbuff* const restrict rb, const ECB_UINT_T total_size, const ECB_UINT_T element_size)
{
    /* Check type limits */
    static_assert(ECB_UINT_MAX >= ECB_ATOMIC_MAX, "ECB_UINT_MAX is required to at least be ECB_ATOMIC_MAX");
    /* Sanity-check parameters */
    assert(total_size <= ECB_ATOMIC_MAX);
    assert(total_size);
    assert(element_size);
    assert(ECB_MODULUS(total_size, element_size) == 0);
    assert(total_size >= element_size * 2);
    assert(rb);
#if defined(ECB_ELEM_ALIGN)
    assert(ECB_CHECK_ALIGN(element_size, ECB_ELEM_ALIGN));
    assert(ECB_CHECK_ALIGN(&rb->elems[0], ECB_ELEM_ALIGN));
    assert(ECB_CHECK_ALIGN(&rb->elems[0] + element_size, ECB_ELEM_ALIGN));
#endif
    rb->total_size = total_size;
    rb->element_size = element_size;
    rb->rp = 0;
    rb->wp = 0;
    FULL_MEMORY_BARRIER();
}

static inline bool ecbuff_is_full_private(const ECB_UINT_T total_size, const ECB_UINT_T element_size,
                                            const ECB_UINT_T rp, const ECB_UINT_T wp)
{
    assert(total_size);
    assert(element_size);
    return ECB_MODULUS((wp + element_size), total_size) == rp;
}

bool ecbuff_is_full(const ecbuff* const restrict rb)
{
    assert(rb);
    FULL_MEMORY_BARRIER();
    ECB_UINT_T total_size = rb->total_size;
    ECB_UINT_T element_size = rb->element_size;
    ECB_UINT_T rp = rb->rp;
    ECB_UINT_T wp = rb->wp;

    return ecbuff_is_full_private(total_size, element_size, rp, wp);
}

static inline bool ecbuff_is_empty_private(const ECB_UINT_T rp, const ECB_UINT_T wp)
{
    return wp == rp;
}

bool ecbuff_is_empty(const ecbuff* const restrict rb)
{
    assert(rb);
    FULL_MEMORY_BARRIER();
    ECB_UINT_T rp = rb->rp;
    ECB_UINT_T wp = rb->wp;
    return ecbuff_is_empty_private(rp, wp);
}

#if defined(ECB_THREAD_VOLATILE)
#define ECB_MEMCPY ecbuff_vmemcpy
static inline void ecbuff_vmemcpy(volatile void* restrict dest, volatile const void* restrict src, size_t len)
{
    while(len--)
        *(volatile char* restrict)dest++ = *(volatile const char* restrict)src++;
}
#else
#include <string.h>
#define ECB_MEMCPY memcpy
#endif

ECB_VOID_BOOL_T ecbuff_write(ecbuff* const restrict rb, const void* const restrict element)
{
    assert(rb);
    assert(element);
    ECB_UINT_T total_size = rb->total_size;
    ECB_UINT_T element_size = rb->element_size;
    ECB_UINT_T wp = rb->wp;
#if defined(ECB_WRITE_DROP) || defined(ECB_WRITE_OVERWRITE) || defined(ECB_ASSERT)
    FULL_MEMORY_BARRIER();
    ECB_UINT_T rp = rb->rp;
#endif

#if defined(ECB_WRITE_DROP)
    if(ecbuff_is_full_private(total_size, element_size, rp, wp))
#if defined(ECB_EXTRA_CHECKS)
        return false;
#else
        return;
#endif
#endif

#if !defined(ECB_WRITE_OVERWRITE)
    assert(!ecbuff_is_full_private(total_size, element_size, rp, wp));
#endif /* ECB_WRITE_OVERWRITE */

    ECB_MEMCPY(&rb->elems[wp], element, element_size);
    FULL_MEMORY_BARRIER();
    wp = ECB_MODULUS((wp + element_size), total_size);
    rb->wp = wp;

#if defined(ECB_WRITE_OVERWRITE)
    if(ecbuff_is_empty_private(rp, wp))
    {   /* Apparently we have just overwritten an element.
         * Move rp to drop the oldest element. */
        rb->rp = ECB_MODULUS((rp + element_size), total_size);
#if defined(ECB_EXTRA_CHECKS)
        return false;
#endif /* ECB_EXTRA_CHECKS */
    }
#endif /* ECB_WRITE_OVERWRITE */
    FULL_MEMORY_BARRIER();
#if defined(ECB_EXTRA_CHECKS)
    return true;
#endif /* ECB_EXTRA_CHECKS */
}

ECB_VOID_BOOL_T ecbuff_read(ecbuff* const restrict rb, void* const restrict element)
{
    assert(rb);
    assert(element);
    FULL_MEMORY_BARRIER();
    ECB_UINT_T rp = rb->rp;
#if defined(ECB_ASSERT) || defined(ECB_EXTRA_CHECKS)
    ECB_UINT_T wp = rb->wp;
#if defined(ECB_ASSERT) && !defined(ECB_EXTRA_CHECKS)
    assert(!ecbuff_is_empty_private(rp, wp));
#elif defined(ECB_EXTRA_CHECKS)
    if(ecbuff_is_empty_private(rp, wp))
        return false;
#endif
#endif
    ECB_UINT_T total_size = rb->total_size;
    ECB_UINT_T element_size = rb->element_size;

    ECB_MEMCPY(element, &rb->elems[rp], element_size);
    FULL_MEMORY_BARRIER();
    rb->rp = ECB_MODULUS((rp + element_size), total_size);
    FULL_MEMORY_BARRIER();
#if defined(ECB_EXTRA_CHECKS)
    return true;
#endif
}

ECB_UINT_T ecbuff_used(const ecbuff* const restrict rb)
{
    assert(rb);
    FULL_MEMORY_BARRIER();
    ECB_UINT_T total_size = rb->total_size;
    ECB_UINT_T element_size = rb->element_size;
    ECB_UINT_T rp = rb->rp;
    ECB_UINT_T wp = rb->wp;

    return ECB_MODULUS((total_size + wp - rp), total_size) / element_size;
}

ECB_UINT_T ecbuff_unused(const ecbuff* const restrict rb)
{
    assert(rb);
    FULL_MEMORY_BARRIER();
    ECB_UINT_T total_size = rb->total_size;
    ECB_UINT_T element_size = rb->element_size;
    ECB_UINT_T rp = rb->rp;
    ECB_UINT_T wp = rb->wp;

    return ECB_MODULUS((total_size - element_size + rp - wp), total_size) / element_size;
}

#ifdef ECB_DIRECT_ACCESS
ECB_VOLATILE_T void* ecbuff_write_alloc(ecbuff* const restrict rb)
{
    assert(rb);
    ECB_UINT_T wp = rb->wp;
#if defined(ECB_ASSERT) && !defined(ECB_WRITE_OVERWRITE) && !defined(ECB_WRITE_DROP)
    FULL_MEMORY_BARRIER();
    ECB_UINT_T total_size = rb->total_size;
    ECB_UINT_T element_size = rb->element_size;
    ECB_UINT_T rp = rb->rp;
    assert(!ecbuff_is_full_private(total_size, element_size, rp, wp));
#endif

    return &rb->elems[wp];
}

ECB_VOID_BOOL_T ecbuff_write_enqueue(ecbuff* const restrict rb)
{
    assert(rb);
    FULL_MEMORY_BARRIER();
    ECB_UINT_T total_size = rb->total_size;
    ECB_UINT_T element_size = rb->element_size;
    ECB_UINT_T wp = rb->wp;
#if defined(ECB_WRITE_DROP) || defined(ECB_WRITE_OVERWRITE)
    ECB_UINT_T rp = rb->rp;
#endif
#if defined(ECB_WRITE_DROP)
    if(ecbuff_is_full_private(total_size, element_size, rp, wp))
#if defined(ECB_EXTRA_CHECKS)
        return false;
#else
        return;
#endif
#endif
    wp = ECB_MODULUS((wp + element_size), total_size);
    rb->wp = wp;
#if defined(ECB_WRITE_OVERWRITE)
    if(ecbuff_is_empty_private(rp, wp))
    {   /* Apparently we have just overwritten an element.
         * Move rp to drop the oldest element. */
        rb->rp = ECB_MODULUS((rp + element_size), total_size);
#if defined(ECB_EXTRA_CHECKS)
        return false;
#endif /* ECB_EXTRA_CHECKS */
    }
#endif /* ECB_WRITE_OVERWRITE */
    FULL_MEMORY_BARRIER();
#if defined(ECB_EXTRA_CHECKS)
    return true;
#endif
}

ECB_VOLATILE_T void* ecbuff_read_dequeue(ecbuff* const restrict rb)
{
    assert(rb);
    FULL_MEMORY_BARRIER();
    ECB_UINT_T rp = rb->rp;
#if defined(ECB_ASSERT) || defined(ECB_EXTRA_CHECKS)
    ECB_UINT_T wp = rb->wp;
#if defined(ECB_ASSERT) && !defined(ECB_EXTRA_CHECKS)
    assert(!ecbuff_is_empty_private(rp, wp));
#elif defined(ECB_EXTRA_CHECKS)
    if(ecbuff_is_empty_private(rp, wp))
        return NULL;
#endif
#endif
    return &rb->elems[rp];
}

ECB_VOID_BOOL_T ecbuff_read_free(ecbuff* const restrict rb)
{
    assert(rb);
    FULL_MEMORY_BARRIER();
    ECB_UINT_T rp = rb->rp;
#if defined(ECB_ASSERT) || defined(ECB_EXTRA_CHECKS)
    ECB_UINT_T wp = rb->wp;
#if defined(ECB_ASSERT) && !defined(ECB_EXTRA_CHECKS)
    assert(!ecbuff_is_empty_private(rp, wp));
#elif defined(ECB_EXTRA_CHECKS)
    if(ecbuff_is_empty_private(rp, wp))
        return false;
#endif
#endif
    ECB_UINT_T total_size = rb->total_size;
    ECB_UINT_T element_size = rb->element_size;

    rb->rp = ECB_MODULUS((rp + element_size), total_size);
    FULL_MEMORY_BARRIER();
#if defined(ECB_EXTRA_CHECKS)
    return true;
#endif
}
#endif