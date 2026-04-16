#include "hamming.h"

#include <string.h>

/* ------------------------------------------------------------------ */
/* Bit-array helpers (0-based bit index, LSB-first packing)            */
/* ------------------------------------------------------------------ */

static inline uint8_t get_bit(const uint8_t *arr, uint8_t idx) {
    return (arr[idx >> 3] >> (idx & 7)) & 1u;
}

static inline void set_bit(uint8_t *arr, uint8_t idx, uint8_t val) {
    if (val)
        arr[idx >> 3] |=  (uint8_t)(1u << (idx & 7));
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

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

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
            dst[d >> 3] |=  (uint8_t)(1u << (d & 7));
        else
            dst[d >> 3] &= (uint8_t)~(1u << (d & 7));
    }
}

int hamming_decode(const hamming_cfg_t *cfg, uint8_t *codeword, uint8_t *data) {
    /*
     * Compute syndrome: for each parity bit i, XOR all bits at positions
     * where bit (i-1) is set.  A non-zero result in check i contributes
     * 2^(i-1) to the syndrome, which equals the 1-indexed error position.
     */
    uint8_t syndrome = 0;
    for (uint8_t i = 0; i < cfg->r; i++) {
        uint8_t parity_pos = (uint8_t)(1u << i);
        uint8_t parity = 0;
        for (uint8_t pos = 1; pos <= cfg->n; pos++) {
            if (pos & parity_pos)
                parity ^= get_bit(codeword, pos - 1);
        }
        if (parity)
            syndrome |= parity_pos;
    }

    int result = 0;
    if (syndrome != 0) {
        if (syndrome <= cfg->n) {
            /* Correct the single-bit error (syndrome = 1-indexed error position). */
            flip_bit(codeword, syndrome - 1);
            result = 1;
        } else {
            result = -1;
        }
    }

    /* Extract data bits from non-parity positions. */
    uint8_t data_bytes = HAMMING_DATA_BYTES(cfg->r);
    memset(data, 0, data_bytes);
    uint8_t data_idx = 0;
    for (uint8_t pos = 1; pos <= cfg->n; pos++) {
        if (!is_parity_pos(pos)) {
            set_bit(data, data_idx, get_bit(codeword, pos - 1));
            data_idx++;
        }
    }

    return result;
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
        memset(tmp_cw,   0, sizeof(tmp_cw));
        copy_bits(tmp_data, 0, src, i * cfg->k, cfg->k);
        hamming_encode(cfg, tmp_data, tmp_cw);
        copy_bits(dst, i * cfg->n, tmp_cw, 0, cfg->n);
    }
}

int hamming_decode_buffer(const hamming_cfg_t *cfg, const uint8_t *src,
                          uint16_t src_bits, uint8_t *dst) {
    uint8_t tmp_cw[HAMMING_CW_BYTES(HAMMING_MAX_R)];
    uint8_t tmp_data[HAMMING_DATA_BYTES(HAMMING_MAX_R)];
    uint16_t num_chunks = src_bits / cfg->n;
    uint16_t dst_bits = (uint16_t)num_chunks * cfg->k;

    memset(dst, 0, (dst_bits + 7u) / 8u);

    int total_errors = 0;
    for (uint16_t i = 0; i < num_chunks; i++) {
        memset(tmp_cw,   0, sizeof(tmp_cw));
        memset(tmp_data, 0, sizeof(tmp_data));
        copy_bits(tmp_cw, 0, src, i * cfg->n, cfg->n);
        int result = hamming_decode(cfg, tmp_cw, tmp_data);
        if (result < 0)
            return -1;
        total_errors += result;
        copy_bits(dst, i * cfg->k, tmp_data, 0, cfg->k);
    }
    return total_errors;
}
