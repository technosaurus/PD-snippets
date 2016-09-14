//https://www.w3.org/TR/css3-color/#svg-color
#define CSS_COLOR_MAP(OP, ...) \
/*	OP(COLOR_ENUM,                "color-name",         HEX string,HEX value, red, green, blue) */ \
	OP(COLOR_ALICEBLUE,           "aliceblue",           "#F0F8FF", 0xF0F8FF, 240, 248, 255) __VA_ARGS__ \
	OP(COLOR_ANTIQUEWHITE,        "antiquewhite",        "#FAEBD7", 0xFAEBD7, 250, 235, 215) __VA_ARGS__ \
	OP(COLOR_AQUA,                "aqua",                "#00FFFF", 0x00FFFF, 0,   255, 255) __VA_ARGS__ \
	OP(COLOR_AQUAMARINE,          "aquamarine",          "#7FFFD4", 0x7FFFD4, 127, 255, 212) __VA_ARGS__ \
	OP(COLOR_AZURE,               "azure",               "#F0FFFF", 0xF0FFFF, 240, 255, 255) __VA_ARGS__ \
	OP(COLOR_BEIGE,               "beige",               "#F5F5DC", 0xF5F5DC, 245, 245, 220) __VA_ARGS__ \
	OP(COLOR_BISQUE,              "bisque",              "#FFE4C4", 0xFFE4C4, 255, 228, 196) __VA_ARGS__ \
	OP(COLOR_BLACK,               "black",               "#000000", 0x000000, 0,   0,   0  ) __VA_ARGS__ \
	OP(COLOR_BLANCHEDALMOND,      "blanchedalmond",      "#FFEBCD", 0xFFEBCD, 255, 235, 205) __VA_ARGS__ \
	OP(COLOR_BLUE,                "blue",                "#0000FF", 0x0000FF, 0,   0,   255) __VA_ARGS__ \
	OP(COLOR_BLUEVIOLET,          "blueviolet",          "#8A2BE2", 0x8A2BE2, 138, 43,  226) __VA_ARGS__ \
	OP(COLOR_BROWN,               "brown",               "#A52A2A", 0xA52A2A, 165, 42,  42 ) __VA_ARGS__ \
	OP(COLOR_BURLYWOOD,           "burlywood",           "#DEB887", 0xDEB887, 222, 184, 135) __VA_ARGS__ \
	OP(COLOR_CADETBLUE,           "cadetblue",           "#5F9EA0", 0x5F9EA0, 95,  158, 160) __VA_ARGS__ \
	OP(COLOR_CHARTREUSE,          "chartreuse",          "#7FFF00", 0x7FFF00, 127, 255, 0  ) __VA_ARGS__ \
	OP(COLOR_CHOCOLATE,           "chocolate",           "#D2691E", 0xD2691E, 210, 105, 30 ) __VA_ARGS__ \
	OP(COLOR_CORAL,               "coral",               "#FF7F50", 0xFF7F50, 255, 127, 80 ) __VA_ARGS__ \
	OP(COLOR_CORNFLOWERBLUE,      "cornflowerblue",      "#6495ED", 0x6495ED, 100, 149, 237) __VA_ARGS__ \
	OP(COLOR_CORNSILK,            "cornsilk",            "#FFF8DC", 0xFFF8DC, 255, 248, 220) __VA_ARGS__ \
	OP(COLOR_CRIMSON,             "crimson",             "#DC143C", 0xDC143C, 220, 20,  60 ) __VA_ARGS__ \
	OP(COLOR_CYAN,                "cyan",                "#00FFFF", 0x00FFFF, 0,   255, 255) __VA_ARGS__ \
	OP(COLOR_DARKBLUE,            "darkblue",            "#00008B", 0x00008B, 0,   0,   139) __VA_ARGS__ \
	OP(COLOR_DARKCYAN,            "darkcyan",            "#008B8B", 0x008B8B, 0,   139, 139) __VA_ARGS__ \
	OP(COLOR_DARKGOLDENROD,       "darkgoldenrod",       "#B8860B", 0xB8860B, 184, 134, 11 ) __VA_ARGS__ \
	OP(COLOR_DARKGRAY,            "darkgray",            "#A9A9A9", 0xA9A9A9, 169, 169, 169) __VA_ARGS__ \
	OP(COLOR_DARKGREEN,           "darkgreen",           "#006400", 0x006400, 0,   100, 0  ) __VA_ARGS__ \
	OP(COLOR_DARKGREY,            "darkgrey",            "#A9A9A9", 0xA9A9A9, 169, 169, 169) __VA_ARGS__ \
	OP(COLOR_DARKKHAKI,           "darkkhaki",           "#BDB76B", 0xBDB76B, 189, 183, 107) __VA_ARGS__ \
	OP(COLOR_DARKMAGENTA,         "darkmagenta",         "#8B008B", 0x8B008B, 139, 0,   139) __VA_ARGS__ \
	OP(COLOR_DARKOLIVEGREEN,      "darkolivegreen",      "#556B2F", 0x556B2F, 85,  107, 47 ) __VA_ARGS__ \
	OP(COLOR_DARKORANGE,          "darkorange",          "#FF8C00", 0xFF8C00, 255, 140, 0  ) __VA_ARGS__ \
	OP(COLOR_DARKORCHID,          "darkorchid",          "#9932CC", 0x9932CC, 153, 50,  204) __VA_ARGS__ \
	OP(COLOR_DARKRED,             "darkred",             "#8B0000", 0x8B0000, 139, 0,   0  ) __VA_ARGS__ \
	OP(COLOR_DARKSALMON,          "darksalmon",          "#E9967A", 0xE9967A, 233, 150, 122) __VA_ARGS__ \
	OP(COLOR_DARKSEAGREEN,        "darkseagreen",        "#8FBC8F", 0x8FBC8F, 143, 188, 143) __VA_ARGS__ \
	OP(COLOR_DARKSLATEBLUE,       "darkslateblue",       "#483D8B", 0x483D8B, 72,  61,  139) __VA_ARGS__ \
	OP(COLOR_DARKSLATEGRAY,       "darkslategray",       "#2F4F4F", 0x2F4F4F, 47,  79,  79 ) __VA_ARGS__ \
	OP(COLOR_DARKSLATEGREY,       "darkslategrey",       "#2F4F4F", 0x2F4F4F, 47,  79,  79 ) __VA_ARGS__ \
	OP(COLOR_DARKTURQUOISE,       "darkturquoise",       "#00CED1", 0x00CED1, 0,   206, 209) __VA_ARGS__ \
	OP(COLOR_DARKVIOLET,          "darkviolet",          "#9400D3", 0x9400D3, 148, 0,   211) __VA_ARGS__ \
	OP(COLOR_DEEPPINK,            "deeppink",            "#FF1493", 0xFF1493, 255, 20,  147) __VA_ARGS__ \
	OP(COLOR_DEEPSKYBLUE,         "deepskyblue",         "#00BFFF", 0x00BFFF, 0,   191, 255) __VA_ARGS__ \
	OP(COLOR_DIMGRAY,             "dimgray",             "#696969", 0x696969, 105, 105, 105) __VA_ARGS__ \
	OP(COLOR_DIMGREY,             "dimgrey",             "#696969", 0x696969, 105, 105, 105) __VA_ARGS__ \
	OP(COLOR_DODGERBLUE,          "dodgerblue",          "#1E90FF", 0x1E90FF, 30,  144, 255) __VA_ARGS__ \
	OP(COLOR_FIREBRICK,           "firebrick",           "#B22222", 0xB22222, 178, 34,  34 ) __VA_ARGS__ \
	OP(COLOR_FLORALWHITE,         "floralwhite",         "#FFFAF0", 0xFFFAF0, 255, 250, 240) __VA_ARGS__ \
	OP(COLOR_FORESTGREEN,         "forestgreen",         "#228B22", 0x228B22, 34,  139, 34 ) __VA_ARGS__ \
	OP(COLOR_FUCHSIA,             "fuchsia",             "#FF00FF", 0xFF00FF, 255, 0,   255) __VA_ARGS__ \
	OP(COLOR_GAINSBORO,           "gainsboro",           "#DCDCDC", 0xDCDCDC, 220, 220, 220) __VA_ARGS__ \
	OP(COLOR_GHOSTWHITE,          "ghostwhite",          "#F8F8FF", 0xF8F8FF, 248, 248, 255) __VA_ARGS__ \
	OP(COLOR_GOLD,                "gold",                "#FFD700", 0xFFD700, 255, 215, 0  ) __VA_ARGS__ \
	OP(COLOR_GOLDENROD,           "goldenrod",           "#DAA520", 0xDAA520, 218, 165, 32 ) __VA_ARGS__ \
	OP(COLOR_GRAY,                "gray",                "#808080", 0x808080, 128, 128, 128) __VA_ARGS__ \
	OP(COLOR_GREEN,               "green",               "#008000", 0x008000, 0,   128, 0  ) __VA_ARGS__ \
	OP(COLOR_GREENYELLOW,         "greenyellow",         "#ADFF2F", 0xADFF2F, 173, 255, 47 ) __VA_ARGS__ \
	OP(COLOR_GREY,                "grey",                "#808080", 0x808080, 128, 128, 128) __VA_ARGS__ \
	OP(COLOR_HONEYDEW,            "honeydew",            "#F0FFF0", 0xF0FFF0, 240, 255, 240) __VA_ARGS__ \
	OP(COLOR_HOTPINK,             "hotpink",             "#FF69B4", 0xFF69B4, 255, 105, 180) __VA_ARGS__ \
	OP(COLOR_INDIANRED,           "indianred",           "#CD5C5C", 0xCD5C5C, 205, 92,  92 ) __VA_ARGS__ \
	OP(COLOR_INDIGO,              "indigo",              "#4B0082", 0x4B0082, 75,  0,   130) __VA_ARGS__ \
	OP(COLOR_IVORY,               "ivory",               "#FFFFF0", 0xFFFFF0, 255, 255, 240) __VA_ARGS__ \
	OP(COLOR_KHAKI,               "khaki",               "#F0E68C", 0xF0E68C, 240, 230, 140) __VA_ARGS__ \
	OP(COLOR_LAVENDER,            "lavender",            "#E6E6FA", 0xE6E6FA, 230, 230, 250) __VA_ARGS__ \
	OP(COLOR_LAVENDERBLUSH,       "lavenderblush",       "#FFF0F5", 0xFFF0F5, 255, 240, 245) __VA_ARGS__ \
	OP(COLOR_LAWNGREEN,           "lawngreen",           "#7CFC00", 0x7CFC00, 124, 252, 0  ) __VA_ARGS__ \
	OP(COLOR_LEMONCHIFFON,        "lemonchiffon",        "#FFFACD", 0xFFFACD, 255, 250, 205) __VA_ARGS__ \
	OP(COLOR_LIGHTBLUE,           "lightblue",           "#ADD8E6", 0xADD8E6, 173, 216, 230) __VA_ARGS__ \
	OP(COLOR_LIGHTCORAL,          "lightcoral",          "#F08080", 0xF08080, 240, 128, 128) __VA_ARGS__ \
	OP(COLOR_LIGHTCYAN,           "lightcyan",           "#E0FFFF", 0xE0FFFF, 224, 255, 255) __VA_ARGS__ \
	OP(COLOR_LIGHTGOLDENRODYELLOW,"lightgoldenrodyellow","#FAFAD2", 0xFAFAD2, 250, 250, 210) __VA_ARGS__ \
	OP(COLOR_LIGHTGRAY,           "lightgray",           "#D3D3D3", 0xD3D3D3, 211, 211, 211) __VA_ARGS__ \
	OP(COLOR_LIGHTGREEN,          "lightgreen",          "#90EE90", 0x90EE90, 144, 238, 144) __VA_ARGS__ \
	OP(COLOR_LIGHTGREY,           "lightgrey",           "#D3D3D3", 0xD3D3D3, 211, 211, 211) __VA_ARGS__ \
	OP(COLOR_LIGHTPINK,           "lightpink",           "#FFB6C1", 0xFFB6C1, 255, 182, 193) __VA_ARGS__ \
	OP(COLOR_LIGHTSALMON,         "lightsalmon",         "#FFA07A", 0xFFA07A, 255, 160, 122) __VA_ARGS__ \
	OP(COLOR_LIGHTSEAGREEN,       "lightseagreen",       "#20B2AA", 0x20B2AA, 32,  178, 170) __VA_ARGS__ \
	OP(COLOR_LIGHTSKYBLUE,        "lightskyblue",        "#87CEFA", 0x87CEFA, 135, 206, 250) __VA_ARGS__ \
	OP(COLOR_LIGHTSLATEGRAY,      "lightslategray",      "#778899", 0x778899, 119, 136, 153) __VA_ARGS__ \
	OP(COLOR_LIGHTSLATEGREY,      "lightslategrey",      "#778899", 0x778899, 119, 136, 153) __VA_ARGS__ \
	OP(COLOR_LIGHTSTEELBLUE,      "lightsteelblue",      "#B0C4DE", 0xB0C4DE, 176, 196, 222) __VA_ARGS__ \
	OP(COLOR_LIGHTYELLOW,         "lightyellow",         "#FFFFE0", 0xFFFFE0, 255, 255, 224) __VA_ARGS__ \
	OP(COLOR_LIME,                "lime",                "#00FF00", 0x00FF00, 0  , 255, 0  ) __VA_ARGS__ \
	OP(COLOR_LIMEGREEN,           "limegreen",           "#32CD32", 0x32CD32, 50 , 205, 50 ) __VA_ARGS__ \
	OP(COLOR_LINEN,               "linen",               "#FAF0E6", 0xFAF0E6, 250, 240, 230) __VA_ARGS__ \
	OP(COLOR_MAGENTA,             "magenta",             "#FF00FF", 0xFF00FF, 255, 0,   255) __VA_ARGS__ \
	OP(COLOR_MAROON,              "maroon",              "#800000", 0x800000, 128, 0,   0  ) __VA_ARGS__ \
	OP(COLOR_MEDIUMAQUAMARINE,    "mediumaquamarine",    "#66CDAA", 0x66CDAA, 102, 205, 170) __VA_ARGS__ \
	OP(COLOR_MEDIUMBLUE,          "mediumblue",          "#0000CD", 0x0000CD, 0,   0,   205) __VA_ARGS__ \
	OP(COLOR_MEDIUMORCHID,        "mediumorchid",        "#BA55D3", 0xBA55D3, 186, 85,  211) __VA_ARGS__ \
	OP(COLOR_MEDIUMPURPLE,        "mediumpurple",        "#9370DB", 0x9370DB, 147, 112, 219) __VA_ARGS__ \
	OP(COLOR_MEDIUMSEAGREEN,      "mediumseagreen",      "#3CB371", 0x3CB371, 60,  179, 113) __VA_ARGS__ \
	OP(COLOR_MEDIUMSLATEBLUE,     "mediumslateblue",     "#7B68EE", 0x7B68EE, 123, 104, 238) __VA_ARGS__ \
	OP(COLOR_MEDIUMSPRINGGREEN,   "mediumspringgreen",   "#00FA9A", 0x00FA9A, 0,   250, 154) __VA_ARGS__ \
	OP(COLOR_MEDIUMTURQUOISE,     "mediumturquoise",     "#48D1CC", 0x48D1CC, 72,  209, 204) __VA_ARGS__ \
	OP(COLOR_MEDIUMVIOLETRED,     "mediumvioletred",     "#C71585", 0xC71585, 199, 21,  133) __VA_ARGS__ \
	OP(COLOR_MIDNIGHTBLUE,        "midnightblue",        "#191970", 0x191970, 25,  25,  112) __VA_ARGS__ \
	OP(COLOR_MINTCREAM,           "mintcream",           "#F5FFFA", 0xF5FFFA, 245, 255, 250) __VA_ARGS__ \
	OP(COLOR_MISTYROSE,           "mistyrose",           "#FFE4E1", 0xFFE4E1, 255, 228, 225) __VA_ARGS__ \
	OP(COLOR_MOCCASIN,            "moccasin",            "#FFE4B5", 0xFFE4B5, 255, 228, 181) __VA_ARGS__ \
	OP(COLOR_NAVAJOWHITE,         "navajowhite",         "#FFDEAD", 0xFFDEAD, 255, 222, 173) __VA_ARGS__ \
	OP(COLOR_NAVY,                "navy",                "#000080", 0x000080, 0,   0,   128) __VA_ARGS__ \
	OP(COLOR_OLDLACE,             "oldlace",             "#FDF5E6", 0xFDF5E6, 253, 245, 230) __VA_ARGS__ \
	OP(COLOR_OLIVE,               "olive",               "#808000", 0x808000, 128, 128, 0  ) __VA_ARGS__ \
	OP(COLOR_OLIVEDRAB,           "olivedrab",           "#6B8E23", 0x6B8E23, 107, 142, 35 ) __VA_ARGS__ \
	OP(COLOR_ORANGE,              "orange",              "#FFA500", 0xFFA500, 255, 165, 0  ) __VA_ARGS__ \
	OP(COLOR_ORANGERED,           "orangered",           "#FF4500", 0xFF4500, 255, 69,  0  ) __VA_ARGS__ \
	OP(COLOR_ORCHID,              "orchid",              "#DA70D6", 0xDA70D6, 218, 112, 214) __VA_ARGS__ \
	OP(COLOR_PALEGOLDENROD,       "palegoldenrod",       "#EEE8AA", 0xEEE8AA, 238, 232, 170) __VA_ARGS__ \
	OP(COLOR_PALEGREEN,           "palegreen",           "#98FB98", 0x98FB98, 152, 251, 152) __VA_ARGS__ \
	OP(COLOR_PALETURQUOISE,       "paleturquoise",       "#AFEEEE", 0xAFEEEE, 175, 238, 238) __VA_ARGS__ \
	OP(COLOR_PALEVIOLETRED,       "palevioletred",       "#DB7093", 0xDB7093, 219, 112, 147) __VA_ARGS__ \
	OP(COLOR_PAPAYAWHIP,          "papayawhip",          "#FFEFD5", 0xFFEFD5, 255, 239, 213) __VA_ARGS__ \
	OP(COLOR_PEACHPUFF,           "peachpuff",           "#FFDAB9", 0xFFDAB9, 255, 218, 185) __VA_ARGS__ \
	OP(COLOR_PERU,                "peru",                "#CD853F", 0xCD853F, 205, 133, 63 ) __VA_ARGS__ \
	OP(COLOR_PINK,                "pink",                "#FFC0CB", 0xFFC0CB, 255, 192, 203) __VA_ARGS__ \
	OP(COLOR_PLUM,                "plum",                "#DDA0DD", 0xDDA0DD, 221, 160, 221) __VA_ARGS__ \
	OP(COLOR_POWDERBLUE,          "powderblue",          "#B0E0E6", 0xB0E0E6, 176, 224, 230) __VA_ARGS__ \
	OP(COLOR_PURPLE,              "purple",              "#800080", 0x800080, 128, 0,   128) __VA_ARGS__ \
	OP(COLOR_RED,                 "red",                 "#FF0000", 0xFF0000, 255, 0,   0  ) __VA_ARGS__ \
	OP(COLOR_ROSYBROWN,           "rosybrown",           "#BC8F8F", 0xBC8F8F, 188, 143, 143) __VA_ARGS__ \
	OP(COLOR_ROYALBLUE,           "royalblue",           "#4169E1", 0x4169E1, 65,  105, 225) __VA_ARGS__ \
	OP(COLOR_SADDLEBROWN,         "saddlebrown",         "#8B4513", 0x8B4513, 139, 69,  19 ) __VA_ARGS__ \
	OP(COLOR_SALMON,              "salmon",              "#FA8072", 0xFA8072, 250, 128, 114) __VA_ARGS__ \
	OP(COLOR_SANDYBROWN,          "sandybrown",          "#F4A460", 0xF4A460, 244, 164, 96 ) __VA_ARGS__ \
	OP(COLOR_SEAGREEN,            "seagreen",            "#2E8B57", 0x2E8B57, 46,  139, 87 ) __VA_ARGS__ \
	OP(COLOR_SEASHELL,            "seashell",            "#FFF5EE", 0xFFF5EE, 255, 245, 238) __VA_ARGS__ \
	OP(COLOR_SIENNA,              "sienna",              "#A0522D", 0xA0522D, 160, 82,  45 ) __VA_ARGS__ \
	OP(COLOR_SILVER,              "silver",              "#C0C0C0", 0xC0C0C0, 192, 192, 192) __VA_ARGS__ \
	OP(COLOR_SKYBLUE,             "skyblue",             "#87CEEB", 0x87CEEB, 135, 206, 235) __VA_ARGS__ \
	OP(COLOR_SLATEBLUE,           "slateblue",           "#6A5ACD", 0x6A5ACD, 106, 90,  205) __VA_ARGS__ \
	OP(COLOR_SLATEGRAY,           "slategray",           "#708090", 0x708090, 112, 128, 144) __VA_ARGS__ \
	OP(COLOR_SLATEGREY,           "slategrey",           "#708090", 0x708090, 112, 128, 144) __VA_ARGS__ \
	OP(COLOR_SNOW,                "snow",                "#FFFAFA", 0xFFFAFA, 255, 250, 250) __VA_ARGS__ \
	OP(COLOR_SPRINGGREEN,         "springgreen",         "#00FF7F", 0x00FF7F, 0,   255, 127) __VA_ARGS__ \
	OP(COLOR_STEELBLUE,           "steelblue",           "#4682B4", 0x4682B4, 70,  130, 180) __VA_ARGS__ \
	OP(COLOR_TAN,                 "tan",                 "#D2B48C", 0xD2B48C, 210, 180, 140) __VA_ARGS__ \
	OP(COLOR_TEAL,                "teal",                "#008080", 0x008080, 0,   128, 128) __VA_ARGS__ \
	OP(COLOR_THISTLE,             "thistle",             "#D8BFD8", 0xD8BFD8, 216, 191, 216) __VA_ARGS__ \
	OP(COLOR_TOMATO,              "tomato",              "#FF6347", 0xFF6347, 255, 99,  71 ) __VA_ARGS__ \
	OP(COLOR_TURQUOISE,           "turquoise",           "#40E0D0", 0x40E0D0, 64,  224, 208) __VA_ARGS__ \
	OP(COLOR_VIOLET,              "violet",              "#EE82EE", 0xEE82EE, 238, 130, 238) __VA_ARGS__ \
	OP(COLOR_WHEAT,               "wheat",               "#F5DEB3", 0xF5DEB3, 245, 222, 179) __VA_ARGS__ \
	OP(COLOR_WHITE,               "white",               "#FFFFFF", 0xFFFFFF, 255, 255, 255) __VA_ARGS__ \
	OP(COLOR_WHITESMOKE,          "whitesmoke",          "#F5F5F5", 0xF5F5F5, 245, 245, 245) __VA_ARGS__ \
	OP(COLOR_YELLOW,              "yellow",              "#FFFF00", 0xFFFF00, 255, 255, 0  ) __VA_ARGS__ \
	OP(COLOR_YELLOWGREEN,         "yellowgreen",         "#9ACD32", 0x9ACD32, 154, 205, 50 ) __VA_ARGS__
