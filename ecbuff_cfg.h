#ifndef ECBUFF_CFG_H
#define ECBUFF_CFG_H

/* Types used by ecbuff
 * ECB_ATOMIC_T has to be atomic.
 */
#include <signal.h>
#include <stdint.h>
#define ECB_ATOMIC_T sig_atomic_t
#define ECB_ATOMIC_MAX SIG_ATOMIC_MAX
#define ECB_UINT_T unsigned int
#define ECB_UINT_MAX UINT_MAX

/* ECB_ASSERT
 * Enable runtime checks
 *
 * ECB_ELEM_ALIGN
 * Additional assertions to check whether buffer meets alignment of ECB_ELEM_ALIGN.
 */
#define ECB_ASSERT
//#define ECB_ELEM_ALIGN 4


/* ECB_THREAD_SINGLE
 *
 * Disable multi-threading capabilities.
 */
//#define ECB_THREAD_SINGLE


/* ECB_THREAD_MULTI
 *
 * This was the primary design goal of ecbuff. It has been carefully crafted to allow
 * for high performance and lock-free communication between different threads. This even
 * allows to pass data from and to interrupts. There is only a minor limitation to it:
 * There can only ever be one read- and one write-thread per ecbuff instance. Thus for
 * bi-directional communication two separate buffer instances are required.
 */
#define ECB_THREAD_MULTI


/* ECB_THREAD_BARRIER
 *
 * This is the preferred way to enable multi-threading. While barriers are only supported
 * by modern compilers, they should work on any architecture supported by the compiler
 * and result in the best possible performance.
 */
#define ECB_THREAD_BARRIER


/* ECB_THREAD_VOLATILE ***WARNING: USE WITH CARE***
 *
 * Enabling ECB_THREAD_VOLATILE causes ecbuff to rely on the instruction re-ordering
 * restrictions imposed by the volatile keyword. This only affects the compiler's
 * instruction ordering and can't suppress out of order execution.
 * If available ECB_THREAD_BARRIER should be preferred.
 *
 * You should NOT enable this if:
 * 1. Your application communicates between different CPU cores with separate caches.
 * 2. Your architecture reorders stores and/or loads (out of order execution)
 *
 * This option should work on most single core micro-controllers
 * without out of order execution. e.g.:
 * ARM Cortex M (Tested)
 * AVR, PIC, 8051 (Untested)
 */
//#define ECB_THREAD_VOLATILE


/* ECB_EXTRA_CHECKS
 *
 * Defining ECB_EXTRA_CHECKS changes the declaration of the read and write api
 * to include a return value. Reading an empty buffer and writing an already
 * full buffer is no longer undefined behaviour. Reading an empty buffer will
 * now return false and prevent buffer corruption. The same is true for writing
 * a full buffer, though the exact behaviour has to be defined with
 * ECB_WRITE_OVERWRITE or ECB_WRITE_DROP.
 */
#define ECB_EXTRA_CHECKS


/* ECB_WRITE_DROP
 *
 * If the buffer is full and new data will be silently dropped.
 * ECB_WRITE_DROP and ECB_WRITE_OVERWRITE are mutually exclusive.
 */
#define ECB_WRITE_DROP


/* ECB_WRITE_OVERWRITE ***WARNING: Not thread-safe!***
 *
 * If the buffer is full new data will silently overwrite the oldest element.
 * Only supported when ECB_THREAD_SINGLE is defined.
 * ECB_WRITE_DROP and ECB_WRITE_OVERWRITE are mutually exclusive.
 */
//#define ECB_WRITE_OVERWRITE

/* ECB_DIRECT_ACCESS
 *
 * Enables advanced two staged read/write API.
 *
 * Writing:
 * ecbuff_write_alloc() returns a pointer to the next element which can be written to.
 * ecbuff_write_enqueue() queues the element to be read.
 * Reading:
 * ecbuff_read_dequeue() returns a pointer to the next elememt ready to be read.
 * ecbuff_read_free() frees the memory allocated by the element
 *
 * This exposes the element's memory for direct access by peripherals or DMA,
 * enabling true zero-copy operation.
 */
//#define ECB_DIRECT_ACCESS

#endif /* ECBUFF_CFG_H */
