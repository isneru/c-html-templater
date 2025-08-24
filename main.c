
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "server.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

typedef struct {
  const char* key;
  const char* value;
} Template;

Template templates[] = {{"title", "Hello from C!"},
                        {"content",
                         "This is a simple web server written in C, supporting "
                         "basic HTML templating."},
                        {"img_src", "https://github.com/isneru.png"}};

size_t t_count = 3;

int main(int argc, char const** argv) {
  serve_forever(3000);

  return 0;
}

char* get_mime_type(const char* path) {
  const char* ext = strrchr(path, '.');
  if (!ext) return "application/octet-stream";

  if (strcmp(ext, ".html") == 0) return "text/html";
  if (strcmp(ext, ".css") == 0) return "text/css";
  if (strcmp(ext, ".js") == 0) return "application/javascript";
  if (strcmp(ext, ".png") == 0) return "image/png";
  if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0)
    return "image/jpeg";
  if (strcmp(ext, ".gif") == 0) return "image/gif";

  return "application/octet-stream";
}

// Replace all occurrences of {{key}} in template with value
char* render_template(const char* template, const char* key,
                      const char* value) {
  size_t template_len = strlen(template);
  size_t key_len = strlen(key);
  size_t value_len = strlen(value);
  char placeholder[128];
  snprintf(placeholder, sizeof(placeholder), "{{ %s }}", key);
  size_t placeholder_len = strlen(placeholder);

  // Estimate output size
  size_t out_size = template_len + 1;
  const char* p = template;
  while ((p = strstr(p, placeholder)) != NULL) {
    out_size += value_len - placeholder_len;
    p += placeholder_len;
  }

  char* result = malloc(out_size);
  if (!result) return NULL;
  result[0] = '\0';

  const char* curr = template;
  char* out = result;
  while ((p = strstr(curr, placeholder)) != NULL) {
    size_t n = p - curr;
    memcpy(out, curr, n);
    out += n;
    memcpy(out, value, value_len);
    out += value_len;
    curr = p + placeholder_len;
  }
  strcpy(out, curr);
  return result;
}

// Replace all occurrences of multiple {{key}} in template with their values
char* render_template_multi(const char* template, Template* templates,
                            size_t count) {
  char* result = strdup(template);
  for (size_t i = 0; i < count; ++i) {
    char* replaced =
        render_template(result, templates[i].key, templates[i].value);
    free(result);
    result = replaced;
  }
  return result;
}

void handle_request(HttpRequest* req, SOCKET client_fd) {
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
    char index_path[512];
    snprintf(index_path, sizeof(index_path), "%s%sindex.html", path_buffer,
             (path_buffer[strlen(path_buffer) - 1] == '/' ? "" : "/"));
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
            char* rendered = render_template_multi(content, templates, t_count);
            char header[256];
            snprintf(header, sizeof(header),
                     "HTTP/1.1 %s\r\nContent-Type: "
                     "text/html\r\nContent-Length: %lld\r\n\r\n",
                     HTTP_200, (long long)strlen(rendered));
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
        "listing</title></head><body><h1>Directory listing for /%s</h1><ul>",
        uri);
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    char search_path[512];
    snprintf(search_path, sizeof(search_path), "%s%s*", path_buffer,
             (path_buffer[strlen(path_buffer) - 1] == '/' ? "" : "/"));
    HANDLE hFind = FindFirstFile(search_path, &findFileData);
    if (hFind != INVALID_HANDLE_VALUE) {
      do {
        if (strcmp(findFileData.cFileName, ".") == 0 ||
            strcmp(findFileData.cFileName, "..") == 0)
          continue;
        int entry_is_dir =
            (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
        snprintf(html + strlen(html), sizeof(html) - strlen(html),
                 "<li><a href=\"%s/%s\">%s%s</a></li>", uri,
                 findFileData.cFileName, findFileData.cFileName,
                 entry_is_dir ? "/" : "");
      } while (FindNextFile(hFind, &findFileData));
      FindClose(hFind);
    }
#else
    DIR* dir = opendir(path_buffer);
    if (dir) {
      struct dirent* entry;
      while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
          continue;
        // Check if entry is a directory
        char entry_path[512];
        snprintf(entry_path, sizeof(entry_path), "%s%s%s", path_buffer,
                 (path_buffer[strlen(path_buffer) - 1] == '/' ? "" : "/"),
                 entry->d_name);
        struct stat entry_st;
        int entry_is_dir = 0;
        if (stat(entry_path, &entry_st) == 0) {
#ifdef _WIN32
          entry_is_dir = (entry_st.st_mode & _S_IFDIR) != 0;
#else
          entry_is_dir = S_ISDIR(entry_st.st_mode);
#endif
        }
        snprintf(html + strlen(html), sizeof(html) - strlen(html),
                 "<li><a href=\"%s/%s\">%s%s</a></li>", uri, entry->d_name,
                 entry->d_name, entry_is_dir ? "/" : "");
      }
      closedir(dir);
    }
#endif
    strcat(html, "</ul></body></html>");
    char header[256];
    snprintf(header, sizeof(header),
             "HTTP/1.1 %s\r\nContent-Type: text/html\r\nContent-Length: "
             "%lld\r\n\r\n",
             HTTP_200, (long long)strlen(html));
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
    rendered = render_template_multi(content, templates, t_count);
  } else {
    rendered = strdup(content);
  }

  char header[256];
  snprintf(header, sizeof(header),
           "HTTP/1.1 %s\r\n"
           "Content-Type: %s\r\n"
           "Content-Length: %lld\r\n\r\n",
           HTTP_200, mime_type, (long long)strlen(rendered));

  send(client_fd, header, (int)strlen(header), 0);
  send(client_fd, rendered, (int)strlen(rendered), 0);

  free(content);
  free(rendered);
}
