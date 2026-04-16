#pragma once

#include <stdbool.h>
#include <stdint.h>

/**
 * Generic Hamming code encoder/decoder.
 *
 * Supported configurations (r = number of parity bits):
 *
 *   r=2 -> Hamming(3,1)   : 1 data bit,  3 codeword bits
 *   r=3 -> Hamming(7,4)   : 4 data bits,  7 codeword bits
 *   r=4 -> Hamming(15,11) : 11 data bits, 15 codeword bits
 *   r=5 -> Hamming(31,26) : 26 data bits, 31 codeword bits
 *   r=6 -> Hamming(63,57) : 57 data bits, 63 codeword bits
 *
 * Parity bits occupy positions that are powers of two (1, 2, 4, …).
 * Data bits fill all remaining positions.
 * Even parity convention.  Single-bit errors are detected and corrected.
 *
 * Bits are packed LSB-first into byte arrays.
 * Buffer sizes required by the caller:
 *   data     : HAMMING_DATA_BYTES(r)
 *   codeword : HAMMING_CW_BYTES(r)
 */

/* Number of codeword bits for r parity bits: 2^r - 1 */
#define HAMMING_N(r) ((1u << (r)) - 1u)

/* Number of data bits for r parity bits: 2^r - r - 1 */
#define HAMMING_K(r) (HAMMING_N(r) - (r))

/* Byte counts (ceiling division) */
#define HAMMING_CW_BYTES(r)   ((HAMMING_N(r) + 7u) / 8u)
#define HAMMING_DATA_BYTES(r) ((HAMMING_K(r) + 7u) / 8u)

/* Maximum supported r */
#define HAMMING_MAX_R 6

typedef struct {
    uint8_t r; /* parity bits  */
    uint8_t n; /* codeword bits = 2^r - 1 */
    uint8_t k; /* data bits    = n - r    */
} hamming_cfg_t;

/**
 * Initialise a hamming_cfg_t for the given number of parity bits.
 * @param cfg  Output configuration struct.
 * @param r    Number of parity bits (2 <= r <= HAMMING_MAX_R).
 * @return     true on success, false if r is out of range.
 */
bool hamming_init(hamming_cfg_t *cfg, uint8_t r);

/**
 * Encode k data bits into an n-bit codeword.
 * @param cfg      Initialised configuration.
 * @param data     Input: ceil(k/8) bytes, packed LSB-first.
 * @param codeword Output: ceil(n/8) bytes, packed LSB-first (must be zeroed by caller or will be zeroed internally).
 */
void hamming_encode(const hamming_cfg_t *cfg, const uint8_t *data,
                    uint8_t *codeword);

/**
 * Decode an n-bit codeword and recover k data bits.
 * A single-bit error in the codeword is automatically corrected.
 *
 * @param cfg      Initialised configuration.
 * @param codeword Input/output: ceil(n/8) bytes.  The byte array is corrected in-place.
 * @param data     Output: ceil(k/8) bytes.
 * @return  0  – no error detected
 *          1  – single-bit error detected and corrected
 *         -1  – syndrome out of range (should not happen with valid r)
 */
int hamming_decode(const hamming_cfg_t *cfg, uint8_t *codeword, uint8_t *data);

/**
 * Encode a buffer of data bits, processing cfg->k bits at a time.
 *
 * src_bits must be an exact multiple of cfg->k.
 * dst must hold at least (src_bits / cfg->k) * cfg->n bits,
 * i.e. ceil(src_bits * cfg->n / cfg->k / 8) bytes.
 *
 * Example with Hamming(7,4): 64 data bits (8 bytes) → 112 codeword bits (14 bytes).
 *
 * @param cfg      Initialised configuration.
 * @param src      Input data, packed LSB-first.
 * @param src_bits Number of data bits to encode (must be a multiple of cfg->k).
 * @param dst      Output codeword buffer, packed LSB-first (zeroed internally).
 */
void hamming_encode_buffer(const hamming_cfg_t *cfg, const uint8_t *src,
                           uint16_t src_bits, uint8_t *dst);

/**
 * Decode a buffer of codeword bits, processing cfg->n bits at a time.
 * Single-bit errors within each block are corrected.
 *
 * src_bits must be an exact multiple of cfg->n.
 * dst must hold at least (src_bits / cfg->n) * cfg->k bits.
 *
 * @param cfg      Initialised configuration.
 * @param src      Input codeword buffer, packed LSB-first (read-only).
 * @param src_bits Number of codeword bits (must be a multiple of cfg->n).
 * @param dst      Output data buffer, packed LSB-first (zeroed internally).
 * @return  >= 0  total single-bit errors corrected across all blocks
 *            -1  uncorrectable error encountered in at least one block
 */
int hamming_decode_buffer(const hamming_cfg_t *cfg, const uint8_t *src,
                          uint16_t src_bits, uint8_t *dst);
