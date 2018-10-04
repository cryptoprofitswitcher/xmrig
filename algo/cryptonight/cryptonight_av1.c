/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017      fireice-uk  <https://github.com/fireice-uk>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2018      Lee Clagett <https://github.com/vtnerd>
 * Copyright 2016-2018 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <x86intrin.h>
#include <string.h>

#include "crypto/c_keccak.h"
#include "cryptonight.h"
#include "cryptonight_aesni.h"
#include "cryptonight_monero.h"


void cryptonight_av1_v0(const uint8_t *restrict input, size_t size, uint8_t *restrict output, struct cryptonight_ctx **restrict ctx)
{
    keccak(input, size, ctx[0]->state, 200);

    cn_explode_scratchpad((__m128i*) ctx[0]->state, (__m128i*) ctx[0]->memory);

    const uint8_t* l0 = ctx[0]->memory;
    uint64_t* h0 = (uint64_t*) ctx[0]->state;

    uint64_t al0 = h0[0] ^ h0[4];
    uint64_t ah0 = h0[1] ^ h0[5];
    __m128i bx0 = _mm_set_epi64x(h0[3] ^ h0[7], h0[2] ^ h0[6]);

    uint64_t idx0 = h0[0] ^ h0[4];

    for (size_t i = 0; __builtin_expect(i < 0x80000, 1); i++) {
        __m128i cx;
        cx = _mm_load_si128((__m128i *) &l0[idx0 & 0x1FFFF0]);
        cx = _mm_aesenc_si128(cx, _mm_set_epi64x(ah0, al0));

        _mm_store_si128((__m128i *) &l0[idx0 & 0x1FFFF0], _mm_xor_si128(bx0, cx));
        idx0 = EXTRACT64(cx);
        bx0 = cx;

        uint64_t hi, lo, cl, ch;
        cl = ((uint64_t*) &l0[idx0 & 0x1FFFF0])[0];
        ch = ((uint64_t*) &l0[idx0 & 0x1FFFF0])[1];
        lo = _umul128(idx0, cl, &hi);

        al0 += hi;
        ah0 += lo;

        ((uint64_t*)&l0[idx0 & 0x1FFFF0])[0] = al0;
        ((uint64_t*)&l0[idx0 & 0x1FFFF0])[1] = ah0;

        ah0 ^= ch;
        al0 ^= cl;
        idx0 = al0;
    }

    cn_implode_scratchpad((__m128i*) ctx[0]->memory, (__m128i*) ctx[0]->state);

    keccakf(h0, 24);
    extra_hashes[ctx[0]->state[0] & 3](ctx[0]->state, 200, output);
}


void cryptonight_av1_v1(const uint8_t *restrict input, size_t size, uint8_t *restrict output, struct cryptonight_ctx **restrict ctx)
{
    if (size < 43) {
        memset(output, 0, 32);
        return;
    }

    keccak(input, size, ctx[0]->state, 200);

    VARIANT1_INIT(0);

    cn_explode_scratchpad((__m128i*) ctx[0]->state, (__m128i*) ctx[0]->memory);

    const uint8_t* l0 = ctx[0]->memory;
    uint64_t* h0 = (uint64_t*) ctx[0]->state;

    uint64_t al0 = h0[0] ^ h0[4];
    uint64_t ah0 = h0[1] ^ h0[5];
    __m128i bx0 = _mm_set_epi64x(h0[3] ^ h0[7], h0[2] ^ h0[6]);

    uint64_t idx0 = h0[0] ^ h0[4];

    for (size_t i = 0; __builtin_expect(i < 0x80000, 1); i++) {
        __m128i cx;
        cx = _mm_load_si128((__m128i *) &l0[idx0 & 0x1FFFF0]);
        cx = _mm_aesenc_si128(cx, _mm_set_epi64x(ah0, al0));

        cryptonight_monero_tweak((uint64_t*)&l0[idx0 & 0x1FFFF0], _mm_xor_si128(bx0, cx));

        idx0 = EXTRACT64(cx);
        bx0 = cx;

        uint64_t hi, lo, cl, ch;
        cl = ((uint64_t*) &l0[idx0 & 0x1FFFF0])[0];
        ch = ((uint64_t*) &l0[idx0 & 0x1FFFF0])[1];
        lo = _umul128(idx0, cl, &hi);

        al0 += hi;
        ah0 += lo;

        ((uint64_t*)&l0[idx0 & 0x1FFFF0])[0] = al0;
        ((uint64_t*)&l0[idx0 & 0x1FFFF0])[1] = ah0 ^ tweak1_2_0;

        ah0 ^= ch;
        al0 ^= cl;
        idx0 = al0;
    }

    cn_implode_scratchpad((__m128i*) ctx[0]->memory, (__m128i*) ctx[0]->state);

    keccakf(h0, 24);
    extra_hashes[ctx[0]->state[0] & 3](ctx[0]->state, 200, output);
}


void cryptonight_av1_v2(const uint8_t *restrict input, size_t size, uint8_t *restrict output, struct cryptonight_ctx **restrict ctx)
{
    keccak(input, size, ctx[0]->state, 200);

    cn_explode_scratchpad((__m128i*) ctx[0]->state, (__m128i*) ctx[0]->memory);

    const uint8_t* l0 = ctx[0]->memory;
    uint64_t* h0 = (uint64_t*) ctx[0]->state;

    VARIANT2_INIT(0);
    VARIANT2_SET_ROUNDING_MODE();

    uint64_t al0 = h0[0] ^ h0[4];
    uint64_t ah0 = h0[1] ^ h0[5];
    __m128i bx0 = _mm_set_epi64x(h0[3] ^ h0[7], h0[2] ^ h0[6]);
    __m128i bx1 = _mm_set_epi64x(h0[9] ^ h0[11], h0[8] ^ h0[10]);

    uint64_t idx0 = al0;

    for (size_t i = 0; __builtin_expect(i < 0x80000, 1); i++) {
        __m128i cx        = _mm_load_si128((__m128i *) &l0[idx0 & 0x1FFFF0]);
        const __m128i ax0 = _mm_set_epi64x(ah0, al0);

        cx = _mm_aesenc_si128(cx, ax0);

        VARIANT2_SHUFFLE(l0, idx0 & 0x1FFFF0, ax0, bx0, bx1);
        _mm_store_si128((__m128i *) &l0[idx0 & 0x1FFFF0], _mm_xor_si128(bx0, cx));

        idx0 = _mm_cvtsi128_si64(cx);

        uint64_t hi, lo, cl, ch;
        cl = ((uint64_t*) &l0[idx0 & 0x1FFFF0])[0];
        ch = ((uint64_t*) &l0[idx0 & 0x1FFFF0])[1];

        VARIANT2_INTEGER_MATH(0, cl, cx);
        lo = _umul128(idx0, cl, &hi);
        VARIANT2_SHUFFLE2(l0, idx0 & 0x1FFFF0, ax0, bx0, bx1, hi, lo);

        al0 += hi;
        ah0 += lo;

        ((uint64_t*)&l0[idx0 & 0x1FFFF0])[0] = al0;
        ((uint64_t*)&l0[idx0 & 0x1FFFF0])[1] = ah0;

        al0 ^= cl;
        ah0 ^= ch;
        idx0 = al0;

        bx1 = bx0;
        bx0 = cx;
    }

    cn_implode_scratchpad((__m128i*) ctx[0]->memory, (__m128i*) ctx[0]->state);

    keccakf(h0, 24);
    extra_hashes[ctx[0]->state[0] & 3](ctx[0]->state, 200, output);
}