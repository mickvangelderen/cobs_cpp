#pragma once

#include <cstdint>
#include <cstddef>

/*
 * Packet framing uses the Consistent Overhead Byte Stuffing algorithm for
 * encoding data bytes. The maximum length for each packet is limited to 254 bytes.
 * Refer to: https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing
 */

namespace cobs {

    size_t max_encoded_len(size_t decoded_len);

    size_t max_decoded_len(size_t encoded_len);

    size_t encode(const uint8_t * const src, size_t src_len, uint8_t * const dst, size_t dst_len);

    size_t decode(const uint8_t * const src, size_t src_len, uint8_t * const dst, size_t dst_len);

}
