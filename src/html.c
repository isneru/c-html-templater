#include "html.h"

// TODO: generalize element creation with tag name and attributes

const char* html_div(const char* children) {
    size_t len = strlen(children) + strlen("<div></div>") + 1;
    char* result = malloc(len);
    if (!result) return NULL;
    snprintf(result, len, "<div>%s</div>", children);
    return result;
}

const char* html_span(const char* children) {
    size_t len = strlen(children) + strlen("<span></span>") + 1;
    char* result = malloc(len);
    if (!result) return NULL;
    snprintf(result, len, "<span>%s</span>", children);
    return result;
}

const char* html_h1(const char* children) {
    size_t len = strlen(children) + strlen("<h1></h1>") + 1;
    char* result = malloc(len);
    if (!result) return NULL;
    snprintf(result, len, "<h1>%s</h1>", children);
    return result;
}

const char* html_p(const char* children) {
    size_t len = strlen(children) + strlen("<p></p>") + 1;
    char* result = malloc(len);
    if (!result) return NULL;
    snprintf(result, len, "<p>%s</p>", children);
    return result;
}

const char* html_img(const char* src, const char* alt) {
    size_t len = strlen(src) + strlen(alt) + strlen("<img src=\"\" alt=\"\"/>") + 1;
    char* result = malloc(len);
    if (!result) return NULL;
    snprintf(result, len, "<img src=\"%s\" alt=\"%s\"/>", src, alt);
    return result;
}

const char* html_a(const char* href, const char* text) {
    size_t len = strlen(href) + strlen(text) + strlen("<a href=\"\"></a>") + 1;
    char* result = malloc(len);
    if (!result) return NULL;
    snprintf(result, len, "<a href=\"%s\">%s</a>", href, text);
    return result;
}

const char* html_fragment(size_t count, ...) {
    va_list args;
    va_start(args, count);
    size_t total_len = 1;

    for (size_t i = 0; i < count; ++i) {
        Element el = va_arg(args, Element);
        if (el) total_len += strlen(el);
    }
    va_end(args);

    char* result = malloc(total_len + 1);
    if (!result) return NULL;

    result[0] = '\0';
    va_start(args, count);
    for (size_t i = 0; i < count; ++i) {
        Element el = va_arg(args, Element);
        if (el) strcat(result, el);
    }

    va_end(args);
    return result;
}

Template template_el(const char* func_name, Element (*el)(void)) {
    Template t;
    t.key = func_name;
    t.value = el();
    return t;
}