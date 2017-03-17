#include "cobs.hpp"

namespace cobs {
   /**
   @see max_encoded_length to calculate a safe length of the destination buffer.
   */
   EncodeResult encode(
      /** Address of the source buffer. */
      const uint8_t * const src_start,

      /** Length of the source buffer. */
      const size_t src_len,

      /** Address of the destination buffer. */
      uint8_t * const dst_start,

      /** Length of the destination buffer. */
      const size_t dst_len
   ) {
      const uint8_t * const src_end = src_start + src_len;
      const uint8_t * const dst_end = dst_start + dst_len;

      // This variable will always point to the next byte to read.
      const uint8_t * src = src_start;

      // Because this implementation copies bytes while seeking for the next
      // zero, we need to have two pointers. One to point to the next location
      // to copy bytes to and one to point to the location where we should write
      // the offset. Since dst_offset is always < dst_copy when writing so we
      // don't have to check for a write overflow.
      uint8_t * dst_copy = dst_start;
      uint8_t * dst_offset = dst_copy++;

      auto bytes_produced = [&dst_copy, &dst_start]() -> size_t {
         // Can cast because dst_copy >= dst_start.
         return static_cast<size_t> (dst_copy - dst_start);
      };

      auto create_ok_status = [&bytes_produced]() {
         return EncodeResult { EncodeResult::Status::OK, bytes_produced() };
      };

      auto create_write_overflow_status = []() {
         return EncodeResult { EncodeResult::Status::WRITE_OVERFLOW, 0 };
      };

      while (src < src_end) {
         const uint8_t byte = *(src++);
         if (byte != 0x00) {
            // Append the data byte if possible.
            if (dst_copy >= dst_end) return create_write_overflow_status();
            *(dst_copy++) = byte;

            // Unless we hit the maximum offset, keep copying.
            if ((dst_copy - dst_offset) != 0xff) continue;
         }

         // Write back the offset, set the offset index to the
         // current copy location and advance the copy index.
         *(dst_offset) = dst_copy - dst_offset;
         dst_offset = dst_copy++;
      }

      // Write back the offset. There is no need to update the pointer
      // anymore.
      *(dst_offset) = dst_copy - dst_offset;

      // Append the zero marker if possible.
      if (dst_copy >= dst_end) return create_write_overflow_status();
      *(dst_copy++) = 0x00;

      return create_ok_status();
   }

   /**
   @see max_decoded_length to calculate a safe length of the destination buffer.
   */
   DecodeResult decode(
      /** Address of the source buffer. */
      const uint8_t * const src_start,

      /** Length of the source buffer. */
      const size_t src_len,

      /** Address of the destination buffer. */
      uint8_t * const dst_start,

      /** Length of the destination buffer. */
      const size_t dst_len
   ) {
      const uint8_t * const src_end = src_start + src_len;
      const uint8_t * const dst_end = dst_start + dst_len;

      // This variable will always point to the next byte to read.
      const uint8_t * src = src_start;

      // This variable will always point to the next byte to write.
      uint8_t * dst = dst_start;

      auto bytes_consumed = [&src, &src_start]() -> size_t {
         // Can cast because src >= src_start.
         return static_cast<size_t> (src - src_start);
      };

      auto bytes_produced = [&dst, &dst_start]() -> size_t {
         // Can cast because dst >= dst_start.
         return static_cast<size_t> (dst - dst_start);
      };

      auto create_ok_status = [&bytes_consumed, &bytes_produced]() {
         return DecodeResult { DecodeResult::Status::OK, bytes_consumed(), bytes_produced() };
      };

      auto create_unexpected_zero_status = [&bytes_consumed]() {
         return DecodeResult { DecodeResult::Status::UNEXPECTED_ZERO, bytes_consumed(), 0 };
      };

      auto create_read_overflow_status = []() {
         return DecodeResult { DecodeResult::Status::READ_OVERFLOW, 0, 0 };
      };

      auto create_write_overflow_status = []() {
         return DecodeResult { DecodeResult::Status::WRITE_OVERFLOW, 0, 0 };
      };

      // Read the first offset.
      if (src >= src_end) {
         return create_read_overflow_status();
      }
      uint8_t offset = *(src++);

      // If the first offset is 0x00 we can stop immediately.
      if (offset == 0x00) {
         return create_unexpected_zero_status();
      }

      while (true) {
         // Compute the location of the next zero. Subtract one from
         // offset to get the number of data bytes that follow.
         const uint8_t * const src_copy_end = src + offset - 1;
         const uint8_t * const dst_copy_end = dst + offset - 1;

         // Check if we can copy the data until the next zero.
         if (src_copy_end > src_end) {
            return create_read_overflow_status();
         }
         if (dst_copy_end > dst_end) {
            return create_write_overflow_status();
         }

         // Copy data until we are at the next zero.
         while (src < src_copy_end) {
            const uint8_t byte = *(src++);

            // We should not encounter a zero in the encoded data.
            if (byte == 0x00) {
               // Return control to the caller so it can restart a
               // decoding operation from where we stop now.
               return create_unexpected_zero_status();
            }

            *(dst++) = byte;
         }

         // Retrieve the next zero offset.
         if (src >= src_end) {
            return create_read_overflow_status();
         }
         const uint8_t next_offset = *(src++);

         // Check if we've hit the end.
         if (next_offset == 0x00) break;

         // If the last offset was not equal to 0xff and we have not
         // reached the end, output a zero.
         if (offset != 0xff) {
            if (dst >= dst_end) {
               return create_write_overflow_status();
            }
            *(dst++) = 0x00;
         }

         // Store the offset for the next iteration.
         offset = next_offset;
      }

      return create_ok_status();
   }
}

