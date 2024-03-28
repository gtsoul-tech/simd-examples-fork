#include <emmintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1000000000
typedef uint16_t dctcoef;
void
print_vector(__m128i v) {
    uint16_t buf[8];
    _mm_storeu_si128((__m128i *) buf, v);
    for (int i = 0; i < 8; i++) {
        printf("%x ", buf[i]);
    }
    printf("\n");
}

void
dct4x4dc(dctcoef d[16]) {
    dctcoef tmp[16];

    for (int i = 0; i < 4; i++) {
        int s01 = d[i * 4 + 0] + d[i * 4 + 1];
        int d01 = d[i * 4 + 0] - d[i * 4 + 1];
        int s23 = d[i * 4 + 2] + d[i * 4 + 3];
        int d23 = d[i * 4 + 2] - d[i * 4 + 3];

        tmp[0 * 4 + i] = s01 + s23;
        tmp[1 * 4 + i] = s01 - s23;
        tmp[2 * 4 + i] = d01 - d23;
        tmp[3 * 4 + i] = d01 + d23;
    }

    printf("\n");
    for (int i = 0; i < 16; i += 4) {
        printf("%02x %02x %02x %02x\n", tmp[i], tmp[i + 1], tmp[i + 2], tmp[i + 3]);
    }

    for (int i = 0; i < 4; i++) {
        int s01 = tmp[i * 4 + 0] + tmp[i * 4 + 1];
        int d01 = tmp[i * 4 + 0] - tmp[i * 4 + 1];
        int s23 = tmp[i * 4 + 2] + tmp[i * 4 + 3];
        int d23 = tmp[i * 4 + 2] - tmp[i * 4 + 3];

        d[i * 4 + 0] = (s01 + s23 + 1) >> 1;
        d[i * 4 + 1] = (s01 - s23 + 1) >> 1;
        d[i * 4 + 2] = (d01 - d23 + 1) >> 1;
        d[i * 4 + 3] = (d01 + d23 + 1) >> 1;
    }
}

void
dct4x4dc_sse(dctcoef d[16]) {
    dctcoef dT[4][4];   // This will hold the transposed matrix

    // Load the rows of d into 128-bit vectors
    __m128i row1row2 = _mm_loadu_si128((__m128i *) &d[0]);
    __m128i row3row4 = _mm_loadu_si128((__m128i *) &d[8]);   // load instead of set
    __m128i tmp1 = _mm_unpacklo_epi16(row1row2, row3row4);
    __m128i tmp3 = _mm_unpackhi_epi16(row1row2, row3row4);
    row1row2 = _mm_unpacklo_epi16(tmp1, tmp3);
    row3row4 = _mm_unpackhi_epi16(tmp1, tmp3);

    __m128i totalSum1 = _mm_add_epi16(row1row2, row3row4);
    __m128i totalSum2 = _mm_sub_epi16(row1row2, row3row4);
    __m128i shuffled1 = _mm_shuffle_epi32(totalSum1, _MM_SHUFFLE(2, 3, 3, 2));
    __m128i shuffled2 = _mm_shuffle_epi32(totalSum2, _MM_SHUFFLE(2, 3, 3, 2));
    totalSum1 = _mm_add_epi16(totalSum1, shuffled1);
    totalSum2 = _mm_add_epi16(totalSum2, shuffled2);
    // 1st + 2nd +3rd +4rth
    // 1st + 2nd -3rd -4rth
    // 1st - 2nd -3rd +4rth
    __m128i maskFF = _mm_set1_epi16(0xFF);
    __m128i zero = _mm_setzero_si128();
    __m128i mask1 = _mm_slli_si128(maskFF, 8);
    __m128i mask2 = _mm_srli_si128(maskFF, 8);
    __m128i masked_part = _mm_and_si128(mask1, row1row2);
    __m128i neg_masked_part = _mm_sub_epi16(zero, masked_part);
    row1row2 = _mm_or_si128(_mm_andnot_si128(mask1, row1row2), neg_masked_part);
    masked_part = _mm_and_si128(mask2, row3row4);
    neg_masked_part = _mm_sub_epi16(zero, masked_part);
    row3row4 = _mm_or_si128(_mm_andnot_si128(mask2, row3row4), neg_masked_part);

    __m128i totalSum3 = _mm_add_epi16(row1row2, row3row4);
    __m128i shuffled3 = _mm_shuffle_epi32(totalSum3, _MM_SHUFFLE(2, 3, 3, 2));
    totalSum3 = _mm_add_epi16(totalSum3, shuffled3);
    // 1st - 2nd +3rd -4rth
    __m128i totalSum4 = _mm_sub_epi16(row1row2, row3row4);
    __m128i shuffled4 = _mm_shuffle_epi32(totalSum4, _MM_SHUFFLE(2, 3, 3, 2));
    totalSum4 = _mm_add_epi16(totalSum4, shuffled4);

    _mm_storel_epi64((__m128i *) dT[0], totalSum1);
    _mm_storel_epi64((__m128i *) dT[1], totalSum2);
    _mm_storel_epi64((__m128i *) dT[2], totalSum3);
    _mm_storel_epi64((__m128i *) dT[3], totalSum4);
    printf("\n");
    /*
    // PHASE 2
    row1row2 = _mm_loadu_si128((__m128i *) &dT[0][0]);
    row3row4 = _mm_loadu_si128((__m128i *) &dT[2][0]);   // load instead of set

    // transpose dT back
    __m128i ones = _mm_set1_epi32(1);   // to divide
    tmp1 = _mm_unpacklo_epi16(row1row2, row3row4);
    tmp3 = _mm_unpackhi_epi16(row1row2, row3row4);
    row1row2 = _mm_unpacklo_epi16(tmp1, tmp3);
    row3row4 = _mm_unpackhi_epi16(tmp1, tmp3);

    printf("\n\n");
    print_vector(row1row2);
    print_vector(row3row4);
    /*
    __m128i row1 = _mm_loadu_si128((__m128i *) &d[0]);
    __m128i extended_row1 = _mm_unpacklo_epi16(row1, zero);
    __m128i row2 = _mm_loadu_si128((__m128i *) &d[4]);
    __m128i extended_row2 = _mm_unpacklo_epi16(row2, zero);
    __m128i row3 = _mm_loadu_si128((__m128i *) &d[8]);
    __m128i extended_row3 = _mm_unpacklo_epi16(row3, zero);
    __m128i row4 = _mm_loadu_si128((__m128i *) &d[12]);
    __m128i extended_row4 = _mm_unpacklo_epi16(row4, zero);
    // 1st + 2nd +3rd +4rth

    __m128i tempSum = _mm_add_epi32(extended_row1, extended_row2);
    totalSum = _mm_add_epi32(extended_row3, extended_row4);
    totalSum = _mm_add_epi32(totalSum, tempSum);
    totalSum = _mm_add_epi32(totalSum, ones);
    totalSum = _mm_srli_epi32(totalSum, 1);

    print_vector(totalSum);
    _mm_storeu_si64((__m128i *) &d[0], _mm_packs_epi32(totalSum, totalSum));

    printf("\n");
    for (int i = 0; i < 16; i += 4) {
        printf("%02x %02x %02x %02x\n", d[i], d[i + 1], d[i + 2], d[i + 3]);
    }
    // 1st + 2nd +3rd +4rth
    totalSum = _mm_add_epi32(row1, row3row4);
    shuffled = _mm_shuffle_epi32(totalSum, _MM_SHUFFLE(2, 3, 3, 2));
    totalSum = _mm_add_epi16(totalSum, shuffled);

    printf("\n");
    totalSum = _mm_add_epi16(totalSum, ones);
    _mm_storel_epi64((__m128i *) &d[0], _mm_srli_epi16(totalSum, 1));
    * /
    /*
       // 1st + 2nd -3rd -4rth
       // 1st - 2nd -3rd +4rth
       // 1st - 2nd +3rd -4rth
       */
}
int
main() {
    struct timespec start, mid, end;
    srand(time(NULL));
    dctcoef matrix[16];
    dctcoef matrix2[16];
    for (int i = 0; i < 16; i++) {
        matrix[i] = rand() & 0xFF;   // 8 bit unsigned
        matrix2[i] = matrix[i];
    }
    printf("Original matrix:\n");
    for (int i = 0; i < 16; i += 4) {
        printf("%02x %02x %02x %02x\n", matrix[i], matrix[i + 1], matrix[i + 2], matrix[i + 3]);
    }
    clock_gettime(CLOCK_MONOTONIC, &start);
    dct4x4dc(matrix);
    clock_gettime(CLOCK_MONOTONIC, &mid);
    dct4x4dc_sse(matrix2);
    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("\nMatrix after dct4x4dc:\n");
    for (int i = 0; i < 16; i += 4) {
        printf("%02x %02x %02x %02x\n", matrix[i], matrix[i + 1], matrix[i + 2], matrix[i + 3]);
    }

    printf("\nMatrix2 after dct4x4dc_sse:\n");
    for (int i = 0; i < 16; i += 4) {
        printf("%02x %02x %02x %02x\n", matrix2[i], matrix2[i + 1], matrix2[i + 2], matrix2[i + 3]);
    }

    long seconds1 = mid.tv_sec - start.tv_sec;
    long nanoseconds1 = mid.tv_nsec - start.tv_nsec;
    if (nanoseconds1 < 0) {
        seconds1--;
        nanoseconds1 += 1000000000;
    }
    long seconds2 = end.tv_sec - mid.tv_sec;
    long nanoseconds2 = end.tv_nsec - mid.tv_nsec;
    if (nanoseconds2 < 0) {
        seconds2--;
        nanoseconds2 += 1000000000;
    }
    printf("scalar: %ld.%09ld seconds\n", seconds1, nanoseconds1);
    printf("SSE   : %ld.%09ld seconds\n", seconds2, nanoseconds2);
    return 0;
}