/**
 * Tobias Mages & Wenqing Yan
 *
 * support functions to generate payload data and function
 *
 */

#ifndef PACKET_GERNATION_LIB
#define PACKET_GERNATION_LIB

#include "hamming.h"
#include "packet_generation.h"
#include "pico/stdlib.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define MESSAGE_LEN 16 // 2B pseudo-seq + 14B data

#define MESSAGE_BITS (MESSAGE_LEN * 8)
/* Number of 4-bit blocks (rounded up) */
#define HAMMING_BLOCKS ((MESSAGE_BITS + HAMMING_K(HAMMING_R) - 1) / HAMMING_K(HAMMING_R))

/* Total encoded bits */
#define HAMMING_ENCODED_BITS (HAMMING_BLOCKS * HAMMING_N(HAMMING_R))

#define PAYLOADSIZE       ((HAMMING_ENCODED_BITS + 7) / 8)
#define HEADER_LEN        10                                                       // 8 header + length + seq
#define buffer_size(x, y) (((x + y) % 4 == 0) ? ((x + y) / 4) : ((x + y) / 4 + 1)) // define the buffer size with ceil((PAYLOADSIZE+HEADER_LEN)/4)

/*
 * Hamming(7,4): r=3, k=4 data bits, n=7 codeword bits per block.
 * 8 bytes (64 bits) of raw data → 16 blocks → 112 codeword bits = 14 bytes,
 * which fits exactly into PAYLOADSIZE.
 */

#define HAMMING_R 3

#ifndef MINMAX
#define MINMAX
#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))
#endif

/*
 * obtain the packet header template for the corresponding radio
 */
uint8_t *packet_hdr_template(uint16_t receiver);

/*
 * generate of a uniform random number.
 */
uint32_t rnd();

/*
 * generate compressible payload sample
 * file_position provides the index of the next data byte (increments by 2 each time the function is called)
 */
extern uint16_t file_position;
uint16_t generate_sample();

/*
 * fill packet with 16-bit samples
 * include_index: shall the file index be included at the first two byte?
 * length: the length of the buffer which can be filled with data
 */
void generate_data(uint8_t *buffer, uint8_t length, bool include_index);

/* including a header to the packet:
 * - 8B header sequence
 * - 1B payload length
 * - 1B sequence number
 *
 * packet: buffer to be updated with the header
 * seq: sequence number of the packet
 */
void add_header(uint8_t *packet, uint8_t seq, uint8_t *header_template);

#endif