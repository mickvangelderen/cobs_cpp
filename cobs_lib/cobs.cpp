#include "cobs.hpp"
#include <cassert>
#define debug_assert(condition, remark) assert(condition)

namespace cobs {
    /*
    Here are some interesting encoding cases.

    decoded, length -> encoded, length
           , 0      -> 0      , 1 ?
           , 0      -> 1|0    , 2 ?
    x      , 1      -> 2|x|0  , 3
    0      , 1      -> 1|1|0  , 3

    decoded, length -> encoded       , length
    252x|0 , 253    -> 253|252x|1|0  , 255
    253x   , 253    -> 254|253x|0    , 255
    253x|0 , 254    -> 254|253x|1|0  , 256
    254x   , 254    -> 255|254x|0    , 256 ?
    254x   , 254    -> 255|254x|1|0  , 257 ?
    254x|0 , 255    -> 255|254x|1|1|0, 258
    255x   , 255    -> 255|254x|2|x|0, 258

    decoded, length -> encoded0               , length
    506x|0 , 507    -> 255|254x|253|252x|1|0  , 510
    507x   , 507    -> 255|254x|254|253x|0    , 510
    507x|0 , 508    -> 255|254x|255|253x|1|0  , 511
    508x   , 508    -> 255|254x|255|254x|0    , 511 ?
    508x   , 508    -> 255|254x|255|254x|1|0  , 512 ?
    508x|0 , 509    -> 255|254x|255|254x|1|1|0, 513
    509x   , 509    -> 255|254x|255|254x|1|x|0, 513
    */

    /*
    The maximum encoded length is composed of:

     - the decoded byte length +
     - 1 overhead byte for every 254 decoded bytes +
     - the frame marker

    It could be implemented as:

    max_enc_len(len) {
        return len + ceil(len/254) + 1
    }

    To get rid of the ceil we can abuse integer division:

    max_enc_len(len) {
        return len + (len + 254)/254 + 1
    }
    */
    size_t max_encoded_length(size_t decoded_length) {
        return decoded_length + (decoded_length + 0xfe)/0xfe + 1;
    }

    /*
    The smallest encoded size is obtained when the input contains no
    zeros more than 254 bytes apart. In this case the overhead is only 2
    bytes: the first zero offset and the frame marker. Thus the maximum
    decoded length is the length minus 2.
    */
    size_t max_decoded_length(size_t encoded_length) {
        return encoded_length - 2;
    }


    size_t encode(const uint8_t * const src, size_t src_len, uint8_t * const dst, size_t dst_len) {
        size_t src_idx = 0;
        size_t dst_offset_idx = 0;
        size_t dst_copy_idx = 1;
        size_t offset = 1;

        while (src_idx < src_len) {
            const uint8_t byte = src[src_idx++];
            if (byte != 0x00) {
                // Append the data byte.
                debug_assert(dst_copy_idx < dst_len, "buffer write overflow");
                dst[dst_copy_idx++] = byte;
                offset++;
                if (offset != 0xff) continue;
            }
            // Write back and reset the offset.
            debug_assert(dst_offset_idx < dst_len, "buffer write overflow");
            dst[dst_offset_idx] = offset;
            offset = 1;
            dst_offset_idx = dst_copy_idx++;
        }

        if (offset != 1) {
            // Write back the offset only if it is larger than 1. If the
            // offset equals 1, we can achieve an equivalent encoding by
            // not writing it. The last offset will then immediately
            // arrive at the frame marker.
            debug_assert(dst_offset_idx < dst_len, "buffer write overflow");
            dst[dst_offset_idx] = offset;
        }

        // Append the frame marker.
        debug_assert(dst_copy_idx < dst_len, "buffer overflow");
        dst[dst_copy_idx++] = 0x00;

        return dst_copy_idx;
    }

    size_t decode(const uint8_t * const src, size_t src_len, uint8_t * const dst, size_t dst_len) {
        size_t src_idx = 0;
        size_t dst_idx = 0;
        bool write_zero = false;

        debug_assert(src_idx < src_len, "buffer read overflow");
        uint8_t zero_offset = src[src_idx];

        while (zero_offset != 0x00) {
            // Compute the index of the next zero.
            size_t src_zero_idx = src_idx + zero_offset;
            src_idx++;

            debug_assert(src_zero_idx < src_len, "buffer read overflow");
            debug_assert(dst_idx + zero_offset < dst_len, "buffer write overflow");

            // Copy data until we are at the next zero.
            while (src_idx < src_zero_idx) {
                dst[dst_idx++] = src[src_idx++];
            }

            // If the last offset was not the first nor equal to 0xff, output a zero.
            if (write_zero) {
                debug_assert(dst_idx < dst_len, "buffer write overflow");
                dst[dst_idx++] = 0x00;
            }

            write_zero = zero_offset != 0xff;
        }

        return dst_idx;
    }
}
