#include "server.h"

Template templates[] = {
    {"title", "Hello from C!"},
    {"content", "This is a simple web server written in C, supporting basic HTML templating."},
    {"img_src", "https://github.com/isneru.png"},
    {"children", "<p>This is a child element.</p>"}
};

const TemplateContext ctx = {templates, .count = sizeof(templates) / sizeof(templates[0])};

int main(int argc, char const **argv) {
    serve_forever(3000);

    return 0;
}