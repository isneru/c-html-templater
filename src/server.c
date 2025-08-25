
#include "server.h"
#include "templater.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#endif

extern Templates ts;

const char* NOT_FOUND =
    "HTTP/1.1 404 Not Found\r\n"
    "Content-Type: text/html\r\n\r\n"
    "<h1>404 Not Found</h1>";

void log_info(const char* format, ...) {
    // Get the current time
    time_t now = time(NULL);
    char ts[20];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&now));

    // Print the timestamp
    fprintf(stderr, "[%s] ", ts);

    // Handle variadic arguments and print the message
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    // Add a newline
    fprintf(stderr, "\n");
}

int start_server(int port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return -1;
    }
#endif

    socket_t server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int max_port = port + 100;
    int bound = 0;

    for (int try_port = port; try_port <= max_port; ++try_port) {
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(try_port);

        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            perror("socket() error");
            exit(EXIT_FAILURE);
        }
#ifdef _WIN32
        int exclusive = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (const char*)&exclusive, sizeof(exclusive));
#else
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == 0) {
            bound = 1;
            port = try_port;
            break;
        } else {
            CLOSESOCKET(server_fd);
        }
    }
    if (!bound) {
        fprintf(stderr, "No available port found in range %d-%d\n", port, max_port);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 100) < 0) {
        perror("listen() error");
        exit(EXIT_FAILURE);
    }

    log_info("Server started: http://127.0.0.1:%d", port);
    return server_fd;
}

char* get_mime_type(const char* path) {
    const char* ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream";

    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".gif") == 0) return "image/gif";

    return "application/octet-stream";
}

void handle_request(HttpRequest* req, socket_t client_fd) {
    log_info("request: %s", req->uri);

    char path_buffer[512];
    const char* uri = req->uri;
    // If root, map to current directory
    if (strcmp(uri, "/") == 0) {
        snprintf(path_buffer, sizeof(path_buffer), ".");
    } else {
        if (uri[0] == '/') uri++;  // Remove leading slash for relative path
        snprintf(path_buffer, sizeof(path_buffer), "%s", uri);
    }

    struct stat st;
    if (stat(path_buffer, &st) == -1) {
        send(client_fd, NOT_FOUND, (int)strlen(NOT_FOUND), 0);
        return;
    }

    // Check if path is a directory
    int is_dir = 0;
#ifdef _WIN32
    is_dir = (st.st_mode & _S_IFDIR) != 0;
#else
    is_dir = S_ISDIR(st.st_mode);
#endif

    if (is_dir) {
        // Try to serve index.html
        char index_path[1024];
        snprintf(
            index_path, sizeof(index_path), "%s%sindex.html", path_buffer,
            (path_buffer[strlen(path_buffer) - 1] == '/' ? "" : "/")
        );
        struct stat index_st;
        if (stat(index_path, &index_st) == 0) {
            FILE* f = fopen(index_path, "rb");
            if (f) {
                char* content = malloc(index_st.st_size + 1);
                if (content) {
                    size_t read_bytes = fread(content, 1, index_st.st_size, f);
                    content[index_st.st_size] = '\0';
                    fclose(f);
                    if (read_bytes == index_st.st_size) {
                        char* rendered = render_template_multi(content, ts);
                        char header[256];
                        snprintf(
                            header, sizeof(header),
                            "HTTP/1.1 %s\r\nContent-Type: "
                            "text/html\r\nContent-Length: %lld\r\n\r\n",
                            HTTP_200, (long long)strlen(rendered)
                        );
                        send(client_fd, header, (int)strlen(header), 0);
                        send(client_fd, rendered, (int)strlen(rendered), 0);
                        free(content);
                        free(rendered);
                        return;
                    }
                    free(content);
                }
                fclose(f);
            }
        }
        // No index.html, list files in directory (OS-agnostic)
        char html[4096];
        snprintf(
            html, sizeof(html),
            "<html><head><title>Directory "
            "listing</title></head><body><h1>Directory listing for "
            "/%s</h1><ul>",
            uri
        );
#ifdef _WIN32
        WIN32_FIND_DATA findFileData;
        char search_path[512];
        snprintf(
            search_path, sizeof(search_path), "%s%s*", path_buffer,
            (path_buffer[strlen(path_buffer) - 1] == '/' ? "" : "/")
        );
        HANDLE hFind = FindFirstFile(search_path, &findFileData);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..") == 0) continue;
                int entry_is_dir = (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
                snprintf(
                    html + strlen(html), sizeof(html) - strlen(html), "<li><a href=\"%s/%s\">%s%s</a></li>", uri,
                    findFileData.cFileName, findFileData.cFileName, entry_is_dir ? "/" : ""
                );
            } while (FindNextFile(hFind, &findFileData));
            FindClose(hFind);
        }
#else
        DIR* dir = opendir(path_buffer);
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
                // Check if entry is a directory
                char entry_path[1024];
                snprintf(
                    entry_path, sizeof(entry_path), "%s%s%s", path_buffer,
                    (path_buffer[strlen(path_buffer) - 1] == '/' ? "" : "/"), entry->d_name
                );
                struct stat entry_st;
                int entry_is_dir = 0;
                if (stat(entry_path, &entry_st) == 0) {
#ifdef _WIN32
                    entry_is_dir = (entry_st.st_mode & _S_IFDIR) != 0;
#else
                    entry_is_dir = S_ISDIR(entry_st.st_mode);
#endif
                }
                snprintf(
                    html + strlen(html), sizeof(html) - strlen(html), "<li><a href=\"%s/%s\">%s%s</a></li>", uri,
                    entry->d_name, entry->d_name, entry_is_dir ? "/" : ""
                );
            }
            closedir(dir);
        }
#endif
        strcat(html, "</ul></body></html>");
        char header[256];
        snprintf(
            header, sizeof(header),
            "HTTP/1.1 %s\r\nContent-Type: text/html\r\nContent-Length: "
            "%lld\r\n\r\n",
            HTTP_200, (long long)strlen(html)
        );
        send(client_fd, header, (int)strlen(header), 0);
        send(client_fd, html, (int)strlen(html), 0);
        return;
    }

    // If not a directory, serve file as before
    FILE* f = fopen(path_buffer, "rb");
    if (!f) {
        send(client_fd, NOT_FOUND, (int)strlen(NOT_FOUND), 0);
        return;
    }

    char* content = malloc(st.st_size + 1);
    if (!content) {
        fclose(f);
        return;
    }

    size_t read_bytes = fread(content, 1, st.st_size, f);
    content[st.st_size] = '\0';
    fclose(f);
    if (read_bytes != st.st_size) {
        free(content);
        return;
    }

    // Only template .html files
    const char* mime_type = get_mime_type(req->uri);
    char* rendered = NULL;
    if (strcmp(mime_type, "text/html") == 0) {
        rendered = render_template_multi(content, ts);
    } else {
        rendered = strdup(content);
    }

    char header[256];
    snprintf(
        header, sizeof(header),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %lld\r\n\r\n",
        HTTP_200, mime_type, (long long)strlen(rendered)
    );

    send(client_fd, header, (int)strlen(header), 0);
    send(client_fd, rendered, (int)strlen(rendered), 0);

    free(content);
    free(rendered);
}

void parse_request(char* request_buffer, HttpRequest* req) {
    // Split request into lines
    char* lines[20] = {NULL};
    int line_count = 0;

    char* line = strtok(request_buffer, "\r\n");
    while (line) {
        lines[line_count++] = line;
        line = strtok(NULL, "\r\n");
        if (line_count > 19) break;
    }
    // for(size_t i=0; lines[i] && i<20; i++) printf("--> %s\n", lines[i]);

    // parse request line
    req->method = strtok(lines[0], " ");
    req->uri = strtok(NULL, " ");
    req->http_version = strtok(NULL, "\r\n");

    // parse headers
    req->header_count = 0;
    for (size_t i = 0; lines[i]; i++) {
        char* line = lines[i];
        char* colon = strchr(line, ':');
        if (!colon) continue;

        *colon = '\0';
        colon++;
        while (*colon == ' ' || *colon == '\t') colon++;
        req->headers[req->header_count].key = line;
        req->headers[req->header_count].value = colon;
        req->header_count++;
    }
}

void serve_forever(int PORT) {
    socket_t server_fd = start_server(PORT);

    // ACCEPT connections
    while (1) {
        struct sockaddr_in clientaddr;
        socklen_t addrlen = sizeof(clientaddr);
        socket_t client_fd = accept(server_fd, (struct sockaddr*)&clientaddr, &addrlen);
        if (client_fd < 0) continue;

        char request_buffer[8192];
        int n = recv(client_fd, request_buffer, sizeof(request_buffer), 0);

        if (n < 0) {
            perror("read error");
        } else if (n == 0) {
            perror("client disconnected");
        } else {
            HttpRequest req;
            parse_request(request_buffer, &req);  // Parse the request
            handle_request(&req, client_fd);      // Serve the file or 404
        }

        CLOSESOCKET(client_fd);
    }

    CLOSESOCKET(server_fd);
}
