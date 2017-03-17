#pragma once

#include <cstdint>
#include <cstddef>

/** @mainpage
Consistent Overhead Byte Stuffing (COBS) removes a specific value from a list of
values. The removed value can then, for example, be used as a frame marker.

Refer to: https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing

## Notation

The character `x' is used to denote a single non-zero byte. To denote a sequence
of non-zero bytes, "Nx" is used where N is the number of bytes. For example 4x
denotes 4 consecutive non-zero bytes.

## Considerations:

1. Configurable marker byte value. The current implementation assumes 0x00.

2. Append the marker byte to the encoded result. The current implementation
appends the marker byte.

3. If the encoding results in [ ... | 255 | 254x | 1 | 0 ], encode it as [ ... |
255 | 254x | 0 ]. This is possible because when we reach the frame marker, the
zero from the last offset is not appended. Since the maximum offset does not
append a zero either, the encoding can be shortened. The same principle applies
to encoding the empty byte array. The COBS paper mentions this but does not
implement it in their examples. The current implementation does not apply this
reduced encoding because 1) the savings are minimal, 2) it requires keeping
track of the last offset and 3) it makes `max_encoded_length` less symmetric
with `max_decoded_length`.

4. If the encoding results in [ ... | n | ... | x | 0 ] where n is the last
offset and x > n, encode it as [ ... | x | ... | 0 ]. The standard
implementation would run into an decoding error because x > n is bigger than the
remaining number of bytes. The current implementation does not apply this
reduced encoding for compatibility with other implementations.

## Examples:

Examples without a maximum offset.

```
decoded  , length -> encoded        , length, note
         , 0      -> 0              , 1     , reduced (consideration 3)
         , 0      -> 1|0            , 2     , standard (consideration 3)
x        , 1      -> 2|x|0          , 3     ,
0        , 1      -> 1|1|0          , 3     ,
x|x      , 2      -> 3|x|x|0        , 4     ,
x|0      , 2      -> 2|x|1|0        , 4     ,
0|x      , 2      -> 1|2|x|0        , 4     ,
0|0      , 2      -> 1|1|1|0        , 4     ,
10x|0|20x, 31     -> 11|10x|21|20x|0, 33    ,
```

Examples close to one maximum offset.

```
decoded, length -> encoded       , length, note
252x|0 , 253    -> 253|252x|1|0  , 255   ,
253x   , 253    -> 254|253x|0    , 255   ,
253x|0 , 254    -> 254|253x|1|0  , 256   ,
254x   , 254    -> 255|254x|0    , 256   , reduced (consideration 3)
254x   , 254    -> 255|254x|1|0  , 257   , standard (consideration 3)
254x|0 , 255    -> 255|254x|1|1|0, 258   ,
255x   , 255    -> 255|254x|2|x|0, 258   ,
```

Examples close to two maximum offsets.

```
decoded, length -> encoded                , length, note
506x|0 , 507    -> 255|254x|253|252x|1|0  , 510   ,
507x   , 507    -> 255|254x|254|253x|0    , 510   ,
507x|0 , 508    -> 255|254x|254|253x|1|0  , 511   ,
508x   , 508    -> 255|254x|255|254x|0    , 511   , reduced (consideration 3)
508x   , 508    -> 255|254x|255|254x|1|0  , 512   , standard (consideration 3)
508x|0 , 509    -> 255|254x|255|254x|1|1|0, 513   ,
509x   , 509    -> 255|254x|255|254x|1|x|0, 513   ,
```
*/

namespace cobs {
   /**
   @brief Given the length of a decoded byte array, compute the maximum length
   the encoded byte array can attain.

   The maximum length is attained when the decoded byte array contains no zeros.
   The maximum length consists of the decoded byte array length, the offset
   overhead and the frame marker. It is computed as `decoded_length +
   floor(decoded_length/254) + 2`.
   */
   constexpr size_t max_encoded_length(
      /**
      The number of bytes that will be encoded. Must be equal to or more than 0.
      */
      const size_t decoded_length
   ) {
      return decoded_length + decoded_length/0xfe + 2;
   }

   /**
   @brief Given the length of an encoded byte array, compute the maximum length
   the decoded byte array can attain.

   A case where the minimum overhead is achieved is when the decoded byte array
   consists only of marker bytes. This means that the maximum decoded length can
   be as big as the encoded length minus the packet overhead, consisting of the
   first offset and the marker byte. The maximum decoded length is thus given by
   encoded_length - 2.
   */
   constexpr size_t max_decoded_length(
      /**
      The number of bytes that will be decoded. Must be equal to or more than 2.
      */
      const size_t encoded_length
   ) {
      return encoded_length - 2;
   }

   /**
   @see encode, the function returning this struct.
   */
   struct EncodeResult {
      enum class Status {
         /** The operation was successful. */
         OK,

         /** The destination buffer was too small. */
         WRITE_OVERFLOW
      };

      const Status status;

      /**
      @brief The number of bytes written.

      If the status is OK, the number of bytes that were written to dst_start.
      Otherwise it is set to 0.
      */
      const size_t produced;
   };

   EncodeResult encode(const uint8_t * const src_start, const size_t src_len, uint8_t * const dst_start, const size_t dst_len);

   /**
   @see decode, the function returning this struct.
   */
   struct DecodeResult {
      enum class Status {
         /**
         The operation was successful. A new decoding operation can be started
         from `src_start + result.consumed`.
         */
         OK,

         /** The destination buffer was too small. */
         WRITE_OVERFLOW,

         /** The source buffer did not contain at least one full packet. */
         READ_OVERFLOW,

         /**
         The source buffer contained a 0 in an unexpected place. A new decoding
         operation can be started from `src_start + result.consumed`.
         */
         UNEXPECTED_ZERO
      };

      const Status status;

      /**
      @brief The number of bytes read.

      If the status is OK or UNEXPECTED_ZERO, the number of bytes that were read
      from src_start. Otherwise it is set to 0.
      */
      const size_t consumed;

      /**
      @brief The number of bytes written.

      If the status is OK, the number of bytes that were written to dst_start.
      Otherwise it is set to 0.
      */
      const size_t produced;
   };

   DecodeResult decode(const uint8_t * const src_start, const size_t src_len, uint8_t * const dst_start, const size_t dst_len);
}
