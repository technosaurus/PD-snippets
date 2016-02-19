//https://www.w3.org/TR/css3-color/#svg-color
//MAP(COLOR_ENUM,                "color-name",          "#FAFAD2", 0xFAFAD2, red, green, blue),
#define CSS_COLOR_MAP { \
	MAP(COLOR_ALICEBLUE,           "aliceblue",           "#F0F8FF", 0xF0F8FF, 240, 248, 255), \
	MAP(COLOR_ANTIQUEWHITE,        "antiquewhite",        "#FAEBD7", 0xFAEBD7, 250, 235, 215), \
	MAP(COLOR_AQUA,                "aqua",                "#00FFFF", 0x00FFFF, 0,   255, 255), \
	MAP(COLOR_AQUAMARINE,          "aquamarine",          "#7FFFD4", 0x7FFFD4, 127, 255, 212), \
	MAP(COLOR_AZURE,               "azure",               "#F0FFFF", 0xF0FFFF, 240, 255, 255), \
	MAP(COLOR_BEIGE,               "beige",               "#F5F5DC", 0xF5F5DC, 245, 245, 220), \
	MAP(COLOR_BISQUE,              "bisque",              "#FFE4C4", 0xFFE4C4, 255, 228, 196), \
	MAP(COLOR_BLACK,               "black",               "#000000", 0x000000, 0,   0,   0  ), \
	MAP(COLOR_BLANCHEDALMOND,      "blanchedalmond",      "#FFEBCD", 0xFFEBCD, 255, 235, 205), \
	MAP(COLOR_BLUE,                "blue",                "#0000FF", 0x0000FF, 0,   0,   255), \
	MAP(COLOR_BLUEVIOLET,          "blueviolet",          "#8A2BE2", 0x8A2BE2, 138, 43,  226), \
	MAP(COLOR_BROWN,               "brown",               "#A52A2A", 0xA52A2A, 165, 42,  42 ), \
	MAP(COLOR_BURLYWOOD,           "burlywood",           "#DEB887", 0xDEB887, 222, 184, 135), \
	MAP(COLOR_CADETBLUE,           "cadetblue",           "#5F9EA0", 0x5F9EA0, 95,  158, 160), \
	MAP(COLOR_CHARTREUSE,          "chartreuse",          "#7FFF00", 0x7FFF00, 127, 255, 0  ), \
	MAP(COLOR_CHOCOLATE,           "chocolate",           "#D2691E", 0xD2691E, 210, 105, 30 ), \
	MAP(COLOR_CORAL,               "coral",               "#FF7F50", 0xFF7F50, 255, 127, 80 ), \
	MAP(COLOR_CORNFLOWERBLUE,      "cornflowerblue",      "#6495ED", 0x6495ED, 100, 149, 237), \
	MAP(COLOR_CORNSILK,            "cornsilk",            "#FFF8DC", 0xFFF8DC, 255, 248, 220), \
	MAP(COLOR_CRIMSON,             "crimson",             "#DC143C", 0xDC143C, 220, 20,  60 ), \
	MAP(COLOR_CYAN,                "cyan",                "#00FFFF", 0x00FFFF, 0,   255, 255), \
	MAP(COLOR_DARKBLUE,            "darkblue",            "#00008B", 0x00008B, 0,   0,   139), \
	MAP(COLOR_DARKCYAN,            "darkcyan",            "#008B8B", 0x008B8B, 0,   139, 139), \
	MAP(COLOR_DARKGOLDENROD,       "darkgoldenrod",       "#B8860B", 0xB8860B, 184, 134, 11 ), \
	MAP(COLOR_DARKGRAY,            "darkgray",            "#A9A9A9", 0xA9A9A9, 169, 169, 169), \
	MAP(COLOR_DARKGREEN,           "darkgreen",           "#006400", 0x006400, 0,   100, 0  ), \
	MAP(COLOR_DARKGREY,            "darkgrey",            "#A9A9A9", 0xA9A9A9, 169, 169, 169), \
	MAP(COLOR_DARKKHAKI,           "darkkhaki",           "#BDB76B", 0xBDB76B, 189, 183, 107), \
	MAP(COLOR_DARKMAGENTA,         "darkmagenta",         "#8B008B", 0x8B008B, 139, 0,   139), \
	MAP(COLOR_DARKOLIVEGREEN,      "darkolivegreen",      "#556B2F", 0x556B2F, 85,  107, 47 ), \
	MAP(COLOR_DARKORANGE,          "darkorange",          "#FF8C00", 0xFF8C00, 255, 140, 0  ), \
	MAP(COLOR_DARKORCHID,          "darkorchid",          "#9932CC", 0x9932CC, 153, 50,  204), \
	MAP(COLOR_DARKRED,             "darkred",             "#8B0000", 0x8B0000, 139, 0,   0  ), \
	MAP(COLOR_DARKSALMON,          "darksalmon",          "#E9967A", 0xE9967A, 233, 150, 122), \
	MAP(COLOR_DARKSEAGREEN,        "darkseagreen",        "#8FBC8F", 0x8FBC8F, 143, 188, 143), \
	MAP(COLOR_DARKSLATEBLUE,       "darkslateblue",       "#483D8B", 0x483D8B, 72,  61,  139), \
	MAP(COLOR_DARKSLATEGRAY,       "darkslategray",       "#2F4F4F", 0x2F4F4F, 47,  79,  79 ), \
	MAP(COLOR_DARKSLATEGREY,       "darkslategrey",       "#2F4F4F", 0x2F4F4F, 47,  79,  79 ), \
	MAP(COLOR_DARKTURQUOISE,       "darkturquoise",       "#00CED1", 0x00CED1, 0,   206, 209), \
	MAP(COLOR_DARKVIOLET,          "darkviolet",          "#9400D3", 0x9400D3, 148, 0,   211), \
	MAP(COLOR_DEEPPINK,            "deeppink",            "#FF1493", 0xFF1493, 255, 20,  147), \
	MAP(COLOR_DEEPSKYBLUE,         "deepskyblue",         "#00BFFF", 0x00BFFF, 0,   191, 255), \
	MAP(COLOR_DIMGRAY,             "dimgray",             "#696969", 0x696969, 105, 105, 105), \
	MAP(COLOR_DIMGREY,             "dimgrey",             "#696969", 0x696969, 105, 105, 105), \
	MAP(COLOR_DODGERBLUE,          "dodgerblue",          "#1E90FF", 0x1E90FF, 30,  144, 255), \
	MAP(COLOR_FIREBRICK,           "firebrick",           "#B22222", 0xB22222, 178, 34,  34 ), \
	MAP(COLOR_FLORALWHITE,         "floralwhite",         "#FFFAF0", 0xFFFAF0, 255, 250, 240), \
	MAP(COLOR_FORESTGREEN,         "forestgreen",         "#228B22", 0x228B22, 34,  139, 34 ), \
	MAP(COLOR_FUCHSIA,             "fuchsia",             "#FF00FF", 0xFF00FF, 255, 0,   255), \
	MAP(COLOR_GAINSBORO,           "gainsboro",           "#DCDCDC", 0xDCDCDC, 220, 220, 220), \
	MAP(COLOR_GHOSTWHITE,          "ghostwhite",          "#F8F8FF", 0xF8F8FF, 248, 248, 255), \
	MAP(COLOR_GOLD,                "gold",                "#FFD700", 0xFFD700, 255, 215, 0  ), \
	MAP(COLOR_GOLDENROD,           "goldenrod",           "#DAA520", 0xDAA520, 218, 165, 32 ), \
	MAP(COLOR_GRAY,                "gray",                "#808080", 0x808080, 128, 128, 128), \
	MAP(COLOR_GREEN,               "green",               "#008000", 0x008000, 0,   128, 0  ), \
	MAP(COLOR_GREENYELLOW,         "greenyellow",         "#ADFF2F", 0xADFF2F, 173, 255, 47 ), \
	MAP(COLOR_GREY,                "grey",                "#808080", 0x808080, 128, 128, 128), \
	MAP(COLOR_HONEYDEW,            "honeydew",            "#F0FFF0", 0xF0FFF0, 240, 255, 240), \
	MAP(COLOR_HOTPINK,             "hotpink",             "#FF69B4", 0xFF69B4, 255, 105, 180), \
	MAP(COLOR_INDIANRED,           "indianred",           "#CD5C5C", 0xCD5C5C, 205, 92,  92 ), \
	MAP(COLOR_INDIGO,              "indigo",              "#4B0082", 0x4B0082, 75,  0,   130), \
	MAP(COLOR_IVORY,               "ivory",               "#FFFFF0", 0xFFFFF0, 255, 255, 240), \
	MAP(COLOR_KHAKI,               "khaki",               "#F0E68C", 0xF0E68C, 240, 230, 140), \
	MAP(COLOR_LAVENDER,            "lavender",            "#E6E6FA", 0xE6E6FA, 230, 230, 250), \
	MAP(COLOR_LAVENDERBLUSH,       "lavenderblush",       "#FFF0F5", 0xFFF0F5, 255, 240, 245), \
	MAP(COLOR_LAWNGREEN,           "lawngreen",           "#7CFC00", 0x7CFC00, 124, 252, 0  ), \
	MAP(COLOR_LEMONCHIFFON,        "lemonchiffon",        "#FFFACD", 0xFFFACD, 255, 250, 205), \
	MAP(COLOR_LIGHTBLUE,           "lightblue",           "#ADD8E6", 0xADD8E6, 173, 216, 230), \
	MAP(COLOR_LIGHTCORAL,          "lightcoral",          "#F08080", 0xF08080, 240, 128, 128), \
	MAP(COLOR_LIGHTCYAN,           "lightcyan",           "#E0FFFF", 0xE0FFFF, 224, 255, 255), \
	MAP(COLOR_LIGHTGOLDENRODYELLOW,"lightgoldenrodyellow","#FAFAD2", 0xFAFAD2, 250, 250, 210), \
	MAP(COLOR_LIGHTGRAY,           "lightgray",           "#D3D3D3", 0xD3D3D3, 211, 211, 211), \
	MAP(COLOR_LIGHTGREEN,          "lightgreen",          "#90EE90", 0x90EE90, 144, 238, 144), \
	MAP(COLOR_LIGHTGREY,           "lightgrey",           "#D3D3D3", 0xD3D3D3, 211, 211, 211), \
	MAP(COLOR_LIGHTPINK,           "lightpink",           "#FFB6C1", 0xFFB6C1, 255, 182, 193), \
	MAP(COLOR_LIGHTSALMON,         "lightsalmon",         "#FFA07A", 0xFFA07A, 255, 160, 122), \
	MAP(COLOR_LIGHTSEAGREEN,       "lightseagreen",       "#20B2AA", 0x20B2AA, 32,  178, 170), \
	MAP(COLOR_LIGHTSKYBLUE,        "lightskyblue",        "#87CEFA", 0x87CEFA, 135, 206, 250), \
	MAP(COLOR_LIGHTSLATEGRAY,      "lightslategray",      "#778899", 0x778899, 119, 136, 153), \
	MAP(COLOR_LIGHTSLATEGREY,      "lightslategrey",      "#778899", 0x778899, 119, 136, 153), \
	MAP(COLOR_LIGHTSTEELBLUE,      "lightsteelblue",      "#B0C4DE", 0xB0C4DE, 176, 196, 222), \
	MAP(COLOR_LIGHTYELLOW,         "lightyellow",         "#FFFFE0", 0xFFFFE0, 255, 255, 224), \
	MAP(COLOR_LIME,                "lime",                "#00FF00", 0x00FF00, 0  , 255, 0  ), \
	MAP(COLOR_LIMEGREEN,           "limegreen",           "#32CD32", 0x32CD32, 50 , 205, 50 ), \
	MAP(COLOR_LINEN,               "linen",               "#FAF0E6", 0xFAF0E6, 250, 240, 230), \
	MAP(COLOR_MAGENTA,             "magenta",             "#FF00FF", 0xFF00FF, 255, 0,   255), \
	MAP(COLOR_MAROON,              "maroon",              "#800000", 0x800000, 128, 0,   0  ), \
	MAP(COLOR_MEDIUMAQUAMARINE,    "mediumaquamarine",    "#66CDAA", 0x66CDAA, 102, 205, 170), \
	MAP(COLOR_MEDIUMBLUE,          "mediumblue",          "#0000CD", 0x0000CD, 0,   0,   205), \
	MAP(COLOR_MEDIUMORCHID,        "mediumorchid",        "#BA55D3", 0xBA55D3, 186, 85,  211), \
	MAP(COLOR_MEDIUMPURPLE,        "mediumpurple",        "#9370DB", 0x9370DB, 147, 112, 219), \
	MAP(COLOR_MEDIUMSEAGREEN,      "mediumseagreen",      "#3CB371", 0x3CB371, 60,  179, 113), \
	MAP(COLOR_MEDIUMSLATEBLUE,     "mediumslateblue",     "#7B68EE", 0x7B68EE, 123, 104, 238), \
	MAP(COLOR_MEDIUMSPRINGGREEN,   "mediumspringgreen",   "#00FA9A", 0x00FA9A, 0,   250, 154), \
	MAP(COLOR_MEDIUMTURQUOISE,     "mediumturquoise",     "#48D1CC", 0x48D1CC, 72,  209, 204), \
	MAP(COLOR_MEDIUMVIOLETRED,     "mediumvioletred",     "#C71585", 0xC71585, 199, 21,  133), \
	MAP(COLOR_MIDNIGHTBLUE,        "midnightblue",        "#191970", 0x191970, 25,  25,  112), \
	MAP(COLOR_MINTCREAM,           "mintcream",           "#F5FFFA", 0xF5FFFA, 245, 255, 250), \
	MAP(COLOR_MISTYROSE,           "mistyrose",           "#FFE4E1", 0xFFE4E1, 255, 228, 225), \
	MAP(COLOR_MOCCASIN,            "moccasin",            "#FFE4B5", 0xFFE4B5, 255, 228, 181), \
	MAP(COLOR_NAVAJOWHITE,         "navajowhite",         "#FFDEAD", 0xFFDEAD, 255, 222, 173), \
	MAP(COLOR_NAVY,                "navy",                "#000080", 0x000080, 0,   0,   128), \
	MAP(COLOR_OLDLACE,             "oldlace",             "#FDF5E6", 0xFDF5E6, 253, 245, 230), \
	MAP(COLOR_OLIVE,               "olive",               "#808000", 0x808000, 128, 128, 0  ), \
	MAP(COLOR_OLIVEDRAB,           "olivedrab",           "#6B8E23", 0x6B8E23, 107, 142, 35 ), \
	MAP(COLOR_ORANGE,              "orange",              "#FFA500", 0xFFA500, 255, 165, 0  ), \
	MAP(COLOR_ORANGERED,           "orangered",           "#FF4500", 0xFF4500, 255, 69,  0  ), \
	MAP(COLOR_ORCHID,              "orchid",              "#DA70D6", 0xDA70D6, 218, 112, 214), \
	MAP(COLOR_PALEGOLDENROD,       "palegoldenrod",       "#EEE8AA", 0xEEE8AA, 238, 232, 170), \
	MAP(COLOR_PALEGREEN,           "palegreen",           "#98FB98", 0x98FB98, 152, 251, 152), \
	MAP(COLOR_PALETURQUOISE,       "paleturquoise",       "#AFEEEE", 0xAFEEEE, 175, 238, 238), \
	MAP(COLOR_PALEVIOLETRED,       "palevioletred",       "#DB7093", 0xDB7093, 219, 112, 147), \
	MAP(COLOR_PAPAYAWHIP,          "papayawhip",          "#FFEFD5", 0xFFEFD5, 255, 239, 213), \
	MAP(COLOR_PEACHPUFF,           "peachpuff",           "#FFDAB9", 0xFFDAB9, 255, 218, 185), \
	MAP(COLOR_PERU,                "peru",                "#CD853F", 0xCD853F, 205, 133, 63 ), \
	MAP(COLOR_PINK,                "pink",                "#FFC0CB", 0xFFC0CB, 255, 192, 203), \
	MAP(COLOR_PLUM,                "plum",                "#DDA0DD", 0xDDA0DD, 221, 160, 221), \
	MAP(COLOR_POWDERBLUE,          "powderblue",          "#B0E0E6", 0xB0E0E6, 176, 224, 230), \
	MAP(COLOR_PURPLE,              "purple",              "#800080", 0x800080, 128, 0,   128), \
	MAP(COLOR_RED,                 "red",                 "#FF0000", 0xFF0000, 255, 0,   0  ), \
	MAP(COLOR_ROSYBROWN,           "rosybrown",           "#BC8F8F", 0xBC8F8F, 188, 143, 143), \
	MAP(COLOR_ROYALBLUE,           "royalblue",           "#4169E1", 0x4169E1, 65,  105, 225), \
	MAP(COLOR_SADDLEBROWN,         "saddlebrown",         "#8B4513", 0x8B4513, 139, 69,  19 ), \
	MAP(COLOR_SALMON,              "salmon",              "#FA8072", 0xFA8072, 250, 128, 114), \
	MAP(COLOR_SANDYBROWN,          "sandybrown",          "#F4A460", 0xF4A460, 244, 164, 96 ), \
	MAP(COLOR_SEAGREEN,            "seagreen",            "#2E8B57", 0x2E8B57, 46,  139, 87 ), \
	MAP(COLOR_SEASHELL,            "seashell",            "#FFF5EE", 0xFFF5EE, 255, 245, 238), \
	MAP(COLOR_SIENNA,              "sienna",              "#A0522D", 0xA0522D, 160, 82,  45 ), \
	MAP(COLOR_SILVER,              "silver",              "#C0C0C0", 0xC0C0C0, 192, 192, 192), \
	MAP(COLOR_SKYBLUE,             "skyblue",             "#87CEEB", 0x87CEEB, 135, 206, 235), \
	MAP(COLOR_SLATEBLUE,           "slateblue",           "#6A5ACD", 0x6A5ACD, 106, 90,  205), \
	MAP(COLOR_SLATEGRAY,           "slategray",           "#708090", 0x708090, 112, 128, 144), \
	MAP(COLOR_SLATEGREY,           "slategrey",           "#708090", 0x708090, 112, 128, 144), \
	MAP(COLOR_SNOW,                "snow",                "#FFFAFA", 0xFFFAFA, 255, 250, 250), \
	MAP(COLOR_SPRINGGREEN,         "springgreen",         "#00FF7F", 0x00FF7F, 0,   255, 127), \
	MAP(COLOR_STEELBLUE,           "steelblue",           "#4682B4", 0x4682B4, 70,  130, 180), \
	MAP(COLOR_TAN,                 "tan",                 "#D2B48C", 0xD2B48C, 210, 180, 140), \
	MAP(COLOR_TEAL,                "teal",                "#008080", 0x008080, 0,   128, 128), \
	MAP(COLOR_THISTLE,             "thistle",             "#D8BFD8", 0xD8BFD8, 216, 191, 216), \
	MAP(COLOR_TOMATO,              "tomato",              "#FF6347", 0xFF6347, 255, 99,  71 ), \
	MAP(COLOR_TURQUOISE,           "turquoise",           "#40E0D0", 0x40E0D0, 64,  224, 208), \
	MAP(COLOR_VIOLET,              "violet",              "#EE82EE", 0xEE82EE, 238, 130, 238), \
	MAP(COLOR_WHEAT,               "wheat",               "#F5DEB3", 0xF5DEB3, 245, 222, 179), \
	MAP(COLOR_WHITE,               "white",               "#FFFFFF", 0xFFFFFF, 255, 255, 255), \
	MAP(COLOR_WHITESMOKE,          "whitesmoke",          "#F5F5F5", 0xF5F5F5, 245, 245, 245), \
	MAP(COLOR_YELLOW,              "yellow",              "#FFFF00", 0xFFFF00, 255, 255, 0  ), \
	MAP(COLOR_YELLOWGREEN,         "yellowgreen",         "#9ACD32", 0x9ACD32, 154, 205, 50 ), \
}
