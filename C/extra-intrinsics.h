/* similar to div, ldiv, lldiv and imaxdiv, but for shorts
 * computes n/d and n%d and stores quotient in low byte, remainder in high byte
 * results of / must fit in a byte, for a divisor of 255 max numerator = 65279
 * it is good for finding an average of up to 255 bytes
 */
static inline unsigned short wdiv(unsigned short n, unsigned char d){
  __asm("divb	%1" : "+ax"(n) :"q"(d));
  return n;
}
