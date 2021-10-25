/*
 * The main design goal of ecbuff is being a lock-free high-throughput
 * inter-thread circular/ring buffer. In addition to that it allows
 * exposing of element pointers, enabling zero-copy operations and
 * direct peripheral or DMA access. For the most part ecbuff is written
 * in plain C99, with the notable exception of using memory barriers if
 * selected. Alternatively multi-threading can on some architectures be
 * enabled using the volatile keyword, though this leaves the scope of
 * the C standard.
 *
 * Written by Elias Oenal <ecbuff@eliasoenal.com>, released as public domain.
 */

#ifndef ECBUFF_H
#define ECBUFF_H

#include <stdbool.h>

#ifndef ECB_NO_CFG
#include "ecbuff_cfg.h"
#else
#include <signal.h>
#include <stdint.h>
#endif

#if defined(ECB_THREAD_VOLATILE)
#define ECB_VOLATILE_T volatile
#else
#define ECB_VOLATILE_T
#endif

#if defined(ECB_EXTRA_CHECKS)
#define ECB_VOID_BOOL_T bool
#else
#define ECB_VOID_BOOL_T void
#endif

typedef struct {
    ECB_VOLATILE_T ECB_ATOMIC_T total_size;
    ECB_VOLATILE_T ECB_ATOMIC_T element_size;
    ECB_VOLATILE_T ECB_ATOMIC_T wp;             /* write pointer */
    ECB_VOLATILE_T ECB_ATOMIC_T rp;             /* read pointer */
    ECB_VOLATILE_T char elems[];                /* flexible array member can be used to allocate buffer as part of this struct */
} ecbuff;

/* ecbuff_init
 * total_size has to be an integer multiple of element_size
 */
void ecbuff_init(ecbuff* const restrict rb, const ECB_UINT_T total_size, const ECB_UINT_T element_size);
bool ecbuff_is_full(const ecbuff* const restrict rb);
bool ecbuff_is_empty(const ecbuff* const restrict rb);
ECB_VOID_BOOL_T ecbuff_write(ecbuff* const restrict rb, const void* const restrict element);
ECB_VOID_BOOL_T ecbuff_read(ecbuff* const restrict rb, void* const restrict element);
ECB_UINT_T ecbuff_unused(const ecbuff* const restrict rb);
ECB_UINT_T ecbuff_used(const ecbuff* const restrict rb);

/*
 * Clear/reset the buffer
 * This function should be invoked only from the consumer/reader thread.
 * Theoretically it clears the buffer but functionally the read and write pointers
 * are set to point at each other. Therefore the buffer is not actually cleared
 * but the existing data in the buffer is just discarded without completely reading it.
 */
void ecbuff_read_discard(ecbuff* const restrict rb);

#ifdef ECB_DIRECT_ACCESS
ECB_VOLATILE_T void* ecbuff_write_alloc(ecbuff* const restrict rb);
ECB_VOID_BOOL_T ecbuff_write_enqueue(ecbuff* const restrict rb);
ECB_VOLATILE_T void* ecbuff_read_dequeue(ecbuff* const restrict rb);
ECB_VOID_BOOL_T ecbuff_read_free(ecbuff* const restrict rb);
#endif // ECB_DIRECT_ACCESS

#endif // ECBUFF_H
