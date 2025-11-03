#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Get a globally unique hint value for hash functions and randomization
 *
 * Returns a pseudo-random value derived from a global atomic counter that is incremented
 * by a constant on each call. The counter is initialized with high-resolution time data
 * at program startup (performance counter on Windows, nanosecond timestamp on other systems).
 * The returned value is processed through splitmix64 to ensure good distribution.
 *
 * This function is thread-safe and provides different values on each call, making it
 * suitable for initializing hash function seeds, random number generators, or any
 * situation where a unique hint value is needed.
 *
 * @return A pseudo-random 64-bit value unique to this call
 *
 * @example
 *   // Initialize hash table with unique seed
 *   uint64_t seed = ov_rand_get_global_hint();
 *   hashmap_init(&map, seed);
 *
 *   // Generate multiple independent seeds
 *   uint64_t seed1 = ov_rand_get_global_hint();
 *   uint64_t seed2 = ov_rand_get_global_hint();
 *   assert(seed1 != seed2);  // Always different
 */
uint64_t ov_rand_get_global_hint(void);

/**
 * @brief SplitMix64 hash function for 64-bit values
 *
 * Transforms a 64-bit input into a well-distributed 64-bit output.
 * Useful for generating hash values from sequential or poorly distributed inputs.
 *
 * @param x Input 64-bit value to hash
 * @return Hashed 64-bit value with good distribution properties
 *
 * @example
 *   // Hash memory addresses (avoids alignment bias in lower bits)
 *   void *ptr = malloc(100);
 *   uint64_t bucket = ov_rand_splitmix64((uintptr_t)ptr) % table_size;
 *   // Without splitmix: ptr % table_size would cluster due to 8/16-byte alignment
 *
 *   // Hash composite keys (ensures all bits contribute to hash)
 *   uint64_t compound_key = (category_id << 32) | item_id;
 *   uint64_t hash = ov_rand_splitmix64(compound_key) % table_size;
 *   // Without splitmix: only lower 32 bits would affect small table_size
 *
 *   // Generate sequence of well-distributed pseudo-random values
 *   uint64_t seq = 0;
 *   for (int i = 0; i < 10; i++) {
 *       uint64_t hash = ov_rand_splitmix64(seq);
 *       printf("hash[%d] = 0x%016llx\n", i, hash);
 *       seq = ov_rand_splitmix64_next(seq);
 *   }
 *
 * @see https://xoshiro.di.unimi.it/splitmix64.c
 * @author Sebastiano Vigna
 * @copyright 2015 Sebastiano Vigna. Public Domain.
 */
static inline uint64_t ov_rand_splitmix64(uint64_t x) {
  x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
  x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
  return x ^ (x >> 31);
}

/**
 * @brief Generate next value in SplitMix64 sequence
 *
 * Increments the input value by the golden ratio constant used in SplitMix64.
 * Used to generate sequential values for the SplitMix64 hash function.
 *
 * @param x Current 64-bit sequence value
 * @return Next value in the sequence (x + golden_ratio_constant)
 *
 * @see ov_rand_splitmix64
 */
static inline uint64_t ov_rand_splitmix64_next(uint64_t const x) { return x + 0x9e3779b97f4a7c15; }

/**
 * @brief SplitMix32 hash function for 32-bit values
 *
 * Transforms a 32-bit input into a well-distributed 32-bit output.
 * Similar to SplitMix64 but optimized for 32-bit values.
 *
 * @param x Input 32-bit value to hash
 * @return Hashed 32-bit value with good distribution properties
 *
 * @see ov_rand_splitmix64 for detailed usage patterns and benefits
 * @see https://github.com/skeeto/hash-prospector
 * @author Chris Wellons (skeeto)
 * @copyright Public Domain (Unlicense)
 */
static inline uint32_t ov_rand_splitmix32(uint32_t x) {
  x = (x + 1) ^ (x >> 17);
  x = (x ^ (x >> 11)) * 0xed5ad4bb;
  x = (x ^ (x >> 15)) * 0xac4c1b51;
  x = (x ^ (x >> 14)) * 0x31848bab;
  return x ^ (x >> 14);
}

/**
 * @brief Generate next value in SplitMix32 sequence
 *
 * Increments the input value by the golden ratio constant used in SplitMix32.
 * Used to generate sequential values for the SplitMix32 hash function.
 *
 * @param x Current 32-bit sequence value
 * @return Next value in the sequence (x + golden_ratio_constant)
 *
 * @see ov_rand_splitmix32
 */
static inline uint32_t ov_rand_splitmix32_next(uint32_t const x) { return x + 0x9e3779b9; }

/**
 * @brief Xoshiro256++ pseudorandom number generator context
 *
 * High-quality, all-purpose 64-bit pseudorandom number generator with excellent speed
 * and statistical properties. Suitable for general use including parallel applications.
 *
 * @see https://prng.di.unimi.it/xoshiro256plusplus.c
 * @author David Blackman and Sebastiano Vigna
 * @copyright 2018 David Blackman and Sebastiano Vigna. Public Domain.
 */
struct ov_rand_xoshiro256pp {
  uint64_t s[4];
};

/**
 * @brief Rotate left operation for 64-bit values (internal use)
 *
 * @param x Value to rotate
 * @param k Number of bits to rotate left
 * @return Rotated value
 */
static inline uint64_t ov_rand_xoshiro256pp_rotl64(uint64_t const x, int const k) { return (x << k) | (x >> (64 - k)); }

/**
 * @brief Initialize Xoshiro256++ generator
 *
 * Sets up the initial state using SplitMix64 for proper seed distribution.
 * The seed must be non-zero to avoid degenerate states.
 *
 * @param ctx Pointer to xoshiro256++ context to initialize
 * @param seed 64-bit seed value (must be non-zero)
 *
 * @example
 *   struct ov_rand_xoshiro256pp rng;
 *   uint64_t seed = ov_rand_get_global_hint();
 *   ov_rand_xoshiro256pp_init(&rng, seed);
 *
 *   // Generate random numbers
 *   for (int i = 0; i < 1000; i++) {
 *       uint64_t random = ov_rand_xoshiro256pp_next(&rng);
 *       printf("Random: %llu\n", random);
 *   }
 */
static inline void ov_rand_xoshiro256pp_init(struct ov_rand_xoshiro256pp *const ctx, uint64_t seed) {
  assert(ctx != NULL && "ctx must not be NULL");
  // Use SplitMix64 to properly distribute the seed across all state elements
  for (int i = 0; i < 4; i++) {
    ctx->s[i] = ov_rand_splitmix64(seed);
    seed = ov_rand_splitmix64_next(seed);
  }
}

/**
 * @brief Generate next random number from Xoshiro256++
 *
 * Produces a high-quality 64-bit pseudorandom number with excellent
 * statistical properties suitable for all applications.
 *
 * @param ctx Pointer to xoshiro256++ context
 * @return 64-bit pseudorandom number
 *
 * @example
 *   struct ov_rand_xoshiro256pp rng;
 *   ov_rand_xoshiro256pp_init(&rng, 12345);
 *
 *   uint64_t dice = (ov_rand_xoshiro256pp_next(&rng) % 6) + 1;
 *   double uniform = ov_rand_xoshiro256pp_next(&rng) / (double)UINT64_MAX;
 */
static inline uint64_t ov_rand_xoshiro256pp_next(struct ov_rand_xoshiro256pp *const ctx) {
  assert(ctx != NULL && "ctx must not be NULL");
  uint64_t const result = ov_rand_xoshiro256pp_rotl64(ctx->s[0] + ctx->s[3], 23) + ctx->s[0];
  uint64_t const t = ctx->s[1] << 17;

  ctx->s[2] ^= ctx->s[0];
  ctx->s[3] ^= ctx->s[1];
  ctx->s[1] ^= ctx->s[2];
  ctx->s[0] ^= ctx->s[3];

  ctx->s[2] ^= t;
  ctx->s[3] = ov_rand_xoshiro256pp_rotl64(ctx->s[3], 45);

  return result;
}

/**
 * @brief Xoshiro128++ pseudorandom number generator context
 *
 * High-quality, all-purpose 32-bit pseudorandom number generator optimized
 * for environments where 64-bit operations are expensive (e.g., WebAssembly).
 *
 * @see https://prng.di.unimi.it/xoshiro128plusplus.c
 * @author David Blackman and Sebastiano Vigna
 * @copyright 2018 David Blackman and Sebastiano Vigna. Public Domain.
 */
struct ov_rand_xoshiro128pp {
  uint32_t s[4];
};

/**
 * @brief Rotate left operation for 32-bit values (internal use)
 *
 * @param x Value to rotate
 * @param k Number of bits to rotate left
 * @return Rotated value
 */
static inline uint32_t ov_rand_xoshiro128pp_rotl32(uint32_t const x, int const k) { return (x << k) | (x >> (32 - k)); }

/**
 * @brief Initialize Xoshiro128++ generator
 *
 * Sets up the initial state using SplitMix32 for proper seed distribution.
 * The seed must be non-zero to avoid degenerate states.
 *
 * @param ctx Pointer to xoshiro128++ context to initialize
 * @param seed 32-bit seed value (must be non-zero)
 *
 * @example
 *   struct ov_rand_xoshiro128pp rng;
 *   uint32_t seed = (uint32_t)ov_rand_get_global_hint();
 *   ov_rand_xoshiro128pp_init(&rng, seed);
 *
 *   // Generate random numbers in WebAssembly environment
 *   for (int i = 0; i < 1000; i++) {
 *       uint32_t random = ov_rand_xoshiro128pp_next(&rng);
 *       printf("Random: %u\n", random);
 *   }
 */
static inline void ov_rand_xoshiro128pp_init(struct ov_rand_xoshiro128pp *const ctx, uint32_t seed) {
  assert(ctx != NULL && "ctx must not be NULL");
  // Use SplitMix32 to properly distribute the seed across all state elements
  for (int i = 0; i < 4; i++) {
    ctx->s[i] = ov_rand_splitmix32(seed);
    seed = ov_rand_splitmix32_next(seed);
  }
}

/**
 * @brief Generate next random number from Xoshiro128++
 *
 * Produces a high-quality 32-bit pseudorandom number optimized for
 * environments where 64-bit operations are expensive.
 *
 * @param ctx Pointer to xoshiro128++ context
 * @return 32-bit pseudorandom number
 *
 * @example
 *   struct ov_rand_xoshiro128pp rng;
 *   ov_rand_xoshiro128pp_init(&rng, 42);
 *
 *   uint32_t dice = (ov_rand_xoshiro128pp_next(&rng) % 6) + 1;
 *   float uniform = ov_rand_xoshiro128pp_next(&rng) / (float)UINT32_MAX;
 */
static inline uint32_t ov_rand_xoshiro128pp_next(struct ov_rand_xoshiro128pp *const ctx) {
  assert(ctx != NULL && "ctx must not be NULL");
  uint32_t const result = ov_rand_xoshiro128pp_rotl32(ctx->s[0] + ctx->s[3], 7) + ctx->s[0];
  uint32_t const t = ctx->s[1] << 9;

  ctx->s[2] ^= ctx->s[0];
  ctx->s[3] ^= ctx->s[1];
  ctx->s[1] ^= ctx->s[2];
  ctx->s[0] ^= ctx->s[3];

  ctx->s[2] ^= t;
  ctx->s[3] = ov_rand_xoshiro128pp_rotl32(ctx->s[3], 11);

  return result;
}
