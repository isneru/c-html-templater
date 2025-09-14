#ifndef __templater_h__
#define __templater_h__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

typedef struct {
    const char* key;
    const char* value;
} Template;

typedef struct {
    Template* items;
    size_t count;
    size_t capacity;
} Templates;

void templates_append(Templates* ts, Template t);
void templates_append_many(Templates* ts, Template* templates, size_t count);
int contains_placeholder(const char* value);
char* render_template(const char* template, const char* key, const char* value);
char* render_template_multi_depth(const char* template, Templates ts, int depth);
char* render_template_multi(const char* template, Templates ts);

#endif  // __templater_h__