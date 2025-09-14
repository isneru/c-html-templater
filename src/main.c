#include "server.h"
#include "templater.h"
#include "html.h"

Templates ts = {0};

Template templates[] = {
    {"content", "This is a simple web server written in C, supporting basic HTML templating."},
    {"img_src", "https://github.com/isneru.png"},
    {"title", "Hello from C!"},
};

Element img() {
    return html_img("{{img_src}}", "isneru");
}

Element root() {
    Element el1 = html_div(img());
    Element el2 = html_div(html_a("{{img_src}}", "isneru"));
    Element el3 = html_div(html_p("{{content}}"));
    Element el4 = html_div(html_h1("{{title}}"));
    Element el5 = html_div(html_fragment(2, html_div("Nested div 1"), html_div("Nested div 2")));

    return html_fragment(5, el1, el2, el3, el4, el5);
}

int main(int argc, char const** argv) {
    // templaterize turns the Element-returning function into a Template named by the function name, e.g. "root"
    templates_append(&ts, templaterize(root));
    templates_append_many(&ts, templates, 3);

    serve_forever(3000);

    return 0;
}