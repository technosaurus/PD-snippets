/* config-helpers.h - conditional compilation with boolean logic support
 * Copyright (C) 2016, Bradley Conroy
 * This software is released to the public domain and may also be used in
 * accordance with any license acknowledged by the Open Source Initiative.
 * These licenses may be found at https://opensource.org/licenses
 */
#define PASTE_(x,y)           x##y
#define PASTE(x,y)            PASTE_(x,y)
#define Enabled(...)          __VA_ARGS__
#define Disabled(...)
#define NOT(x)                PASTE(NOT_,x)
#define NOT_Disabled          Enabled
#define NOT_Enabled           Disabled
#define OR(x,y)               PASTE(x,PASTE(_OR_,y))
#define Disabled_OR_Disabled  Disabled
#define Disabled_OR_Enabled   Enabled
#define Enabled_OR_Disabled   Enabled
#define Enabled_OR_Enabled    Enabled
#define AND(x,y)              PASTE(x,PASTE(_AND_,y))
#define Disabled_AND_Disabled Disabled
#define Disabled_AND_Enabled  Disabled
#define Enabled_AND_Disabled  Disabled
#define Enabled_AND_Enabled   Enabled
#define XOR(x,y)              PASTE(x,PASTE(_XOR_,y))
#define Disabled_XOR_Disabled Disabled
#define Disabled_XOR_Enabled  Enabled
#define Enabled_XOR_Disabled  Enabled
#define Enabled_XOR_Enabled   Disabled
#define NOR(x,y)              PASTE(NOT_, OR(x,y))
#define NAND(x,y)             PASTE(NOT_, AND(x,y))
#define XNOR(x,y)             PASTE(NOT_, XOR(x,y))
//HACK: #error requires its own line and _Pragma support is sketchy
#define ERROR(x) char PASTE(PASTE(ERROR_on_line__,__LINE__),PASTE(__XXX_,x))[-1];
/**
 Macros for inline conditional compilation with boolean logic support
 
 Usage - config file:
 #define FOO Enabled
 #define BAR Disabled
 #define BAZ Enabled

 Usage - conditional code

//replace an #ifdef in 1 line
FOO( printf("Foo enabled\n"); )

// multiline version - Note the ( ... )  
FOO(
  printf("It can span multiple lines too, if desired,\n");
  printf("but uses (...) instead of {...} \n");
)

//multiple #ifdefs in 1 line
puts("Enabled items: " FOO("foo, ") BAR("bar, ") BAZ("baz, ") "etc...");
//replaces this nearly unreadable 10+ line monstrosity:
puts("Enabled items: "
#ifdef FOO
  "foo, "
#endif
#ifdef BAR
  "bar, "
#endif
#ifdef BAZ
  "baz, "
#endif
  "etc..."
);

//boolean logic alternative to nested #ifdefs
XOR(BAR,BAZ)(printf("One of bar or baz is enabled\n");)
//replaces this:
#if defined(BAR) && !defined(BAZ) || defined(BAZ) && !defined(BAR)
  printf("One of bar or baz is enabled\n");
#endif

//compound boolean logic #ifdefs
OR(FOO,AND(BAR,BAZ))(
  FOO(init_foo();)
  NOT(FOO)(
    init_bar();
    init_baz();
  )
)

//inline replacement for #error
XOR(FOO,BAR)(ERROR(Only_1_of_foo_or_bar_may_be_enabled);)

**/

