#include "templater.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void templates_append(Templates* ts, Template t) {
    if (ts->count == ts->capacity) {
        ts->capacity = ts->capacity ? ts->capacity * 2 : 256;
        ts->items = realloc(ts->items, ts->capacity * sizeof(Template));
    }
    ts->items[ts->count++] = t;
}

// Replace all occurrences of multiple {{key}} in template with their values
// Internal: Recursively expand all keys in the context, with depth limit
int contains_placeholder(const char* value) {
    return strstr(value, "{{") != NULL;
}

// Replace all occurrences of {{key}} in template with value
// Internal: Replace all occurrences of {{key}} and {{ key }} in template with
// value
char* render_template(const char* template, const char* key, const char* value) {
    size_t template_len = strlen(template);
    size_t value_len = strlen(value);
    char placeholder1[128], placeholder2[128];
    snprintf(placeholder1, sizeof(placeholder1), "{{%s}}", key);
    snprintf(placeholder2, sizeof(placeholder2), "{{ %s }}", key);
    size_t placeholder1_len = strlen(placeholder1);
    size_t placeholder2_len = strlen(placeholder2);

    // Estimate output size
    size_t out_size = template_len + 1;
    const char* p = template;
    while ((p = strstr(p, placeholder1)) != NULL) {
        out_size += value_len - placeholder1_len;
        p += placeholder1_len;
    }
    p = template;
    while ((p = strstr(p, placeholder2)) != NULL) {
        out_size += value_len - placeholder2_len;
        p += placeholder2_len;
    }

    char* result = malloc(out_size);
    if (!result) return NULL;
    result[0] = '\0';

    const char* curr = template;
    char* out = result;
    while (1) {
        const char* p1 = strstr(curr, placeholder1);
        const char* p2 = strstr(curr, placeholder2);
        const char* p = NULL;
        size_t ph_len = 0;
        if (p1 && (!p2 || p1 < p2)) {
            p = p1;
            ph_len = placeholder1_len;
        } else if (p2) {
            p = p2;
            ph_len = placeholder2_len;
        }
        if (!p) break;
        size_t n = p - curr;
        memcpy(out, curr, n);
        out += n;
        memcpy(out, value, value_len);
        out += value_len;
        curr = p + ph_len;
    }
    strcpy(out, curr);
    return result;
}

char* render_template_multi_depth(const char* template, Templates ts, int depth) {
    if (depth > 16) return strdup(template);  // Prevent infinite recursion
    char* result = strdup(template);
    for (size_t i = 0; i < ts.count; ++i) {
        char* expanded_value;
        if (contains_placeholder(ts.items[i].value)) {
            expanded_value = render_template_multi_depth(ts.items[i].value, ts, depth + 1);
        } else {
            expanded_value = strdup(ts.items[i].value);
        }
        char* replaced = render_template(result, ts.items[i].key, expanded_value);
        free(result);
        free(expanded_value);
        result = replaced;
    }
    return result;
}

// Render a template string using all keys in the context
char* render_template_multi(const char* template, Templates ts) {
    return render_template_multi_depth(template, ts, 0);
}
