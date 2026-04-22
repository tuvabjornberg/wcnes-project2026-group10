#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MESSAGE_LEN   14 // 2B pseudo-seq + 12B data
#define HAMMING_MAX_R 6

#define MESSAGE_BITS (MESSAGE_LEN * 8)
/* Number of 4-bit blocks (rounded up) */
#define HAMMING_BLOCKS ((MESSAGE_BITS + HAMMING_K(HAMMING_R) - 1) / HAMMING_K(HAMMING_R))
/* Number of codeword bits for r parity bits: 2^r - 1 */
#define HAMMING_N(r) ((1u << (r)) - 1u)
#define HAMMING_R    3

/* Number of data bits for r parity bits: 2^r - r - 1 */
#define HAMMING_K(r) (HAMMING_N(r) - (r))

/* Byte counts (ceiling division) */
#define HAMMING_CW_BYTES(r)   ((HAMMING_N(r) + 7u) / 8u)
#define HAMMING_DATA_BYTES(r) ((HAMMING_K(r) + 7u) / 8u)

/* Maximum supported r */
/* Total encoded bits */
#define HAMMING_ENCODED_BITS (HAMMING_BLOCKS * HAMMING_N(HAMMING_R))

#define PAYLOADSIZE       ((HAMMING_ENCODED_BITS + 7) / 8)
#define HEADER_LEN        10                                                       // 8 header + length + seq
#define buffer_size(x, y) (((x + y) % 4 == 0) ? ((x + y) / 4) : ((x + y) / 4 + 1)) // define the buffer size with ceil((PAYLOADSIZE+HEADER_LEN)/4)

/* ------------------------------------------------------------------ */
/* Bit-array helpers (0-based bit index, LSB-first packing)            */
/* ------------------------------------------------------------------ */

static inline uint8_t get_bit(const uint8_t *arr, uint8_t idx) {
    return (arr[idx >> 3] >> (idx & 7)) & 1u;
}

static inline void set_bit(uint8_t *arr, uint8_t idx, uint8_t val) {
    if (val)
        arr[idx >> 3] |= (uint8_t)(1u << (idx & 7));
    else
        arr[idx >> 3] &= (uint8_t)~(1u << (idx & 7));
}

static inline void flip_bit(uint8_t *arr, uint8_t idx) {
    arr[idx >> 3] ^= (uint8_t)(1u << (idx & 7));
}

/* Returns non-zero if pos (1-indexed) is a power of two (parity position). */
static inline bool is_parity_pos(uint8_t pos) {
    return pos && !(pos & (pos - 1u));
}

typedef struct {
    uint8_t r; /* parity bits  */
    uint8_t n; /* codeword bits = 2^r - 1 */
    uint8_t k; /* data bits    = n - r    */
} hamming_cfg_t;

bool hamming_init(hamming_cfg_t *cfg, uint8_t r) {
    if (r < 2 || r > HAMMING_MAX_R)
        return false;

    cfg->r = r;
    cfg->n = (uint8_t)HAMMING_N(r);
    cfg->k = (uint8_t)HAMMING_K(r);
    return true;
}

void hamming_encode(const hamming_cfg_t *cfg, const uint8_t *data,
                    uint8_t *codeword) {
    /* Clear output buffer. */
    memset(codeword, 0, HAMMING_CW_BYTES(cfg->r));

    /* Place data bits into non-parity positions (positions are 1-indexed). */
    uint8_t data_idx = 0;
    for (uint8_t pos = 1; pos <= cfg->n; pos++) {
        if (!is_parity_pos(pos)) {
            set_bit(codeword, pos - 1, get_bit(data, data_idx));
            data_idx++;
        }
    }

    /*
     * Compute each parity bit.
     * Parity bit i covers every position whose binary representation has
     * bit (i-1) set.  We set p_i so that XOR over covered positions is 0.
     */
    for (uint8_t i = 0; i < cfg->r; i++) {
        uint8_t parity_pos = (uint8_t)(1u << i); /* 1, 2, 4, 8, … */
        uint8_t parity = 0;
        for (uint8_t pos = 1; pos <= cfg->n; pos++) {
            if (pos & parity_pos)
                parity ^= get_bit(codeword, pos - 1);
        }
        set_bit(codeword, parity_pos - 1, parity);
    }
}

/* Copy `count` bits from src (starting at src_off) to dst (starting at dst_off). */
static void copy_bits(uint8_t *dst, uint16_t dst_off,
                      const uint8_t *src, uint16_t src_off, uint16_t count) {
    for (uint16_t i = 0; i < count; i++) {
        uint8_t bit = (src[(src_off + i) >> 3] >> ((src_off + i) & 7)) & 1u;
        uint16_t d = dst_off + i;
        if (bit)
            dst[d >> 3] |= (uint8_t)(1u << (d & 7));
        else
            dst[d >> 3] &= (uint8_t)~(1u << (d & 7));
    }
}

void hamming_encode_buffer(const hamming_cfg_t *cfg, const uint8_t *src,
                           uint16_t src_bits, uint8_t *dst) {
    uint8_t tmp_data[HAMMING_DATA_BYTES(HAMMING_MAX_R)];
    uint8_t tmp_cw[HAMMING_CW_BYTES(HAMMING_MAX_R)];
    uint16_t num_chunks = src_bits / cfg->k;
    uint16_t dst_bits = (uint16_t)num_chunks * cfg->n;

    memset(dst, 0, (dst_bits + 7u) / 8u);

    for (uint16_t i = 0; i < num_chunks; i++) {
        memset(tmp_data, 0, sizeof(tmp_data));
        memset(tmp_cw, 0, sizeof(tmp_cw));
        copy_bits(tmp_data, 0, src, i * cfg->k, cfg->k);
        hamming_encode(cfg, tmp_data, tmp_cw);
        copy_bits(dst, i * cfg->n, tmp_cw, 0, cfg->n);
    }
}

void add_header(uint8_t *packet, uint8_t seq, uint8_t *header_template) {
    /* fill in the header sequence*/
    for (int loop = 0; loop < HEADER_LEN - 2; loop++) {
        packet[loop] = header_template[loop];
    }
    /* add the payload length*/
    packet[HEADER_LEN - 2] = PAYLOADSIZE; // The packet length is defined as the payload data, excluding the length byte and the optional CRC. (cc2500 data sheet, p. 30)
    /* add the packet as sequence number. */
    packet[HEADER_LEN - 1] = seq;
}

#define DEFAULT_SEED 0xABCD
uint32_t seed = DEFAULT_SEED;

uint8_t packet_hdr_2500[HEADER_LEN] = {0xaa, 0xaa, 0xaa, 0xaa, 0xd3, 0x91, 0xd3, 0x91, 0x00, 0x00}; // CC2500, the last two byte one for the payload length. and another is seq number
uint8_t packet_hdr_1352[HEADER_LEN] = {0xaa, 0xaa, 0xaa, 0xaa, 0x93, 0x0b, 0x51, 0xde, 0x00, 0x00}; // CC1352P7, the last two byte one for the payload length. and another is seq number

/*
 * obtain the packet header template for the corresponding radio
 * buffer: array of size HEADER_LEN
 * receiver: radio number (2500 or 1352)
 */
uint8_t *packet_hdr_template(uint16_t receiver) {
    if (receiver == 2500) {
        return packet_hdr_2500;
    } else {
        return packet_hdr_1352;
    }
}

/*
 * generate of a uniform random number.
 */
uint32_t rnd() {
    const uint32_t A1 = 1664525;
    const uint32_t C1 = 1013904223;
    const uint32_t RAND_MAX1 = 0xFFFFFFFF;
    seed = ((seed * A1 + C1) & RAND_MAX1);
    return seed;
}

/*
 * generate compressible payload sample
 * file_position provides the index of the next data byte (increments by 2 each time the function is called)
 */
uint16_t file_position = 0;
uint16_t generate_sample() {
    if (file_position == 0) {
        seed = DEFAULT_SEED; /* reset seed when exceeding uint16_t max */
    }
    file_position = file_position + 2;
    double two_pi = 2.0 * M_PI;
    double u1, u2;
    u1 = ((double)rnd()) / ((double)0xFFFFFFFF);
    u2 = ((double)rnd()) / ((double)0xFFFFFFFF);
    double tmp = ((double)0x7FF) * sqrt(-2.0 * log(u1));
    return fmax(0.0, fmin(((double)0x3FFFFF), tmp * cos(two_pi * u2) + ((double)0x1FFF)));
}

/*
 * fill packet with 16-bit samples
 * include_index: shall the file index be included at the first two byte?
 * length: the length of the buffer which can be filled with data
 */
void generate_data(uint8_t *buffer, uint8_t length, bool include_index) {
    if (length % 2 != 0) {
        printf("WARNING: generate_data has been used with an odd length.");
    }

    uint8_t data_start = 0;
    if (include_index) {
        buffer[0] = (uint8_t)(file_position >> 8);
        buffer[1] = (uint8_t)(file_position & 0x00FF);
        data_start = 2;
    }
    for (uint8_t i = data_start; i < length; i = i + 2) {
        uint16_t sample = generate_sample();
        buffer[i] = (uint8_t)(sample >> 8);
        buffer[i + 1] = (uint8_t)(sample & 0x00FF);
    }
}

void hex_print(uint8_t *arr, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x ", arr[i]);
    }
    printf("\n");
}

int main() {
    static uint8_t message[buffer_size(PAYLOADSIZE + 2, HEADER_LEN) * 4] = {0}; // include 10 header bytes
    static uint32_t buffer[buffer_size(PAYLOADSIZE, HEADER_LEN)] = {0};         // initialize the buffer
    static uint8_t seq = 0;
    uint8_t header_tmplate[10] = {0xaa, 0xaa, 0xaa, 0xaa, 0x93, 0x0b, 0x51, 0xde, 0x00, 0x00}; // CC1352P7, the last two byte one for the payload length. and another is seq number

    uint8_t tx_payload_buffer[PAYLOADSIZE];

    /* Hamming setup */
    hamming_cfg_t hamming_cfg;
    hamming_init(&hamming_cfg, HAMMING_R);
    uint8_t raw_data[MESSAGE_LEN];   /* data before encoding */
    uint8_t rx_decoded[MESSAGE_LEN]; /* data after decoding */

    for (size_t i = 0; i < 20; i++) {
        printf("\n%lu MSG:\n", i);
        generate_data(raw_data, MESSAGE_LEN, true);
        hex_print(raw_data, MESSAGE_LEN);
        hamming_encode_buffer(&hamming_cfg, raw_data, MESSAGE_BITS,
                              tx_payload_buffer);

        /* add header (10 byte) to packet */
        add_header(&message[0], seq, header_tmplate);
        /* add Hamming-encoded payload to packet */
        memcpy(&message[HEADER_LEN], tx_payload_buffer, PAYLOADSIZE);
        printf("Encoded:\n");
        hex_print(&message[HEADER_LEN], PAYLOADSIZE);
    }
}