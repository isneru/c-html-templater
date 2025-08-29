#include "server.h"
#include "templater.h"

Templates ts = {0};

Template templates[] = {
    {"content", "This is a simple web server written in C, supporting basic HTML templating."},
    {"img_src", "https://github.com/isneru.png"},
    {"children", "<p>This is a child element.</p>"}
};

int main(int argc, char const **argv) {
    templates_append(&ts, (Template){"title", "Hello from C!"});
    //  templates_append(&ts, templates[0]);
    //  templates_append(&ts, templates[1]);
    //  templates_append(&ts, templates[2]);
    templates_append_many(&ts, templates, 3);

    serve_forever(3000);

    return 0;
}