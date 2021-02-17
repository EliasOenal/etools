/* See ecbuff.h and ecbuff_cfg.h for further information */

#include "ecbuff.h"

#if defined(ECB_ASSERT)
#include <assert.h>
#include <limits.h>

#if !defined(ASSERT)
//use standard assert() if nothing custom was defined
#define ASSERT(x) assert(x)
#endif

#else
#define NDEBUG
#undef ASSERT	//ignore earlier definition from ecbuff_cfg.h
#define ASSERT(x)
#endif

#if defined(ECB_WRITE_DROP)
#include <stddef.h>
#endif

/* Turn static assertions into runtime checks on older compilers */
#if !defined(static_assert)
#define static_assert(x, y) ASSERT(x)
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
#if (__STDC_VERSION__ >= 201112L) /* C11 */
#include <stdatomic.h>
#define FENCE_ACQUIRE() atomic_thread_fence(memory_order_acquire)
#define FENCE_RELEASE() atomic_thread_fence(memory_order_release)
#define FENCE_FULL() atomic_thread_fence(memory_order_acq_rel)
#else /* C11 */
#if defined(__GNUC__) || defined(__clang__)
#define FENCE_FULL() __sync_synchronize()
#elif defined(__ARMCC_VERSION)
#define FENCE_FULL() __dmb(15)
#else
#error ECB_THREAD_BARRIER does not support this compiler!
#endif /* Compiler support */
#define FENCE_ACQUIRE() FENCE_FULL()
#define FENCE_RELEASE() FENCE_FULL()
#endif /* C11 */
#else /* ECB_THREAD_SINGLE or ECB_THREAD_VOLATILE */
#define FENCE_FULL()
#define FENCE_ACQUIRE() FENCE_FULL()
#define FENCE_RELEASE() FENCE_FULL()
#endif /* ECB_THREAD_BARRIER */

#define ECB_MODULUS(x, y) (x % y)
#define ECB_CHECK_ALIGN(ptr, req) (((uintptr_t)ptr) % req == 0)


void ecbuff_init(ecbuff* const restrict rb, const ECB_UINT_T total_size, const ECB_UINT_T element_size)
{
    /* Check type limits */
    static_assert(ECB_UINT_MAX >= ECB_ATOMIC_MAX, "ECB_UINT_MAX is required to at least be ECB_ATOMIC_MAX");
    /* Sanity-check parameters */
    ASSERT(total_size <= ECB_ATOMIC_MAX);
    ASSERT(total_size);
    ASSERT(element_size);
    ASSERT(ECB_MODULUS(total_size, element_size) == 0);
    ASSERT(total_size >= element_size * 2);
    ASSERT(rb);
#if defined(ECB_ELEM_ALIGN)
    ASSERT(ECB_CHECK_ALIGN(element_size, ECB_ELEM_ALIGN));
    ASSERT(ECB_CHECK_ALIGN(&rb->elems[0], ECB_ELEM_ALIGN));
    ASSERT(ECB_CHECK_ALIGN(&rb->elems[0] + element_size, ECB_ELEM_ALIGN));
#endif
    rb->total_size = total_size;
    rb->element_size = element_size;
    rb->rp = 0;
    rb->wp = 0;
}

static inline bool ecbuff_is_full_private(const ECB_UINT_T total_size, const ECB_UINT_T element_size,
                                            const ECB_UINT_T rp, const ECB_UINT_T wp)
{
    ASSERT(total_size);
    ASSERT(element_size);
    return ECB_MODULUS((wp + element_size), total_size) == rp;
}

bool ecbuff_is_full(const ecbuff* const restrict rb)
{
    ASSERT(rb);
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
    ASSERT(rb);
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
    ASSERT(rb);
    ASSERT(element);
    ECB_UINT_T total_size = rb->total_size;
    ECB_UINT_T element_size = rb->element_size;
    ECB_UINT_T wp = rb->wp;
#if defined(ECB_WRITE_DROP) || defined(ECB_WRITE_OVERWRITE) || defined(ECB_ASSERT)
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
    ASSERT(!ecbuff_is_full_private(total_size, element_size, rp, wp));
#endif /* ECB_WRITE_OVERWRITE */
    FENCE_ACQUIRE();
    ECB_MEMCPY(&rb->elems[wp], element, element_size);
    FENCE_RELEASE();
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
#if defined(ECB_EXTRA_CHECKS)
    return true;
#endif /* ECB_EXTRA_CHECKS */
}

ECB_VOID_BOOL_T ecbuff_read(ecbuff* const restrict rb, void* const restrict element)
{
    ASSERT(rb);
    ASSERT(element);
    ECB_UINT_T rp = rb->rp;
#if defined(ECB_ASSERT) || defined(ECB_EXTRA_CHECKS)
    ECB_UINT_T wp = rb->wp;
#if defined(ECB_ASSERT) && !defined(ECB_EXTRA_CHECKS)
    ASSERT(!ecbuff_is_empty_private(rp, wp));
#elif defined(ECB_EXTRA_CHECKS)
    if(ecbuff_is_empty_private(rp, wp))
        return false;
#endif
#endif
    ECB_UINT_T total_size = rb->total_size;
    ECB_UINT_T element_size = rb->element_size;
    FENCE_ACQUIRE();
    ECB_MEMCPY(element, &rb->elems[rp], element_size);
    FENCE_RELEASE();
    rb->rp = ECB_MODULUS((rp + element_size), total_size);
#if defined(ECB_EXTRA_CHECKS)
    return true;
#endif
}

ECB_UINT_T ecbuff_used(const ecbuff* const restrict rb)
{
    ASSERT(rb);
    ECB_UINT_T total_size = rb->total_size;
    ECB_UINT_T element_size = rb->element_size;
    ECB_UINT_T rp = rb->rp;
    ECB_UINT_T wp = rb->wp;

    return ECB_MODULUS((total_size + wp - rp), total_size) / element_size;
}

ECB_UINT_T ecbuff_unused(const ecbuff* const restrict rb)
{
    ASSERT(rb);
    ECB_UINT_T total_size = rb->total_size;
    ECB_UINT_T element_size = rb->element_size;
    ECB_UINT_T rp = rb->rp;
    ECB_UINT_T wp = rb->wp;

    return ECB_MODULUS((total_size - element_size + rp - wp), total_size) / element_size;
}

#ifdef ECB_DIRECT_ACCESS
ECB_VOLATILE_T void* ecbuff_write_alloc(ecbuff* const restrict rb)
{
    ASSERT(rb);
    ECB_UINT_T wp = rb->wp;
#if defined(ECB_ASSERT) && !defined(ECB_WRITE_OVERWRITE) && !defined(ECB_WRITE_DROP)
    ECB_UINT_T total_size = rb->total_size;
    ECB_UINT_T element_size = rb->element_size;
    ECB_UINT_T rp = rb->rp;
    ASSERT(!ecbuff_is_full_private(total_size, element_size, rp, wp));
#endif
    FENCE_ACQUIRE();
    return &rb->elems[wp];
}

ECB_VOID_BOOL_T ecbuff_write_enqueue(ecbuff* const restrict rb)
{
    ASSERT(rb);
    FENCE_RELEASE();
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
#if defined(ECB_EXTRA_CHECKS)
    return true;
#endif
}

ECB_VOLATILE_T void* ecbuff_read_dequeue(ecbuff* const restrict rb)
{
    ASSERT(rb);
    ECB_UINT_T rp = rb->rp;
#if defined(ECB_ASSERT) || defined(ECB_EXTRA_CHECKS)
    ECB_UINT_T wp = rb->wp;
#if defined(ECB_ASSERT) && !defined(ECB_EXTRA_CHECKS)
    ASSERT(!ecbuff_is_empty_private(rp, wp));
#elif defined(ECB_EXTRA_CHECKS)
    if(ecbuff_is_empty_private(rp, wp))
        return NULL;
#endif
#endif
    FENCE_ACQUIRE();
    return &rb->elems[rp];
}

ECB_VOID_BOOL_T ecbuff_read_free(ecbuff* const restrict rb)
{
    ASSERT(rb);
    FENCE_RELEASE();
    ECB_UINT_T rp = rb->rp;
#if defined(ECB_ASSERT) || defined(ECB_EXTRA_CHECKS)
    ECB_UINT_T wp = rb->wp;
#if defined(ECB_ASSERT) && !defined(ECB_EXTRA_CHECKS)
    ASSERT(!ecbuff_is_empty_private(rp, wp));
#elif defined(ECB_EXTRA_CHECKS)
    if(ecbuff_is_empty_private(rp, wp))
        return false;
#endif
#endif
    ECB_UINT_T total_size = rb->total_size;
    ECB_UINT_T element_size = rb->element_size;

    rb->rp = ECB_MODULUS((rp + element_size), total_size);
#if defined(ECB_EXTRA_CHECKS)
    return true;
#endif
}
#endif
