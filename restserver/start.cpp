#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <limits.h>
#include <ctype.h>
#include <mongoose.h>
#include <json.hpp>

#if defined(_WIN32)
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "libtesseract302.lib")
#pragma comment(lib, "liblept168.lib")
#endif

/** SHA256 */
#define SHA256_BLOCK_SIZE 32 

typedef struct
{
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
        hex[i * 2] = hex_chars[(b & 0xf0) >> 4];
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
    char       mime[256];
};

struct FileObject
{
    std::string hash;
    std::string mime;
    std::string content;
    std::string created_at;

    std::string json;
};

char* 
time_to_string(const struct tm *timeptr)
{
    static char result[40];
    sprintf(result, "%.2d:%.2d:%.2d %.2d.%.2d.%.4d",
        timeptr->tm_hour,
        timeptr->tm_min, 
        timeptr->tm_sec,
        timeptr->tm_mday,
        timeptr->tm_mon + 1,
        1900 + timeptr->tm_year);
    return result;
}

void
recognizer_push(const char * const hash);
void
recognizer_start();

static void
load_file_object(const char * const hash, struct FileObject *fo)
{
    *fo = {};
    std::string filename(hash);
    FILE *fd;
    filename += ".json";
    fd = fopen(filename.c_str(), "r");
    if (fd != nullptr) {
        char        buffer[256];
        std::string content;
        int         ret;

        while ((ret = fread(buffer, 1, sizeof(buffer), fd)) > 0) {
            content += std::string(buffer, ret);
        }
        fclose(fd);

        auto jsonObject = nlohmann::json::parse(content);
        fo->hash = jsonObject["filename"].get<std::string>();
        fo->mime = jsonObject["content_type"].get<std::string>();
        if (!jsonObject["content"].is_null()) {
            fo->content = jsonObject["content"].get<std::string>();
        }
        fo->created_at = jsonObject["created_at"].get<std::string>();
        fo->json = content;
    }
}

static std::string
to_json(const char * const hash, const char * const mime, const char * const content = nullptr, bool store = true)
{
    time_t     rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    nlohmann::json jsonObject = {
        { "filename", hash },
        { "content_type", mime },
        { "created_at", time_to_string(timeinfo) }
    };
    if (content != nullptr)
        jsonObject["content"] = content;
    auto simplify = jsonObject.dump(2);
    if (store) {
        std::string filename(hash);
        FILE *fd;
        filename += ".json";
        fd = fopen(filename.c_str(), "w");
        if (fd != nullptr) {
            fwrite(simplify.c_str(), 1, simplify.size(), fd);
            fclose(fd);
        }
    }
    return simplify;
}

static void
save_object(FileObject &object)
{
    nlohmann::json jsonObject = {
        { "filename", object.hash },
        { "content_type", object.mime },
        { "created_at", object.created_at },
        { "content", object.content },
    };
    std::string filename(object.hash);
    FILE *fd;
    filename += ".json";
    fd = fopen(filename.c_str(), "w");
    auto simplify = jsonObject.dump(2);
    if (fd != nullptr) {
        fwrite(simplify.c_str(), 1, simplify.size(), fd);
        fclose(fd);
    }
}

void
save_content(const char * const hash, const std::string &content)
{
    FileObject object;

    load_file_object(hash, &object);
    object.content = content;
    save_object(object);
}



static void handle_request(struct mg_connection *nc)
{

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

    if (data == nullptr) {
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

            if (hm.body.len == static_cast<size_t>(~0) || hm.body.len == 0) {
                mg_printf(nc, "%s",
                    "HTTP/1.1 411 Content-Length required\r\n"
                    "Content-Length: 0\r\n\r\n");
                nc->flags |= MG_F_SEND_AND_CLOSE;
                return;
            }

            // Reset proto_handler so Mongoose's http_handler() won't get confused
            // once we start removing data from the buffer.
            nc->proto_handler = nullptr;

            // Headers will be inaccessible later, so put everything we need into
            // user_data.
            data = static_cast<struct FileWriterData *>(calloc(1, sizeof(struct FileWriterData)));
            {
                for (auto i = 0; i < MG_MAX_HTTP_HEADERS && hm.header_names[i].p != nullptr; ++i) {
                    auto n = hm.header_names[i];
                    if (!strnicmp(n.p, "Content-Type", n.len)) {
                        n = hm.header_values[i];
                        auto it = n.p;
                        auto len = n.len - (it - n.p);
                        while (it && *it == ' ' && (it - n.p) < n.len)
                            ++it;
                        auto start = it;
                        it = n.p + n.len - 1;
                        while (it && (*it == '\r' || *it == '\n' || *it == ' ') && it > n.p)
                            --it, --len;
                        strncpy(data->mime, start, len);
                    }
                }
            }
            data->bytes_left = hm.body.len;
            data->fd = tmpfile();
            sha256_init(&data->ctx);
            if (data->fd == nullptr) {
                mg_printf(nc, "%s",
                    "HTTP/1.1 500 Failed to open a file\r\n"
                    "Content-Length: 0\r\n\r\n");
                nc->flags |= MG_F_SEND_AND_CLOSE;
                return;
            }
            nc->user_data = static_cast<void *>(data);

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
            FileObject object;
            std::string json;
            long        size;
            unsigned char digest[SHA256_BLOCK_SIZE];

            // Request is complete, do something meaningful here.
            sha256_final(&data->ctx, digest);
            to_hexstring(digest, data->hash);
            load_file_object(data->hash, &object);
            {
                size = ftell(data->fd);
                fseek(data->fd, 0, SEEK_SET);
                auto fd = fopen(data->hash, "wb");
                if (fd) {
                    char buffer[256];
                    int  readed;
                    while ((readed = fread(buffer, 1, sizeof(buffer), data->fd)) > 0) {
                        fwrite(buffer, 1, readed, fd);
                    }
                    fclose(fd);
                }
            }
            if (!object.hash.empty()) {
                fprintf(stdout, "%s already stored at %s\n", data->hash, object.created_at.c_str());
                json = object.json;
            } else {
                json = to_json(data->hash, data->mime);
                recognizer_push(data->hash);
            }
            mg_printf(nc,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json; charset=utf-8\r\n"
                "Connection: close\r\n\r\n"
                "%s",
                json.c_str());
            nc->flags |= MG_F_SEND_AND_CLOSE;
            // handle_close will free the resources.
        }
    }
}

// Make sure we free all allocated resources
static void handle_close(struct mg_connection *nc)
{
    auto data = static_cast<struct FileWriterData *>(nc->user_data);

    if (data != nullptr) {
        
        fclose(data->fd);
        free(data);
        nc->user_data = nullptr;
    }
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data)
{
    (void)ev_data;
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

    mg_mgr_init(&mgr, nullptr);
    nc = mg_bind(&mgr, s_http_port, ev_handler);

    // Set up HTTP server parameters
    mg_set_protocol_http_websocket(nc);
    mg_enable_multithreading(nc);

    printf("Starting web server on port %s\n", s_http_port);
    recognizer_start();
    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }
    mg_mgr_free(&mgr);

    return EXIT_SUCCESS;
}
