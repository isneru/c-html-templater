#ifndef __server_h__
#define __server_h__

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
#define CLOSESOCKET closesocket
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
typedef int socket_t;
#define CLOSESOCKET close
#endif

#define HTTP_200 "200 OK"
#define HTTP_204 "204 No Content"
#define HTTP_400 "400 Bad Request"
#define HTTP_401 "401 Unauthorized"
#define HTTP_404 "404 Not Found"

extern const char *NOT_FOUND;

typedef struct {
  char *key;    // Header name
  char *value;  // Header value
} HttpHeader;

typedef struct {
  char *method;            // e.g., GET, POST
  char *uri;               // Resource URI
  char *http_version;      // HTTP version (e.g., HTTP/1.1)
  int header_count;        // Number of headers
  HttpHeader headers[20];  // Array of headers
} HttpRequest;

void log_info(const char *format, ...);

void serve_forever(int PORT);
void handle_request(HttpRequest *request, socket_t client_fd);

#endif
