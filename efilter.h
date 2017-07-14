/*
 * High performance IIR-Filter optimised for 32bit integer performance
 * while minimising rounding error. The filter strength is applied as
 * a power of two.
 *
 * Written by Elias Oenal <efilter@eliasoenal.com>, released as public domain.
 */

#ifndef FILTER_H_
#define FILTER_H_

#include <stdint.h>

int32_t efilter_low_pass(const int32_t last_filtered_sample,
		const int32_t new_sample,
		const int16_t strength);

#endif /* FILTER_H_ */
