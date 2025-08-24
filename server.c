
#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

const char *NOT_FOUND =
    "HTTP/1.1 404 Not Found\r\n"
    "Content-Type: text/html\r\n\r\n"
    "<h1>404 Not Found</h1>";

void log_info(const char *format, ...) {
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

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("socket() error");
    exit(EXIT_FAILURE);
  }
#ifdef _WIN32
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt,
             sizeof(opt));
#else
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind() error");
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 100) < 0) {
    perror("listen() error");
    exit(EXIT_FAILURE);
  }

  return server_fd;
}

void parse_request(char *request_buffer, HttpRequest *req) {
  // Split request into lines
  char *lines[20] = {NULL};
  int line_count = 0;

  char *line = strtok(request_buffer, "\r\n");
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
    char *line = lines[i];
    char *colon = strchr(line, ':');
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
  char buffer[2048];
  int n;

  socket_t server_fd = start_server(PORT);
  log_info("Server started: http://127.0.0.1:%d", PORT);

  // ACCEPT connections
  while (1) {
    struct sockaddr_in clientaddr;
    int addrlen = sizeof(clientaddr);
    socket_t client_fd =
        accept(server_fd, (struct sockaddr *)&clientaddr, &addrlen);
    if (client_fd < 0) continue;
    // log client connection

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
