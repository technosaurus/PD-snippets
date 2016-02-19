#define HTML_TAGMAP { \
	  /*      ENUM       TAG          ... DEFAULT CSS PROPERTIES ... */ \
	MAP(TAG_ADDRESS,   "address",   "display:block","unicode-bidi:embed"), \
	MAP(TAG_B,         "b",         "font-weight:bolder"), \
	MAP(TAG_BIG,       "big",       "font-size:1.17em"), \
	MAP(TAG_BLOCKQUOTE,"blockquote","display:block","margin:1.12em 0","margin-left:40px","margin-right:40px","unicode-bidi:embed"), \
	MAP(TAG_BODY ,     "body",      "display:block","margin:8px","unicode-bidi:embed"), \
	MAP(TAG_BUTTON,    "button",    "display:inline-block"), \
	MAP(TAG_CAPTION,   "caption",   "display:table-caption"), \
	MAP(TAG_CENTER,    "center",    "display:block","text-align:center","unicode-bidi:embed"), \
	MAP(TAG_CITE,      "cite",      "font-style: italic"), \
	MAP(TAG_CODE,      "code",      "font-family:monospace"), \
	MAP(TAG_COL,       "col",       "display:table-column"), \
	MAP(TAG_COLGROUP,  "colgroup",  "display:table-column-group"), \
	MAP(TAG_DD ,       "dd",        "display:block","margin-left:40px","unicode-bidi:embed"), \
	MAP(TAG_DEL,       "del",       "text-decoration:line-through"), \
	MAP(TAG_DIR,       "dir",       "display:block","margin:1.12em 0","unicode-bidi:embed"), \
	MAP(TAG_DIV,       "div",       "display:block","unicode-bidi:embed"), \
	MAP(TAG_DL ,       "dl",        "display:block","margin:1.12em 0","margin:1.12em 0","unicode-bidi:embed"), \
	MAP(TAG_DT ,       "dt",        "display:block","unicode-bidi:embed"), \
	MAP(TAG_EM,        "em",        "font-style: italic"), \
	MAP(TAG_FIELDSET,  "fieldset",  "display:block","margin:1.12em 0","unicode-bidi:embed"), \
	MAP(TAG_FORM ,     "form",      "display:block","margin:1.12em 0","unicode-bidi:embed"), \
	MAP(TAG_FRAME ,    "frame",     "display:block","unicode-bidi:embed"), \
	MAP(TAG_FRAMESET,  "frameset",  "display:block","unicode-bidi:embed"), \
	MAP(TAG_H1 ,       "h1",        "display:block","font-size:2em","font-weight:bolder","margin:.67em 0","unicode-bidi:embed"), \
	MAP(TAG_H2 ,       "h2",        "display:block","font-size:1.5em","font-weight:bolder","margin:.75em 0","unicode-bidi:embed"), \
	MAP(TAG_H3 ,       "h3",        "display:block","font-size: 1.17em","font-weight:bolder","margin:.83em 0","unicode-bidi:embed"), \
	MAP(TAG_H4 ,       "h4",        "display:block","margin:1.12em 0","font-weight:bolder","unicode-bidi:embed"), \
	MAP(TAG_H5 ,       "h5",        "display:block","font-size:.83em","font-weight:bolder","margin:1.5em 0","unicode-bidi:embed"), \
	MAP(TAG_H6 ,       "h6",        "display:block","font-size: .75em","font-weight:bolder","margin: 1.67em 0","unicode-bidi:embed"), \
	MAP(TAG_HEAD ,     "head",      "display:none"), \
	MAP(TAG_HR,        "hr",        "border:1px inset","display:block","unicode-bidi:embed"), \
	MAP(TAG_HTML ,     "html",      "display:block","unicode-bidi:embed"), \
	MAP(TAG_I,         "i",         "font-style: italic"), \
	MAP(TAG_INPUT,     "input",     "display:inline-block"), \
	MAP(TAG_INS,       "ins",       "text-decoration:underline"), \
	MAP(TAG_KBD,       "kbd",       "font-family:monospace"), \
	MAP(TAG_LI,        "li",        "display:list-item"), \
	MAP(TAG_MENU,      "menu",      "display:block","margin:1.12em 0","margin-left:40px","unicode-bidi:embed"), \
	MAP(TAG_NOFRAMES,  "noframes",  "display:block","unicode-bidi:embed"), \
	MAP(TAG_OL,        "ol",        "display:block","list-style-type:decimal","margin:1.12em 0","unicode-bidi:embed"), \
	MAP(TAG_P,         "p",         "display:block","margin:1.12em 0","unicode-bidi:embed"), \
	MAP(TAG_PRE,       "pre",       "display:block","font-family:monospace","unicode-bidi:embed","white-space:pre"), \
	MAP(TAG_S,         "s",         "text-decoration:line-through"), \
	MAP(TAG_SAMP,      "samp",      "font-family:monospace"), \
	MAP(TAG_SELECT,    "select",    "display:inline-block"), \
	MAP(TAG_SMALL,     "small",     "font-size:.83em"), \
	MAP(TAG_STRIKE,    "strike",    "text-decoration:line-through"), \
	MAP(TAG_STRONG,    "strong", 	 "font-weight:bolder"), \
	MAP(TAG_SUB,       "sub",       "font-size:.83em","vertical-align:sub"), \
	MAP(TAG_SUP,       "sup",       "font-size:.83em","vertical-align:super"), \
	MAP(TAG_TABLE,     "table",     "display:table","border-spacing:2px"), \
	MAP(TAG_TBODY,     "tbody",     "display:table-row-group","vertical-align:middle"), \
	MAP(TAG_TD,        "td",        "display:table-cell","vertical-align:inherit"), \
	MAP(TAG_TEXTAREA,  "textarea",  "display:inline-block"), \
	MAP(TAG_TFOOT,     "tfoot",     "display:table-footer-group","vertical-align:middle"), \
	MAP(TAG_TH,        "th",        "display:table-cell","font-weight:bolder","text-align:center","vertical-align:inherit"), \
	MAP(TAG_THEAD,     "thead",     "display:table-header-group","vertical-align:middle"), \
	MAP(TAG_TR,        "tr",        "display:table-row","vertical-align:inherit"), \
	MAP(TAG_TT,        "tt",        "font-family:monospace"), \
	MAP(TAG_VAR,       "var",       "font-style: italic"), \
	MAP(TAG_U,         "u",         "text-decoration:underline"), \
	MAP(TAG_UL,        "ul",        "display:block;","margin:1.12em 0","unicode-bidi:embed"), \
}
