# C HTML Templater

A simple HTTP web server written in C with basic HTML templating support.

## Features

- Serves static files (HTML, CSS, JS, images, documents)
- Supports variable replacement in HTML templates (e.g., `{{ title }}`)
- Handles 404 errors
- OS agnostic

## Getting Started

### Prerequisites

- GCC or compatible C compiler
- Windows: Winsock2 library (included in MinGW/TDM-GCC)
- Make

### Building and running using Makefile

```
make build
make run
```

or just `make` to build and run in one step.

### Building and running using CMake

```
cmake -S . -B build
cmake --build build
./build/Debug/c_html_templater
```

### Project Structure

```
c-html-templater/
├─ .gitignore
├─ index.html
├─ main.c
├─ Makefile
├─ server.c
├─ server.h
└─ README.md
```

## Usage

- Place your static files in the root directory.
- Use `{{ variable }}` syntax in HTML files for templating.
- Define template variables in `main.c`.

## Current Demo

Visit `http://localhost:3000` to see the server in action.

The current template is:

```html
<!doctype html>
<html lang="en">
	<head>
		<meta charset="UTF-8" />
		<meta name="viewport" content="width=device-width, initial-scale=1.0" />
		<title>{{ title }}</title>
	</head>
	<body>
		<h1>{{ title }}</h1>
		<p>{{ content }}</p>
		<img src="{{ img_src }}" alt="" />
		<div>{{ children }}</div>
	</body>
</html>
```

and it gets rendered with the following values:

```html
<!doctype html>
<html lang="en">
	<head>
		<meta charset="UTF-8" />
		<meta name="viewport" content="width=device-width, initial-scale=1.0" />
		<title>Hello from C!</title>
	</head>
	<body>
		<h1>Hello from C!</h1>
		<p>
			This is a simple web server written in C, supporting basic HTML
			templating.
		</p>
		<img src="https://github.com/isneru.png" alt="" />
		<div>
			<p>This is a child element.</p>
		</div>
	</body>
</html>
```
