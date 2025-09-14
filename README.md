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

## Building

You can build and run the project using one of the following methods:

- Makefile
- CMake
- [Nob Build System](https://github.com/tsoding/nob.h)

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

### Building and running using [Nob Build System](https://github.com/tsoding/nob.h)

> [!NOTE]  
> You only need to compile nob once with `cc -o nob nob.c`, from then on it will rebuild itself if you modify `nob.c`.

```
cc -o nob nob.c
./nob
```

### Project Structure

```
c-html-templater/
├─ src/
│  ├─ html.c
│  ├─ html.h
│  ├─ main.c
│  ├─ server.c
│  ├─ server.h
│  ├─ templater.c
│  └─ templater.h
├─ index.html
├─ CMakeLists.txt
├─ Makefile
├─ nob.c
├─ nob.h
├─ .clang-format
├─ .gitignore
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
		{{ root }}
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
		<div>
			<img
				src="https://github.com/isneru.png"
				alt="isneru"
				width="100"
				height="100" />
			<a href="https://github.com/isneru">isneru</a>
		</div>
		<div>
			<div>
				<p>
					This is a simple web server written in C, supporting basic HTML
					templating.
				</p>
			</div>
			<span>Nested 2</span>
		</div>
	</body>
</html>
```
