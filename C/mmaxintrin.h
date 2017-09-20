
#if defined(__AVX512BW__)||defined(__AVX2__)||defined(__SSE2__)
#define USEVMAX
#define VMAXFUNC(func,...) PASTE(VMAXPREFIX,func)(__VA_ARGS__)
#define VMAXFUNC_(func,...) PASTE(PASTE(VMAXPREFIX,func),VMAXBITS)(__VA_ARGS__)

#ifdef __AVX512BW__

#define VMAXPREFIX _mm512_
#define VMAXBITS 512
typedef __m512i mmaxi;
#define VMAXALIGNMASK 63UL
#define VCMPMASK(cmp,typ,...) _mm512_##cmp##_##typ##_mask(__VA_ARGS__)

#elif defined(__AVX2__)

#define VMAXPREFIX _mm256_
#define VMAXBITS 256
typedef __m256i mmaxi;
#define VMAXALIGNMASK 31UL
#define VCMPMASK(cmp,typ,...) _mm256_movemask_##typ(_mm256_##cmp##_##typ(__VA_ARGS__))

#elif defined(__SSE2__)

#define VMAXPREFIX _mm_
#define VMAXBITS 128
typedef __m128i mmaxi;
#define VMAXALIGNMASK 15UL
#define VCMPMASK(cmp,typ,...) _mm_movemask_##typ(_mm_##cmp##_##typ(__VA_ARGS__))

#endif

#endif
