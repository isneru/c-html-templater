# C Webserver Template

A simple HTTP web server written in C with basic HTML templating support.

## Features

- Serves static files (HTML, CSS, JS, images, documents)
- Supports variable replacement in HTML templates (e.g., `{{ title }}`)
- Handles 404 errors
- Soon-to-be cross-platform (Windows, Linux (WIP))

## Getting Started

### Prerequisites

- GCC or compatible C compiler
- Windows: Winsock2 library (included in MinGW/TDM-GCC)

### Building on Windows

```
gcc -o main.exe main.c httpd.c -lws2_32
```

### Running on Windows

```
./main.exe
```

### Project Structure

```
c-html-templater/
├─ .gitignore
├─ index.html
├─ main.c
├─ server.c
├─ server.h
└─ README.md
```

## Usage

- Place your static files in the root directory.
- Use `{{ variable }}` syntax in HTML files for templating.
- Define template variables in `main.c`.
