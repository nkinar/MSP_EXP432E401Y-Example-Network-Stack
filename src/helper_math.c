#include <stdint.h>
#include "helper_math.h"
#include "FreeRTOS_IP.h"

// https://stackoverflow.com/questions/3022552/is-there-any-standard-htonl-like-function-for-64-bits-integers-in-c

uint64_t htonll(uint64_t x)
{
#if __BIG_ENDIAN__
    return x;
#else
    return ((uint64_t)FreeRTOS_htonl((x) & 0xFFFFFFFFLL) << 32) | FreeRTOS_htonl((x) >> 32);
#endif
} // end

uint64_t ntohll(uint64_t x)
{
#if __BIG_ENDIAN__
    return x;
#else
    return ((uint64_t)FreeRTOS_ntohl((x) & 0xFFFFFFFFLL) << 32) | FreeRTOS_ntohl((x) >> 32);
#endif
} // end


uint8_t reverse_byte(uint8_t b)
{
    b = (b * 0x0202020202ULL & 0x010884422010ULL) % 1023;
    return b;
} // end



