#include "html.h"

Element html_element(const char* tag, HtmlAttr* attrs, size_t attr_count, const char* children) {
    size_t attr_len = 0;
    for (size_t i = 0; i < attr_count; ++i) {
        attr_len += strlen(attrs[i].key) + strlen(attrs[i].value) + 4;  // key="value"
    }

    size_t len = strlen(tag) * 2 + attr_len + strlen(children) + 6;
    char* result = malloc(len);
    if (!result) return NULL;

    char* attr_str = malloc(attr_len + 1);
    if (!attr_str) {
        free(result);
        return NULL;
    }
    attr_str[0] = '\0';
    for (size_t i = 0; i < attr_count; ++i) {
        strcat(attr_str, " ");
        strcat(attr_str, attrs[i].key);
        strcat(attr_str, "=\"");
        strcat(attr_str, attrs[i].value);
        strcat(attr_str, "\"");
    }

    snprintf(result, len, "<%s%s>%s</%s>", tag, attr_str, children, tag);

    free(attr_str);
    return result;
}

Element html_div(HtmlAttr* attrs, size_t attr_count, const char* children) {
    return html_element("div", attrs, attr_count, children);
}

Element html_span(HtmlAttr* attrs, size_t attr_count, const char* children) {
    return html_element("span", attrs, attr_count, children);
}

Element html_h1(HtmlAttr* attrs, size_t attr_count, const char* children) {
    return html_element("h1", attrs, attr_count, children);
}

Element html_p(HtmlAttr* attrs, size_t attr_count, const char* children) {
    return html_element("p", attrs, attr_count, children);
}

Element html_img(HtmlAttr* attrs, size_t attr_count, const char* src, const char* alt) {
    size_t attr_len = 0;
    for (size_t i = 0; i < attr_count; ++i) {
        attr_len += strlen(attrs[i].key) + strlen(attrs[i].value) + 4;
    }

    char* attr_str = malloc(attr_len + attr_count + 1);
    if (!attr_str) return NULL;

    attr_str[0] = '\0';
    for (size_t i = 0; i < attr_count; ++i) {
        strcat(attr_str, " ");
        strcat(attr_str, attrs[i].key);
        strcat(attr_str, "=\"");
        strcat(attr_str, attrs[i].value);
        strcat(attr_str, "\"");
    }

    size_t len = strlen("<img src=\"\" alt=\"\" />") + strlen(src) + strlen(alt) + strlen(attr_str) + 1;

    char* result = malloc(len);
    if (!result) {
        free(attr_str);
        return NULL;
    }

    snprintf(result, len, "<img src=\"%s\" alt=\"%s\"%s />", src, alt, attr_str);

    free(attr_str);
    return result;
}

Element html_a(HtmlAttr* attrs, size_t attr_count, const char* href, const char* children) {
    HtmlAttr* new_attrs = malloc((attr_count + 1) * sizeof(HtmlAttr));
    if (!new_attrs) return NULL;

    for (size_t i = 0; i < attr_count; ++i) {
        new_attrs[i] = attrs[i];
    }
    new_attrs[attr_count] = (HtmlAttr){.key = "href", .value = href};

    Element result = html_element("a", new_attrs, attr_count + 1, children);
    free(new_attrs);
    return result;
}

Element html_fragment(size_t count, ...) {
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

Template templaterize(const char* func_name, Element (*el)(void)) {
    Template t;
    t.key = func_name;
    t.value = el();
    return t;
}
