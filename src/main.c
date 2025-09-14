#include "server.h"
#include "templater.h"
#include "html.h"

Templates ts = {0};

Template templates[] = {
    {"content", "This is a simple web server written in C, supporting basic HTML templating."},
    {"img_src", "https://github.com/isneru.png"},
    {"github", "https://github.com/isneru"},
    {"author", "isneru"},
    {"title", "Hello from C!"},
};

Element img() {
    HtmlAttr attrs[] = {{"width", "200"}, {"height", "200"}};

    return html_img(attrs, 2, "{{img_src}}", "isneru");
}

Element gh_profile() {
    return html_a(NULL, 0, "{{github}}", "{{author}}");
}

Element root() {
    Element el1 = html_h1(NULL, 0, "{{title}}");
    Element el2 = html_div(NULL, 0, html_fragment(2, img(), gh_profile()));
    Element el3 = html_div(
        NULL, 0, html_fragment(2, html_div(NULL, 0, html_p(NULL, 0, "{{content}}")), html_span(NULL, 0, "Nested 2"))
    );

    return html_fragment(3, el1, el2, el3);
}

int main(int argc, char const** argv) {
    // html_to_template turns Element-returning functions into a Template named by the function name
    // e.g: root() element will be a Template named "root"
    templates_append(&ts, html_to_template(root));
    templates_append_many(&ts, templates, 5);

    serve_forever(3000);

    return 0;
}