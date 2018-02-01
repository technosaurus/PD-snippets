/* similar to div, ldiv, lldiv and imaxdiv, but for shorts
 * computes n/d and n%d and stores quotient in low byte, remainder in high byte
 * results of / must fit in a byte, for a divisor of 255 max numerator = 65279
 * it is good for finding an average of up to 255 bytes
 */
static inline unsigned short wdiv(unsigned short n, unsigned char d){
  __asm("divb	%1" : "+ax"(n) :"q"(d));
  return n;
}

static const __m128i m1 = {0x5555555555555555ULL,0x5555555555555555ULL};
static const __m128i m2 = {0x3333333333333333ULL,0x3333333333333333ULL};
static const __m128i m3 = {0x0f0f0f0f0f0f0f0fULL,0x0f0f0f0f0f0f0f0fULL};
static const __m128i m4 = {0x001f001f001f001fULL,0x001f001f001f001fULL};
static const __m128i m5 = {0x0000003f0000003fULL,0x0000003f0000003fULL};
 
__m128i _mm_popcnt_epi2(__m128i x) {
    __m128i y;
    y = _mm_srli_epi64(x,1);
    y = _mm_and_si128(y,m1);
    return _mm_subs_epu8(x,y);
}

__m128i _mm_popcnt_epi4(__m128i x) {
  __m128i y;
  x = _mm_popcnt_epi2(x);
  y = _mm_srli_epi64 (x,2);
  y = _mm_and_si128(y,m2);
  x = _mm_and_si128(x,m2);
  x = _mm_adds_epu8(x,y);
}

__m128i _mm_popcnt_epi8(__m128i x) {
  __m128i y;
  x = _mm_popcnt_epi4(x);
  y = _mm_srli_epi64(x,4);
  x = _mm_adds_epu8(x,y);
  x = _mm_and_si128(x,m3);
  return x;
}
 
__m128i _mm_popcnt_epi16(__m128i x) {
	x = _mm_popcnt_epi8(x);
	__m128i y = _mm_srli_si128(x,1);
	x = _mm_add_epi16(x,y);
	return _mm_and_si128(x,m4);
}
 
__m128i _mm_popcnt_epi32(__m128i x) {
	x = _mm_popcnt_epi16(x);
	__m128i y = _mm_srli_si128(x,2);
	x = _mm_add_epi32(x,y);
	return _mm_and_si128(x,m5);
}
 
__m128i _mm_popcnt_epi64(__m128i x){
	return _mm_sad_epu8(_mm_popcnt_epi8(x),(__m128i){0});
}
 
int _mm_popcnt_si128 (__m128i x){
	x = _mm_popcnt_epi64(x);
	__m128i y = _mm_srli_si128(x,8);
	return _mm_add_epi64(x,y)[0];
}
