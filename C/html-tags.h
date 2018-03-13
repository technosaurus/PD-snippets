#define HTML_TAGMAP(OP, ...) \
	/* ENUM/TAG    ... DEFAULT CSS PROPERTIES ... */ \
	OP(!--,       /* */) __VA_ARGS__ \
	OP(!DOCTYPE,  /* */) __VA_ARGS__ \
	OP(a,         /*hyperlink*/) __VA_ARGS__ \
	OP(abbr,      /*abbr*/) __VA_ARGS__ \
	OP(acronym,   /*abbr*/) __VA_ARGS__ \
	OP(address,   "display:block","unicode-bidi:embed") __VA_ARGS__ \
	OP(applet,    /* */) __VA_ARGS__ \
	OP(area,      /*area inside an image map*/) __VA_ARGS__ \
	OP(article,   /* */) __VA_ARGS__ \
	OP(aside,     /* */) __VA_ARGS__ \
	OP(audio,     /* */) __VA_ARGS__ \
	OP(b,         "font-weight:bolder") __VA_ARGS__ \
	OP(base,      /* */) __VA_ARGS__ \
	OP(basefont,  /* */) __VA_ARGS__ \
	OP(bdi,       /* */) __VA_ARGS__ \
	OP(bdo,       /* */) __VA_ARGS__ \
	OP(big,       "font-size:1.17em") __VA_ARGS__ \
	OP(blockquote,"display:block","margin:1.12em 0","margin-left:40px","margin-right:40px","unicode-bidi:embed") __VA_ARGS__ \
	OP(body,      "display:block","margin:8px","unicode-bidi:embed") __VA_ARGS__ \
	OP(br,         /* */) __VA_ARGS__ \
	OP(button,    "display:inline-block") __VA_ARGS__ \
	OP(canvas,    /* */) __VA_ARGS__ \
	OP(caption,   "display:table-caption") __VA_ARGS__ \
	OP(center,    "display:block","text-align:center","unicode-bidi:embed") __VA_ARGS__ \
	OP(cite,      "font-style: italic") __VA_ARGS__ \
	OP(code,      "font-family:monospace") __VA_ARGS__ \
	OP(col,       "display:table-column") __VA_ARGS__ \
	OP(colgroup,  "display:table-column-group") __VA_ARGS__ \
	OP(datalist,  /* */) __VA_ARGS__ \
	OP(dd,        "display:block","margin-left:40px","unicode-bidi:embed") __VA_ARGS__ \
	OP(del,       "text-decoration:line-through") __VA_ARGS__ \
	OP(details,   /* */) __VA_ARGS__ \
	OP(dfn,       /* */) __VA_ARGS__ \
	OP(dialog,    /* */) __VA_ARGS__ \
	OP(dir,       "display:block","margin:1.12em 0","unicode-bidi:embed") __VA_ARGS__ \
	OP(div,       "display:block","unicode-bidi:embed") __VA_ARGS__ \
	OP(dl,        "display:block","margin:1.12em 0","margin:1.12em 0","unicode-bidi:embed") __VA_ARGS__ \
	OP(dt,        "display:block","unicode-bidi:embed") __VA_ARGS__ \
	OP(em,        "font-style: italic") __VA_ARGS__ \
	OP(embed,     /* */) __VA_ARGS__ \
	OP(fieldset,  "display:block","margin:1.12em 0","unicode-bidi:embed") __VA_ARGS__ \
	OP(figcaption,/* */) __VA_ARGS__ \
	OP(figure,    /* */) __VA_ARGS__ \
	OP(font,      /* */) __VA_ARGS__ \
	OP(footer,    /* */) __VA_ARGS__ \
	OP(form,      "display:block","margin:1.12em 0","unicode-bidi:embed") __VA_ARGS__ \
	OP(frame,     "display:block","unicode-bidi:embed") __VA_ARGS__ \
	OP(frameset,  "display:block","unicode-bidi:embed") __VA_ARGS__ \
	OP(h1,        "display:block","font-size:2em","font-weight:bolder","margin:.67em 0","unicode-bidi:embed") __VA_ARGS__ \
	OP(h2,        "display:block","font-size:1.5em","font-weight:bolder","margin:.75em 0","unicode-bidi:embed") __VA_ARGS__ \
	OP(h3,        "display:block","font-size: 1.17em","font-weight:bolder","margin:.83em 0","unicode-bidi:embed") __VA_ARGS__ \
	OP(h4,        "display:block","margin:1.12em 0","font-weight:bolder","unicode-bidi:embed") __VA_ARGS__ \
	OP(h5,        "display:block","font-size:.83em","font-weight:bolder","margin:1.5em 0","unicode-bidi:embed") __VA_ARGS__ \
	OP(h6,        "display:block","font-size: .75em","font-weight:bolder","margin: 1.67em 0","unicode-bidi:embed") __VA_ARGS__ \
	OP(head,      "display:none") __VA_ARGS__ \
	OP(header,    /* */) __VA_ARGS__ \
	OP(hr,        "border:1px inset","display:block","unicode-bidi:embed") __VA_ARGS__ \
	OP(html,      "display:block","unicode-bidi:embed") __VA_ARGS__ \
	OP(i,         "font-style: italic") __VA_ARGS__ \
	OP(iframe,    /* */) __VA_ARGS__ \
	OP(img,       /* */) __VA_ARGS__ \
	OP(input,     "display:inline-block") __VA_ARGS__ \
	OP(ins,       "text-decoration:underline") __VA_ARGS__ \
	OP(kbd,       "font-family:monospace") __VA_ARGS__ \
	OP(label,     /* */) __VA_ARGS__ \
	OP(legend,    /* */) __VA_ARGS__ \
	OP(li,        "display:list-item") __VA_ARGS__ \
	OP(link,      /* */) __VA_ARGS__ \
	OP(main,      /* */) __VA_ARGS__ \
	OP(map,       /* */) __VA_ARGS__ \
	OP(mark,      /* */) __VA_ARGS__ \
	OP(menu,      "display:block","margin:1.12em 0","margin-left:40px","unicode-bidi:embed") __VA_ARGS__ \
	OP(menuitem,  /* */) __VA_ARGS__ \
	OP(meta,      /* */) __VA_ARGS__ \
	OP(meter,     /* */) __VA_ARGS__ \
	OP(nav,       /* */) __VA_ARGS__ \
	OP(noframes,  "display:block","unicode-bidi:embed") __VA_ARGS__ \
	OP(noscript,  /* */) __VA_ARGS__ \
	OP(object,    /* */) __VA_ARGS__ \
	OP(ol,        "display:block","list-style-type:decimal","margin:1.12em 0","unicode-bidi:embed") __VA_ARGS__ \
	OP(optgroup,  /* */) __VA_ARGS__ \
	OP(option,    /* */) __VA_ARGS__ \
	OP(output,    /* */) __VA_ARGS__ \
	OP(p,         "display:block","margin:1.12em 0","unicode-bidi:embed") __VA_ARGS__ \
	OP(param,     /* */) __VA_ARGS__ \
	OP(picture,   /* */) __VA_ARGS__ \
	OP(pre,       "display:block","font-family:monospace","unicode-bidi:embed","white-space:pre") __VA_ARGS__ \
	OP(progress,  /* */) __VA_ARGS__ \
	OP(q,         /* */) __VA_ARGS__ \
	OP(rp,        /* */) __VA_ARGS__ \
	OP(rt,        /* */) __VA_ARGS__ \
	OP(ruby,      /* */) __VA_ARGS__ \
	OP(s,         "text-decoration:line-through") __VA_ARGS__ \
	OP(samp,      "font-family:monospace") __VA_ARGS__ \
	OP(select,    "display:inline-block") __VA_ARGS__ \
	OP(small,     "font-size:.83em") __VA_ARGS__ \
	OP(source,    /* */) __VA_ARGS__ \
	OP(span,         /* */) __VA_ARGS__ \
	OP(strike,    "text-decoration:line-through") __VA_ARGS__ \
	OP(strong,    "font-weight:bolder") __VA_ARGS__ \
	OP(style,     /* */) __VA_ARGS__ \
	OP(sub,       "font-size:.83em","vertical-align:sub") __VA_ARGS__ \
	OP(summary,   /* */) __VA_ARGS__ \
	OP(sup,       "font-size:.83em","vertical-align:super") __VA_ARGS__ \
	OP(svg,         /* */) __VA_ARGS__ \
	OP(table,     "display:table","border-spacing:2px") __VA_ARGS__ \
	OP(tbody,     "display:table-row-group","vertical-align:middle") __VA_ARGS__ \
	OP(td,        "display:table-cell","vertical-align:inherit") __VA_ARGS__ \
	OP(template,  /* */) __VA_ARGS__ \
	OP(textarea,  "display:inline-block") __VA_ARGS__ \
	OP(tfoot,     "display:table-footer-group","vertical-align:middle") __VA_ARGS__ \
	OP(th,        "display:table-cell","font-weight:bolder","text-align:center","vertical-align:inherit") __VA_ARGS__ \
	OP(thead,     "display:table-header-group","vertical-align:middle") __VA_ARGS__ \
	OP(time,      /* */) __VA_ARGS__ \
	OP(title,     /* */) __VA_ARGS__ \
	OP(tr,        "display:table-row","vertical-align:inherit") __VA_ARGS__ \
	OP(track,     /* */) __VA_ARGS__ \
	OP(tt,        "font-family:monospace") __VA_ARGS__ \
	OP(u,         "text-decoration:underline") __VA_ARGS__ \
	OP(ul,        "display:block;","margin:1.12em 0","unicode-bidi:embed") __VA_ARGS__
	OP(var,       "font-style: italic") __VA_ARGS__ \
	OP(video,     /* */)) __VA_ARGS__ \
	OP(wbr,       /* */)) __VA_ARGS__ \
