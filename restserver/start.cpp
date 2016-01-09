#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <limits.h>
#include <ctype.h>
#include <mongoose.h>

#pragma comment(lib, "ws2_32.lib")

/** SHA256 */
#define SHA256_BLOCK_SIZE 32 

typedef struct {
    unsigned char data[64];
    unsigned int datalen;
    unsigned long long bitlen;
    unsigned int state[8];
} SHA256_CTX;

/*********************** FUNCTION DECLARATIONS **********************/
extern "C" {
    void sha256_init(SHA256_CTX *ctx);
    void sha256_update(SHA256_CTX *ctx, const unsigned char data[], size_t len);
    void sha256_final(SHA256_CTX *ctx, unsigned char hash[]);
}

static void
to_hexstring(unsigned char hash[SHA256_BLOCK_SIZE], char hex[SHA256_BLOCK_SIZE * 2 + 1])
{
    static const char hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    for (auto i = 0; i < SHA256_BLOCK_SIZE; ++i) {
        const char b = hash[i];
        hex[i * 2]     = hex_chars[(b & 0xf0) >> 4];
        hex[i * 2 + 1] = hex_chars[(b & 0x0f) >> 0];
    }
}
/***********/
static const char *s_http_port = "9090";

struct FileWriterData 
{
  FILE      *fd;
  size_t     bytes_left;
  SHA256_CTX ctx;
  char       hash[SHA256_BLOCK_SIZE * 2 + 1];
};

static void handle_request(struct mg_connection *nc) 
{
  // This handler gets called only when we don't reset proto_handler in
  // handle_recv, so we just return a page unconditionally.
  mg_printf(nc, "%s",
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<html><body>Upload example."
            "<form method=\"POST\" action=\"/upload\" "
            "  enctype=\"multipart/form-data\">"
            "<input type=\"file\" name=\"file\" /> <br/>"
            "<input type=\"submit\" value=\"Upload\" />"
            "</form></body></html>");
  nc->flags |= MG_F_SEND_AND_CLOSE;
}

static void handle_recv(struct mg_connection *nc) 
{
    auto data = static_cast<struct FileWriterData *>(nc->user_data);

  if (data == NULL) {
    // This is a new connection, try to parse HTTP request.
    struct http_message hm;
    int req_len = mg_parse_http(nc->recv_mbuf.buf, nc->recv_mbuf.len, &hm,
                                1 /* is_req */);

    if (req_len < 0 ||
        (req_len == 0 && nc->recv_mbuf.len >= MG_MAX_HTTP_REQUEST_SIZE)) {
      nc->flags |= MG_F_CLOSE_IMMEDIATELY;
    } else if (req_len == 0) {
      // Request is not complete yet, do nothing.
    } else if (mg_vcasecmp(&hm.method, "POST") == 0 &&
               mg_vcmp(&hm.uri, "/new") == 0) {
      // This is the request that we don't want to buffer in memory.

      if (hm.body.len == (size_t) ~0 || hm.body.len == 0) {
        mg_printf(nc, "%s",
                  "HTTP/1.1 411 Content-Length required\r\n"
                  "Content-Length: 0\r\n\r\n");
        nc->flags |= MG_F_SEND_AND_CLOSE;
        return;
      }

      // Reset proto_handler so Mongoose's http_handler() won't get confused
      // once we start removing data from the buffer.
      nc->proto_handler = NULL;

      // Headers will be inaccessible later, so put everything we need into
      // user_data.
      data = static_cast<struct FileWriterData *>(calloc(1, sizeof(struct FileWriterData)));
      data->bytes_left = hm.body.len;
      data->fd = tmpfile();
      sha256_init(&data->ctx);
      if (data->fd == NULL) {
        mg_printf(nc, "%s",
                  "HTTP/1.1 500 Failed to open a file\r\n"
                  "Content-Length: 0\r\n\r\n");
        nc->flags |= MG_F_SEND_AND_CLOSE;
        return;
      }
      nc->user_data = (void *) data;

      // Remove the headers from the buffer.
      mbuf_remove(&nc->recv_mbuf, hm.body.p - nc->recv_mbuf.buf);

      // Invoke itself again to write the piece of the body that is already in
      // the buffer.
      handle_recv(nc);
    }
  } else {  // data != NULL
    size_t to_write = data->bytes_left, written = 0;
    if (nc->recv_mbuf.len < to_write) 
        to_write = nc->recv_mbuf.len;
    written = fwrite(nc->recv_mbuf.buf, 1, to_write, data->fd);
    sha256_update(&data->ctx, reinterpret_cast<unsigned char * const>(nc->recv_mbuf.buf), written);
    mbuf_remove(&nc->recv_mbuf, written);
    data->bytes_left -= written;
    if (data->bytes_left <= 0) {
        unsigned char digest[SHA256_BLOCK_SIZE];
        
      // Request is complete, do something meaningful here.
        sha256_final(&data->ctx, digest);
        to_hexstring(digest, data->hash);
      mg_printf(nc,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Connection: close\r\n\r\n"
                "Written %ld of POST data to a temp file\n%s\n",
                (long) ftell(data->fd), data->hash);
      nc->flags |= MG_F_SEND_AND_CLOSE;
      // handle_close will free the resources.
    }
  }
}

// Make sure we free all allocated resources
static void handle_close(struct mg_connection *nc)
{
  auto data = static_cast<struct FileWriterData *>(nc->user_data);

  if (data != NULL) {
      auto size = ftell(data->fd);
      fseek(data->fd, 0, SEEK_SET);
      {
          FILE *fd = fopen(data->hash, "wb");
          if (fd) {
              char buffer[256];
              int  readed;
              while ((readed = fread(buffer, 1, sizeof(buffer), data->fd)) > 0) {
                  fwrite(buffer, 1, readed, fd);
              }
              fclose(fd);
          }
      }
    fclose(data->fd);
    free(data);
    nc->user_data = NULL;
  }
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  (void) ev_data;
  switch (ev) {
    case MG_EV_HTTP_REQUEST:
      // Invoked when the full HTTP request is in the buffer (including body).
      handle_request(nc);
      break;
    case MG_EV_RECV:
      // Invoked every time new data arrives.
      handle_recv(nc);
      break;
    case MG_EV_CLOSE:
      handle_close(nc);
      break;
  }
}

int main(void) 
{
  struct mg_mgr mgr;
  struct mg_connection *nc;

  mg_mgr_init(&mgr, NULL);
  nc = mg_bind(&mgr, s_http_port, ev_handler);

  // Set up HTTP server parameters
  mg_set_protocol_http_websocket(nc);
  mg_enable_multithreading(nc);

  printf("Starting web server on port %s\n", s_http_port);
  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }
  mg_mgr_free(&mgr);

  return 0;
}
