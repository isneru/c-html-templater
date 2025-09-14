#ifndef __html_h__
#define __html_h__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include "templater.h"

#define templaterize(func) template_el(#func, func)

typedef const char* Element;

Element html_div(Element children);
Element html_span(Element children);
Element html_h1(Element children);
Element html_p(Element children);
Element html_img(const char* src, const char* alt);
Element html_a(const char* href, const char* text);
Element html_fragment(size_t count, ...);
Template template_el(const char* func_name, Element (*el)(void));

#endif  // __html_h__