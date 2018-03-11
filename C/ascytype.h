//ctype functions that assume ascii
static inline int isalnum(int c){
	return ((unsigned)c-'0' < 10)|(((unsigned)c|32)-'a' < 26);
}
static inline int isalpha(int c){
	return (((unsigned)c|32)-'a' < 26);
}
static inline int isascii(int c){
	return (unsigned)c<128;
}
static inline int isblank(int c){
	return (c==' ')|(c=='\t');
}
static inline int iscntrl(int c){
	return ((unsigned)c < 0x20) | (c == 0x7f);
}
static inline int isconsonant(int c){
    const unsigned letter_mask = (1<<('b'-'a'))|
    (1<<('c'-'a'))|(1<<('d'-'a'))|(1<<('f'-'a'))|(1<<('g'-'a'))|
    (1<<('h'-'a'))|(1<<('j'-'a'))|(1<<('k'-'a'))|(1<<('l'-'a'))|
    (1<<('m'-'a'))|(1<<('n'-'a'))|(1<<('p'-'a'))|(1<<('q'-'a'))|
    (1<<('r'-'a'))|(1<<('s'-'a'))|(1<<('t'-'a'))|(1<<('v'-'a'))|
    (1<<('w'-'a'))|(1<<('x'-'a'))|(1<<('y'-'a'))|(1<<('z'-'a'));
    unsigned x = (c|32)-'a'; // ~ tolower
    /* if 1<<x is in range of int32 set mask to position relative to `a`
     * as in the mask above otherwise it is set to 0 */
    return ((x<32)<<(x&31)) & letter_mask;
}
static inline int isdigit(int c){
	return (unsigned)c-'0' < 10;
}
static inline int isgraph(int c){
	return (unsigned)c-0x21 < 0x5e;
}
static inline int islower(int c){
	return (unsigned)c-'a' < 26;
}
static inline int isprint(int c){
	return (unsigned)c-0x20 < 0x5f;
}
static inline int ispunct(int c){
	unsigned y=(unsigned)c;return ( ( y-'!' < 95 ) & ( ((y|32)-'a' > 25) | (y-'0' > 9) ) );
}
static inline int isspace(int c){
	return ((unsigned)c-'\t' < 5)|(c == ' ');
}
static inline int isupper(int c){
	return (unsigned)c-'A' < 26;
}
static inline int isvowel(int c){
    const unsigned letter_mask = (1<<('a'-'a'))|(1<<('e'-'a'))
		|(1<<('i'-'a'))|(1<<('o'-'a'))|(1<<('u'-'a'));
    unsigned x = (c|32)-'a'; // ~ tolower
    return ((x<32)<<(x&31)) & letter_mask;
}
static inline int isxdigit(int c){
	return ((unsigned)c-'0' < 10) | (((unsigned)c|32)-'a' < 6);
}
static inline int toascii(int c){
	return c & 0x7f;
}
static inline int tolower(int c){
	return c | ( ((unsigned)c-'A' < 26)<<5 );
}
static inline int toupper(int c){
	return c & ~(((unsigned)c-'a' < 26)<<5);
}

