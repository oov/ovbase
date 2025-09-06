#include "common.h"

#include "../mem.h"

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  if __has_warning("-Wextra-semi-stmt")
#    pragma GCC diagnostic ignored "-Wextra-semi-stmt"
#  endif
#  if __has_warning("-Wimplicit-fallthrough")
#    pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#  endif
#endif // __GNUC__
//-----------------------------------------------------------------------------
// SipHash reference C implementation
//
// Copyright (c) 2012-2016 Jean-Philippe Aumasson
// <jeanphilippe.aumasson@gmail.com>
// Copyright (c) 2012-2014 Daniel J. Bernstein <djb@cr.yp.to>
//
// To the extent possible under law, the author(s) have dedicated all copyright
// and related and neighboring rights to this software to the public domain
// worldwide. This software is distributed without any warranty.
//
// You should have received a copy of the CC0 Public Domain Dedication along
// with this software. If not, see
// <http://creativecommons.org/publicdomain/zero/1.0/>.
//-----------------------------------------------------------------------------
uint64_t sip_hash_1_3(const void *data, const size_t inlen, uint64_t seed0, uint64_t seed1) {
#define U8TO64_LE(p)                                                                                                   \
  {(((uint64_t)((p)[0])) | ((uint64_t)((p)[1]) << 8) | ((uint64_t)((p)[2]) << 16) | ((uint64_t)((p)[3]) << 24) |       \
    ((uint64_t)((p)[4]) << 32) | ((uint64_t)((p)[5]) << 40) | ((uint64_t)((p)[6]) << 48) |                             \
    ((uint64_t)((p)[7]) << 56))}
#define U64TO8_LE(p, v)                                                                                                \
  {                                                                                                                    \
    U32TO8_LE((p), (uint32_t)((v)));                                                                                   \
    U32TO8_LE((p) + 4, (uint32_t)((v) >> 32));                                                                         \
  }
#define U32TO8_LE(p, v)                                                                                                \
  {                                                                                                                    \
    (p)[0] = (uint8_t)((v));                                                                                           \
    (p)[1] = (uint8_t)((v) >> 8);                                                                                      \
    (p)[2] = (uint8_t)((v) >> 16);                                                                                     \
    (p)[3] = (uint8_t)((v) >> 24);                                                                                     \
  }
#define ROTL(x, b) (uint64_t)(((x) << (b)) | ((x) >> (64 - (b))))
#define SIPROUND                                                                                                       \
  {                                                                                                                    \
    v0 += v1;                                                                                                          \
    v1 = ROTL(v1, 13);                                                                                                 \
    v1 ^= v0;                                                                                                          \
    v0 = ROTL(v0, 32);                                                                                                 \
    v2 += v3;                                                                                                          \
    v3 = ROTL(v3, 16);                                                                                                 \
    v3 ^= v2;                                                                                                          \
    v0 += v3;                                                                                                          \
    v3 = ROTL(v3, 21);                                                                                                 \
    v3 ^= v0;                                                                                                          \
    v2 += v1;                                                                                                          \
    v1 = ROTL(v1, 17);                                                                                                 \
    v1 ^= v2;                                                                                                          \
    v2 = ROTL(v2, 32);                                                                                                 \
  }
  const uint8_t *in = (const uint8_t *)data;
  uint64_t k0 = U8TO64_LE((uint8_t *)&seed0);
  uint64_t k1 = U8TO64_LE((uint8_t *)&seed1);
  uint64_t v3 = UINT64_C(0x7465646279746573) ^ k1;
  uint64_t v2 = UINT64_C(0x6c7967656e657261) ^ k0;
  uint64_t v1 = UINT64_C(0x646f72616e646f6d) ^ k1;
  uint64_t v0 = UINT64_C(0x736f6d6570736575) ^ k0;
  const uint8_t *end = in + inlen - (inlen % sizeof(uint64_t));
  for (; in != end; in += 8) {
    uint64_t m = U8TO64_LE(in);
    v3 ^= m;
    SIPROUND; // c-rounds = 1 (changed from 2)
    v0 ^= m;
  }
  const int left = inlen & 7;
  uint64_t b = ((uint64_t)inlen) << 56;
  switch (left) {
  case 7:
    b |= ((uint64_t)in[6]) << 48; /* fall through */
  case 6:
    b |= ((uint64_t)in[5]) << 40; /* fall through */
  case 5:
    b |= ((uint64_t)in[4]) << 32; /* fall through */
  case 4:
    b |= ((uint64_t)in[3]) << 24; /* fall through */
  case 3:
    b |= ((uint64_t)in[2]) << 16; /* fall through */
  case 2:
    b |= ((uint64_t)in[1]) << 8; /* fall through */
  case 1:
    b |= ((uint64_t)in[0]);
    break;
  case 0:
    break;
  }
  v3 ^= b;
  SIPROUND; // d-rounds = 1 (changed from 2)
  v0 ^= b;
  v2 ^= 0xff;
  SIPROUND;
  SIPROUND;
  SIPROUND; // finalization rounds = 3 (changed from 4)
  b = v0 ^ v1 ^ v2 ^ v3;
  uint64_t out = 0;
  U64TO8_LE((uint8_t *)&out, b);
  return out;
#undef U8TO64_LE
#undef U64TO8_LE
#undef U32TO8_LE
#undef ROTL
#undef SIPROUND
}
#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif // __GNUC__

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  if __has_warning("-Wreserved-macro-identifier")
#    pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#  endif
#  if __has_warning("-Wcast-align")
#    pragma GCC diagnostic ignored "-Wcast-align"
#  endif
#  if __has_warning("-Wsign-conversion")
#    pragma GCC diagnostic ignored "-Wsign-conversion"
#  endif
#  if __has_warning("-Wextra-semi-stmt")
#    pragma GCC diagnostic ignored "-Wextra-semi-stmt"
#  endif
#  if __has_warning("-Wimplicit-fallthrough")
#    pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#  endif
#  if __has_warning("-Wimplicit-void-ptr-cast")
#    pragma GCC diagnostic ignored "-Wimplicit-void-ptr-cast"
#  endif
#  include "../../3rd/hashmap.c/hashmap.c"
#  pragma GCC diagnostic pop
#else
#  include "../../3rd/hashmap.c/hashmap.c"
#endif // __GNUC__

void *ov_hm_realloc(void *p, size_t const s, void *const udata) {
#ifdef ALLOCATE_LOGGER
  struct ov_hashmap const *const hm = (struct ov_hashmap const *)udata;
  struct ov_filepos const *const filepos = hm ? hm->filepos : NULL;
#else
  (void)udata;
#endif
  void *r = p;
  if (!mem_core_(&r, s MEM_FILEPOS_VALUES_PASSTHRU)) {
    return NULL;
  }
  return r;
}

void ov_hm_free(void *p, void *const udata) {
#ifdef ALLOCATE_LOGGER
  struct ov_hashmap const *const hm = (struct ov_hashmap const *)udata;
  struct ov_filepos const *const filepos = hm ? hm->filepos : NULL;
#else
  (void)udata;
#endif
  mem_core_(&p, 0 MEM_FILEPOS_VALUES_PASSTHRU);
}
