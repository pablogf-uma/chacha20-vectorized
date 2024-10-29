# include <stdio.h>
# include <stdint.h>
# include <string.h>
# include "chacha20_v_functions.h"
# include <immintrin.h>

void permute_v_state(uint32_t state[16], uint32_t *v0, uint32_t *v1, uint32_t *v2, uint32_t *v3, uint32_t output_keystream[64])
{
    // Make copy of the original state, for later addition to the permuted state
    uint32_t original_state[16];
    for (int i = 0; i < 16; i++) {
        original_state[i] = state[i];
    }

    // Initialize vectors
    state_to_vectors(state, v0, v1, v2, v3);

    // Use of vectors for vectorized addition after the rounds
    __m128i v0_og = _mm_loadu_si128((__m128i*)v0);
    __m128i v1_og = _mm_loadu_si128((__m128i*)v1);
    __m128i v2_og = _mm_loadu_si128((__m128i*)v2);
    __m128i v3_og = _mm_loadu_si128((__m128i*)v3);

    // Perform permutations on the state: 20 total rounds (10 column-diagonal operations)
    for (int i = 0; i < 10; i++) {
        double_whole_round(v0, v1, v2, v3);
    }

    __m128i v0_permuted = _mm_loadu_si128((__m128i*)v0);
    __m128i v1_permuted = _mm_loadu_si128((__m128i*)v1);
    __m128i v2_permuted = _mm_loadu_si128((__m128i*)v2);
    __m128i v3_permuted = _mm_loadu_si128((__m128i*)v3);

    // Add the permuted vectors to the original vectors for pseudo-randomness:
    v0_permuted = _mm_add_epi32(v0_permuted, v0_og);
    v1_permuted = _mm_add_epi32(v1_permuted, v1_og);
    v2_permuted = _mm_add_epi32(v2_permuted, v2_og);
    v3_permuted = _mm_add_epi32(v3_permuted, v3_og);

    // Load permuted vectors back into the state
    vectors_to_state(state, v0_permuted, v1_permuted, v2_permuted, v3_permuted);

    // Serialize the permuted state into the output keystream
    for (size_t i = 0; i < 16; i++) {
        uint32_t word = state[i];
        output_keystream[i * 4] = (word >> 0)  & 0xFF;
        output_keystream[i * 4 + 1] = (word >> 8)  & 0xFF;
        output_keystream[i * 4 + 2] = (word >> 16) & 0xFF;
        output_keystream[i * 4 + 3] = (word >> 24) & 0xFF;
    }

    /*
    // TESTING: Output the permuted vectors, state, and keystream
    __m128i vectors[4] = {v0_permuted, v1_permuted, v2_permuted, v3_permuted};

    for (int i = 0; i < 4; i++)
    {
        printf("Vector %i:\n", i + 1);

        for (int b = 0; b < 4; b++)
        {
            printf("%08x", vectors[i][b]);
        }
        printf("\n\n");
    }

    for (int a = 0; a < 4; a++) 
    {
        for (int b = 0; b < 4; b++)
        {
            printf("%08x ", state[a * 4 + b]);  
        }
        printf("\n");  
    }
    printf("\n");

    
    for (int i = 0; i < 64; i++) {
        printf("%02x", output_keystream[i]);
        printf(":");
    }
    printf("\b \b");
    printf("\n");
    */
}
