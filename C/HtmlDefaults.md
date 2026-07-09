
## Core Standard HTML Tags & Default CSS (ASCII Order)

| HTML Tag | Default CSS Properties |
|---|---|
| a | color: -webkit-link; text-decoration: underline; cursor: pointer; |
| abbr | text-decoration: dotted underline; |
| address | display: block; font-style: italic; |
| area | display: none; |
| article | display: block; |
| aside | display: block; |
| b | font-weight: bold; |
| blockquote | display: block; margin-top: 1em; margin-bottom: 1em; margin-left: 40px; margin-right: 40px; |
| body | display: block; margin: 8px; |
| br | display: inline; |
| button | display: inline-block; appearance: button; box-sizing: border-box; |
| caption | display: table-caption; text-align: center; |
| cite | font-style: italic; |
| code | font-family: monospace; |
| col | display: table-column; |
| colgroup | display: table-column-group; |
| dd | display: block; margin-left: 40px; |
| del | text-decoration: line-through; |
| details | display: block; |
| dfn | font-style: italic; |
| div | display: block; |
| dl | display: block; margin-top: 1em; margin-bottom: 1em; |
| dt | display: block; |
| em | font-style: italic; |
| embed | display: inline-block; overflow: hidden; |
| fieldset | display: block; margin-inline: 2px; padding-block: 0.35em; padding-inline: 0.75em; border: 2px groove; |
| figcaption | display: block; |
| figure | display: block; margin-top: 1em; margin-bottom: 1em; margin-left: 40px; margin-right: 40px; |
| footer | display: block; |
| form | display: block; margin-top: 0em; |
| h1 | display: block; font-size: 2em; margin-block: 0.67em; font-weight: bold; |
| h2 | display: block; font-size: 1.5em; margin-block: 0.83em; font-weight: bold; |
| h3 | display: block; font-size: 1.17em; margin-block: 1em; font-weight: bold; |
| h4 | display: block; font-size: 1em; margin-block: 1.33em; font-weight: bold; |
| h5 | display: block; font-size: 0.83em; margin-block: 1.67em; font-weight: bold; |
| h6 | display: block; font-size: 0.67em; margin-block: 2.33em; font-weight: bold; |
| head | display: none; |
| header | display: block; |
| hgroup | display: block; |
| hr | display: block; margin-top: 0.5em; margin-bottom: 0.5em; border-style: inset; border-width: 1px; |
| html | display: block; |
| i | font-style: italic; |
| iframe | display: inline-block; border: 2px inset; |
| img | display: inline-block; |
| input | display: inline-block; padding: 1px 2px; box-sizing: border-box; |
| ins | text-decoration: underline; |
| kbd | font-family: monospace; |
| label | cursor: default; |
| legend | display: block; padding-inline: 2px; border: none; |
| li | display: list-item; |
| link | display: none; |
| main | display: block; |
| map> | display: inline; |
| mark | background-color: yellow; color: black; |
| menu | display: block; list-style-type: disc; margin-top: 1em; margin-bottom: 1em; |
| meta | display: none; |
| nav | display: block; |
| noscript | display: none; |
| object | display: inline-block; |
| ol | display: block; list-style-type: decimal; margin-top: 1em; margin-bottom: 1em; |
| option | display: block; |
| p | display: block; margin-top: 1em; margin-bottom: 1em; |
| param | display: none; |
| pre | display: block; font-family: monospace; white-space: pre; margin-top: 1em; margin-bottom: 1em; |
| q | display: inline; |
| s | text-decoration: line-through; |
| samp | font-family: monospace; |
| script | display: none; |
| section | display: block; |
| select | display: inline-block; box-sizing: border-box; |
| small | font-size: smaller; |
| span | display: inline; |
| strong | font-weight: bold; |
| style | display: none; |
| sub | vertical-align: sub; font-size: smaller; |
| sup | vertical-align: super; font-size: smaller; |
| table | display: table; border-collapse: separate; border-spacing: 2px; border-color: gray; |
| tbody | display: table-row-group; vertical-align: middle; border-color: inherit; |
| td | display: table-cell; vertical-align: inherit; padding: 1px; |
| textarea | display: inline-block; font-family: monospace; box-sizing: border-box; |
| tfoot | display: table-footer-group; vertical-align: middle; border-color: inherit; |
| th | display: table-cell; font-weight: bold; text-align: center; |
| thead | display: table-header-group; vertical-align: middle; border-color: inherit; |
| title> | display: none; |
| tr> | display: table-row; vertical-align: inherit; border-color: inherit; |
| u> | text-decoration: underline; |
| ul | display: block; list-style-type: disc; margin-top: 1em; margin-bottom: 1em; |
| video | display: inline-block; |

```c
#include <stddef.h>

typedef struct {
    const char *tag_name;
    const char *default_css;
} HTMLTagDefaultStyle;

/* 
 * ASCII-Sorted Array of HTML tags and their default layout properties.
 * Compiled according to the WHATWG User Agent consensus rendering model.
 * Monospace fallback, proportional font scalings, and table formatting are preserved.
 */
static const HTMLTagDefaultStyle HTML_TAG_DEFAULTS[] = {
    { "a",          "display: inline; cursor: pointer; text-decoration: underline;" },
    { "abbr",       "display: inline; text-decoration: dotted underline;" },
    { "address",    "display: block; font-style: italic;" },
    { "area",       "display: none;" },
    { "article",    "display: block;" },
    { "aside",      "display: block;" },
    { "audio",      "display: inline-block;" },
    { "b",          "display: inline; font-weight: bold;" },
    { "base",       "display: none;" },
    { "bdi",        "display: inline;" },
    { "bdo",        "display: inline; unicode-bidi: bidi-override;" },
    { "blockquote", "display: block; margin-top: 1em; margin-bottom: 1em; margin-left: 40px; margin-right: 40px;" },
    { "body",       "display: block; margin: 8px; line-height: normal;" },
    { "br",         "display: inline; content: '\\A';" }, /* Layout break hint */
    { "button",     "display: inline-block; margin: 0; font: inherit; box-sizing: border-box;" },
    { "canvas",     "display: inline-block;" },
    { "caption",    "display: table-caption; text-align: center;" },
    { "cite",       "display: inline; font-style: italic;" },
    { "code",       "display: inline; font-family: monospace;" },
    { "col",        "display: table-column;" },
    { "colgroup",   "display: table-column-group;" },
    { "data",       "display: inline;" },
    { "datalist",   "display: none;" },
    { "dd",         "display: block; margin-left: 40px;" },
    { "del",        "display: inline; text-decoration: line-through;" },
    { "details",    "display: block;" },
    { "dfn",        "display: inline; font-style: italic;" },
    { "dialog",     "display: none; position: absolute; margin: auto; border: solid; padding: 1em; background: white;" },
    { "div",        "display: block;" },
    { "dl",         "display: block; margin-top: 1em; margin-bottom: 1em;" },
    { "dt",         "display: block;" },
    { "em",         "display: inline; font-style: italic;" },
    { "embed",      "display: inline-block;" },
    { "fieldset",   "display: block; margin-left: 2px; margin-right: 2px; padding: 0.35em 0.75em 0.625em; border: 2px groove;" },
    { "figcaption", "display: block;" },
    { "figure",     "display: block; margin-top: 1em; margin-bottom: 1em; margin-left: 40px; margin-right: 40px;" },
    { "footer",     "display: block;" },
    { "form",       "display: block;" },
    { "h1",         "display: block; font-size: 2.00em; margin-top: 0.67em; margin-bottom: 0.67em; font-weight: bold;" },
    { "h2",         "display: block; font-size: 1.50em; margin-top: 0.83em; margin-bottom: 0.83em; font-weight: bold;" },
    { "h3",         "display: block; font-size: 1.17em; margin-top: 1.00em; margin-bottom: 1.00em; font-weight: bold;" },
    { "h4",         "display: block; font-size: 1.00em; margin-top: 1.33em; margin-bottom: 1.33em; font-weight: bold;" },
    { "h5",         "display: block; font-size: 0.83em; margin-top: 1.67em; margin-bottom: 1.67em; font-weight: bold;" },
    { "h6",         "display: block; font-size: 0.67em; margin-top: 2.33em; margin-bottom: 2.33em; font-weight: bold;" },
    { "head",       "display: none;" },
    { "header",     "display: block;" },
    { "hgroup",     "display: block;" },
    { "hr",         "display: block; margin-top: 0.5em; margin-bottom: 0.5em; border-style: inset; border-width: 1px;" },
    { "html",       "display: block; address: block;" },
    { "i",          "display: inline; font-style: italic;" },
    { "iframe",     "display: inline-block; border: 2px inset;" },
    { "img",        "display: inline-block;" },
    { "input",      "display: inline-block; margin: 0; font: inherit;" },
    { "ins",        "display: inline; text-decoration: underline;" },
    { "kbd",        "display: inline; font-family: monospace;" },
    { "label",      "display: inline; cursor: default;" },
    { "legend",     "display: block; padding-left: 2px; padding-right: 2px; border: none;" },
    { "li",         "display: list-item;" },
    { "link",       "display: none;" },
    { "main",       "display: block;" },
    { "map",        "display: inline;" },
    { "mark",       "display: inline; background-color: yellow; color: black;" },
    { "menu",       "display: block; list-style-type: disc; margin-top: 1em; margin-bottom: 1em; padding-left: 40px;" },
    { "meta",       "display: none;" },
    { "meter",      "display: inline-block;" },
    { "nav",        "display: block;" },
    { "noscript",   "display: none;" }, /* Assuming JS engine is running */
    { "object",     "display: inline-block;" },
    { "ol",         "display: block; list-style-type: decimal; margin-top: 1em; margin-bottom: 1em; padding-left: 40px;" },
    { "optgroup",   "display: block;" },
    { "option",     "display: block;" },
    { "output",     "display: inline;" },
    { "p",          "display: block; margin-top: 1em; margin-bottom: 1em;" },
    { "param",      "display: none;" },
    { "picture",    "display: inline;" },
    { "pre",        "display: block; font-family: monospace; white-space: pre; margin-top: 1em; margin-bottom: 1em;" },
    { "progress",   "display: inline-block;" },
    { "q",          "display: inline;" },
    { "rp",         "display: none;" },
    { "rt",         "display: block; font-size: 50%;" },
    { "ruby",       "display: inline;" },
    { "s",          "display: inline; text-decoration: line-through;" },
    { "samp",       "display: inline; font-family: monospace;" },
    { "script",     "display: none;" },
    { "search",     "display: block;" },
    { "section",    "display: block;" },
    { "select",     "display: inline-block; margin: 0; font: inherit;" },
    { "slot",       "display: inline;" },
    { "small",      "display: inline; font-size: smaller;" },
    { "source",     "display: none;" },
    { "span",       "display: inline;" },
    { "strong",     "display: inline; font-weight: bold;" },
    { "style",      "display: none;" },
    { "sub",        "display: inline; vertical-align: sub; font-size: smaller;" },
    { "summary",    "display: block;" },
    { "sup",        "display: inline; vertical-align: super; font-size: smaller;" },
    { "table",      "display: table; border-collapse: separate; border-spacing: 2px; border-color: gray;" },
    { "tbody",      "display: table-row-group; vertical-align: middle; border-color: inherit;" },
    { "td",         "display: table-cell; vertical-align: inherit; padding: 1px;" },
    { "template",   "display: none;" },
    { "textarea",   "display: inline-block; margin: 0; font: inherit; white-space: pre-wrap; resize: both;" },
    { "tfoot",      "display: table-footer-group; vertical-align: middle; border-color: inherit;" },
    { "th",         "display: table-cell; font-weight: bold; vertical-align: inherit; text-align: center; padding: 1px;" },
    { "thead",      "display: table-header-group; vertical-align: middle; border-color: inherit;" },
    { "time",       "display: inline;" },
    { "title",      "display: none;" },
    { "tr",         "display: table-row; vertical-align: inherit; border-color: inherit;" },
    { "track",      "display: none;" },
    { "u",          "display: inline; text-decoration: underline;" },
    { "ul",         "display: block; list-style-type: disc; margin-top: 1em; margin-bottom: 1em; padding-left: 40px;" },
    { "var",        "display: inline; font-style: italic;" },
    { "video",      "display: inline-block;" },
    { "wbr",        "display: inline;" }
};

static const size_t HTML_TAG_DEFAULTS_COUNT = sizeof(HTML_TAG_DEFAULTS) / sizeof(HTML_TAG_DEFAULTS[0]);
```
