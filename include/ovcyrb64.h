#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * @brief cyrb64 hash context structure
 *
 * Fast and simple 64-bit hash function with decent collision resistance.
 * Inspired by MurmurHash2/3, uses multiplication and Xorshift techniques.
 * Based on cyrb53, NOT cryptographically secure - do not use for security purposes.
 *
 * @see https://github.com/bryc/code/blob/master/jshash/experimental/cyrb53.js
 * @author bryc (github.com/bryc)
 * @copyright 2018 bryc. Public domain.
 */
struct ov_cyrb64 {
  uint32_t h1;
  uint32_t h2;
};

/**
 * @brief Initialize cyrb64 hash context
 *
 * Sets up the initial state with seed variation for consistent hash generation.
 * The seed parameter allows generating different hash values for the same data.
 *
 * @param ctx Pointer to cyrb64 context to initialize
 * @param seed Seed value for hash initialization (0 for default)
 *
 * @example
 *   struct ov_cyrb64 ctx;
 *   ov_cyrb64_init(&ctx, 0);  // Default seed
 *
 *   // Or with random seed
 *   uint32_t seed = (uint32_t)ov_get_global_hint();
 *   ov_cyrb64_init(&ctx, seed);
 */
static inline void ov_cyrb64_init(struct ov_cyrb64 *const ctx, uint32_t const seed) {
  ctx->h1 = 0x91eb9dc7 ^ seed;
  ctx->h2 = 0x41c6ce57 ^ seed;
}

/**
 * @brief Update cyrb64 hash with data
 *
 * Processes 32-bit values through the hash algorithm for good distribution.
 * Can be called multiple times for streaming hash computation.
 *
 * @param ctx Pointer to cyrb64 context
 * @param src Array of 32-bit values to hash
 * @param len Number of 32-bit values in the array
 *
 * @example
 *   // Hash array of integers
 *   uint32_t data[] = {1234, 5678, 9012};
 *   ov_cyrb64_update(&ctx, data, 3);
 *
 *   // Hash string (cast to uint32_t array)
 *   char *str = "Hello World!";
 *   size_t str_len = strlen(str);
 *   size_t word_len = (str_len + 3) / 4;  // Round up to 32-bit boundary
 *   ov_cyrb64_update(&ctx, (const uint32_t*)str, word_len);
 */
static inline void ov_cyrb64_update(struct ov_cyrb64 *const ctx, uint32_t const *const src, size_t const len) {
  for (size_t i = 0; i < len; ++i) {
    ctx->h1 = (ctx->h1 ^ src[i]) * 2654435761;
    ctx->h2 = (ctx->h2 ^ src[i]) * 1597334677;
  }
}

/**
 * @brief Finalize ov_cyrb64 hash and get result
 *
 * Applies final mixing to ensure good avalanche behavior and returns
 * the 64-bit hash value. The context remains unchanged for reuse.
 *
 * @param ctx Pointer to cyrb64 context
 * @return Final 64-bit hash value with good distribution
 *
 * @example
 *   // Complete hash computation
 *   struct ov_cyrb64 ctx;
 *   ov_cyrb64_init(&ctx, 0);
 *
 *   uint32_t data[] = {0x12345678, 0x9abcdef0};
 *   ov_cyrb64_update(&ctx, data, 2);
 *
 *   uint64_t hash = ov_cyrb64_final(&ctx);
 *   printf("Hash: 0x%016llx\n", hash);
 *
 *   // Context can be reused for another hash
 *   ov_cyrb64_init(&ctx, 42);  // Different seed
 *   ov_cyrb64_update(&ctx, data, 2);
 *   uint64_t hash2 = ov_cyrb64_final(&ctx);
 */
static inline uint64_t ov_cyrb64_final(struct ov_cyrb64 const *const ctx) {
  uint32_t h1 = ctx->h1, h2 = ctx->h2;
  h1 = ((h1 ^ (h1 >> 16)) * 2246822507) ^ ((h2 ^ (h2 >> 13)) * 3266489909);
  h2 = ((h2 ^ (h2 >> 16)) * 2246822507) ^ ((h1 ^ (h1 >> 13)) * 3266489909);
  return (((uint64_t)h2) << 32) | ((uint64_t)h1);
}
