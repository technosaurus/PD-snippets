#include <stdint.h>
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define S2U32(a,b,c,d) ((uint32_t)(d)<<24)|((uint32_t)(c)<<16)|((uint32_t)(b)<<8)|(a)
#define S2U64(a,b,c,d,e,f,g,h) ((uint64_t)S2U32(a,b,c,d)) | (((uint64_t)S2U32(e,f,g,h))<<32)

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define S2U32(a,b,c,d) (((uint32_t)(a)<<24)|(((uint32_t)(b))<<16)|((uint32_t)(c)<<8)|(d))
#define S2U64(a,b,c,d,e,f,g,h) ((uint64_t)S2U32(e,f,g,h)) | (((uint64_t)S2U32(a,b,c,d))<<32)

#else
#error this endianness not supported
#endif

static inline uint32_t s2u32(void *s){return *((uint32_t *)s);}
static inline uint64_t s2u64(void *s){return *((uint64_t *)s);}

/* usage:
  switch(s2u32(mystring)){
  case S2U32('f','o','o','\0') : return foo();
  case S2U32('b','a','r','\0') : return bar();
  case S2U32('b','a','z','\0') : return baz();
  default: break;
  }
  //similar for 64bit if compiler supports 64bit switch
*/
