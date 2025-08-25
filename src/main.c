#include "server.h"
#include "templater.h"

Templates ts = {0};

int main(int argc, char const **argv) {
    templates_append(&ts, (Template){"title", "Hello from C!"});
    templates_append(
        &ts, (Template){"content", "This is a simple web server written in C, supporting basic HTML templating."}
    );
    templates_append(&ts, (Template){"img_src", "https://github.com/isneru.png"});
    templates_append(&ts, (Template){"children", "<p>This is a child element.</p>"});

    serve_forever(3000);

    return 0;
}