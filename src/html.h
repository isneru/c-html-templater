#ifndef __html_h__
#define __html_h__

#include "templater.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>

#define html_to_template(func) templaterize(#func, func)

typedef const char* Element;

typedef struct {
    const char* key;
    const char* value;
} HtmlAttr;

Element html_element(const char* tag, HtmlAttr* attrs, size_t attr_count, const char* children);
Element html_fragment(size_t count, ...);
Element html_div(HtmlAttr* attrs, size_t attr_count, const char* children);
Element html_span(HtmlAttr* attrs, size_t attr_count, const char* children);
Element html_h1(HtmlAttr* attrs, size_t attr_count, const char* children);
Element html_p(HtmlAttr* attrs, size_t attr_count, const char* children);
Element html_img(HtmlAttr* attrs, size_t attr_count, const char* src, const char* alt);
Element html_a(HtmlAttr* attrs, size_t attr_count, const char* href, const char* children);

Template templaterize(const char* func_name, Element (*el)(void));

#endif  // __html_h__