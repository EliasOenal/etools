/* See efilter.h for further information */

#include "efilter.h"

inline int32_t efilter_low_pass(const int32_t last_filtered_sample,
		const int32_t new_sample,
		const int16_t strength)
{
    return ((last_filtered_sample << strength) +
            new_sample - last_filtered_sample) >> strength;
}