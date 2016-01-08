#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <limits.h>
#include <ctype.h>
#include <microhttpd.h>

#pragma  comment(lib, "libmicrohttpd.lib")
#pragma comment(linker, "/NODEFAULTLIB:libcmtd.lib")

/** SHA256 */
#define SHA256_BLOCK_SIZE 32 

typedef struct {
    unsigned char data[64];
    unsigned int datalen;
    unsigned long long bitlen;
    unsigned int state[8];
} SHA256_CTX;

/*********************** FUNCTION DECLARATIONS **********************/
void sha256_init(SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, const unsigned char data[], size_t len);
void sha256_final(SHA256_CTX *ctx, unsigned char hash[]);

static void
to_hexstring(unsigned char hash[SHA256_BLOCK_SIZE], char hex[SHA256_BLOCK_SIZE * 2 + 1])
{
    static const char const hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    for (auto i = 0; i < SHA256_BLOCK_SIZE; ++i) {
        const char b = hash[i];
        hex[i * 2]     = hex_chars[(b & 0xf0) >> 4];
        hex[i * 2 + 1] = hex_chars[(b & 0x0f) >> 0];
    }
}
/***********/
static void
copy_file(const char * const from, const char * const to)
{
    
}


#if defined(_WIN32)
#define snprintf(buf,len, format,...) _snprintf_s(buf, len,len, format, __VA_ARGS__)
#define strdup _strdup
#define unlink _unlink
#define fileno _fileno
#endif

#if defined(CPU_COUNT) && (CPU_COUNT+0) < 2
#undef CPU_COUNT
#endif
#if !defined(CPU_COUNT)
#define CPU_COUNT 2
#endif

#define NUMBER_OF_THREADS CPU_COUNT
#define MAGIC_HEADER_SIZE (16 * 1024)
#define FILE_NOT_FOUND_PAGE "<html><head><title>File not found</title></head><body>File not found</body></html>"
#define INTERNAL_ERROR_PAGE "<html><head><title>Internal error</title></head><body>Internal error</body></html>"
#define REQUEST_REFUSED_PAGE "<html><head><title>Request refused</title></head><body>Request refused (file exists?)</body></html>"
#define INDEX_PAGE_HEADER "<html>\n<head><title>Welcome</title></head>\n<body>\n"\
   "<h1>Upload</h1>\n"\
   "<form method=\"POST\" enctype=\"multipart/form-data\" action=\"/\">\n"\
   "<dl><dt>Content type:</dt><dd>"\
   "<input type=\"radio\" name=\"category\" value=\"books\">Book</input>"\
   "<input type=\"radio\" name=\"category\" value=\"images\">Image</input>"\
   "<input type=\"radio\" name=\"category\" value=\"music\">Music</input>"\
   "<input type=\"radio\" name=\"category\" value=\"software\">Software</input>"\
   "<input type=\"radio\" name=\"category\" value=\"videos\">Videos</input>\n"\
   "<input type=\"radio\" name=\"category\" value=\"other\" checked>Other</input></dd>"\
   "<dt>Language:</dt><dd>"\
   "<input type=\"radio\" name=\"language\" value=\"no-lang\" checked>none</input>"\
   "<input type=\"radio\" name=\"language\" value=\"en\">English</input>"\
   "<input type=\"radio\" name=\"language\" value=\"de\">German</input>"\
   "<input type=\"radio\" name=\"language\" value=\"fr\">French</input>"\
   "<input type=\"radio\" name=\"language\" value=\"es\">Spanish</input></dd>\n"\
   "<dt>File:</dt><dd>"\
   "<input type=\"file\" name=\"upload\"/></dd></dl>"\
   "<input type=\"submit\" value=\"Send!\"/>\n"\
   "</form>\n"\
   "<h1>Download</h1>\n"\
   "<ol>\n"
#define INDEX_PAGE_FOOTER "</ol>\n</body>\n</html>"


static struct MHD_Response *file_not_found_response;
static struct MHD_Response *internal_error_response;
static struct MHD_Response *cached_directory_response;
static struct MHD_Response *request_refused_response;

static void
mark_as_html(struct MHD_Response *response)
{
    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "text/html");
}

void update_directory()
{

}

struct ResponseDataContext
{
    char *buf;
    size_t buf_len;
    size_t off;
};

struct UploadContext
{
    FILE                     *fd;
    char                     *filename;
    struct MHD_PostProcessor *pp;
    struct MHD_Connection    *connection;
    struct MHD_Response      *response;
    char                      hash[SHA256_BLOCK_SIZE * 2 + 1];
    SHA256_CTX                ctx;
};


struct DownloadContext
{
    FILE   *fd;
    size_t  size;
    size_t  offset;
};

static ssize_t
fd_reader(void *cls, uint64_t pos, char *buf, size_t max)
{
    auto ctx = reinterpret_cast<DownloadContext *>(cls);
    ssize_t n;
    const auto offset64 = static_cast<int64_t>(pos + ctx->offset);

    if (offset64 < 0)
        return MHD_CONTENT_READER_END_WITH_ERROR;

#if defined(HAVE_LSEEK64)
    if (lseek64(response->fd, offset64, SEEK_SET) != offset64)
        return MHD_CONTENT_READER_END_WITH_ERROR; /* can't seek to required position */
#elif defined(HAVE___LSEEKI64)
    if (_lseeki64(response->fd, offset64, SEEK_SET) != offset64)
        return MHD_CONTENT_READER_END_WITH_ERROR; /* can't seek to required position */
#else /* !HAVE___LSEEKI64 */
    if (sizeof(off_t) < sizeof(uint64_t) && offset64 > static_cast<uint64_t>(INT32_MAX))
        return MHD_CONTENT_READER_END_WITH_ERROR; /* seek to required position is not possible */
#if defined(_WIN32)
    if (_fseeki64(ctx->fd, offset64, SEEK_SET) != 0)
        return MHD_CONTENT_READER_END_WITH_ERROR;
#else
    if (fseek(ctx->fd, static_cast<off_t>(offset64), SEEK_SET) != 0)
        return MHD_CONTENT_READER_END_WITH_ERROR;
#endif

#endif

#ifndef _WIN32
    if (max > SSIZE_MAX)
        max = SSIZE_MAX;

    n = read(response->fd, buf, max);
#else  /* _WIN32 */
    if (max > INT32_MAX)
        max = INT32_MAX;

//    memset(buf, 0, max);
    n = fread(buf, 1, max, ctx->fd);
#endif /* _WIN32 */

    if (0 == n)
        return MHD_CONTENT_READER_END_OF_STREAM;
    if (n < 0)
        return MHD_CONTENT_READER_END_WITH_ERROR;
    return n;
}


static void
fd_free(void *dc)
{
    auto ctx = reinterpret_cast<DownloadContext *>(dc);

    fclose(ctx->fd);
    ctx->fd = nullptr;
    ctx->offset = 0;
    ctx->size = 0;
    /**FIXME: Что это? */
    free(ctx);
}


struct MHD_Response *
MHD_create_response_from_fd(FILE *fd)
{
    auto dc = static_cast<DownloadContext *>(calloc(1, sizeof(DownloadContext)));
    dc->fd = fd;
    dc->offset = 0;
    fseek(fd, 0, SEEK_END);
    dc->size = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    return MHD_create_response_from_callback(dc->size, 512, &fd_reader, dc, &fd_free);
}

static int
do_append(char **ret, const char *data, size_t size)
{
    char *buf;
    size_t old_len;

    if (NULL == *ret) {
        old_len = 0;
    } else {
        old_len = strlen(*ret);
    }
    buf = static_cast<char *>(malloc(old_len + size + 1));
    if (NULL == buf)
        return MHD_NO;
    memcpy(buf, *ret, old_len);
    if (NULL != *ret)
        free(*ret);
    memcpy(&buf[old_len], data, size);
    buf[old_len + size] = '\0';
    *ret = buf;
    return MHD_YES;
}


/**
* Iterator over key-value pairs where the value
* maybe made available in increments and/or may
* not be zero-terminated.  Used for processing
* POST data.
*
* @param cls user-specified closure
* @param kind type of the value, always MHD_POSTDATA_KIND when called from MHD
* @param key 0-terminated key for the value
* @param filename name of the uploaded file, NULL if not known
* @param content_type mime-type of the data, NULL if not known
* @param transfer_encoding encoding of the data, NULL if not known
* @param data pointer to size bytes of data at the
*              specified offset
* @param off offset of data in the overall value
* @param size number of bytes in data available
* @return MHD_YES to continue iterating,
*         MHD_NO to abort the iteration
*/
static int
process_upload_data(void *cls,
    enum MHD_ValueKind kind,
    const char *key,
    const char *filename,
    const char *content_type,
    const char *transfer_encoding,
    const char *data,
    uint64_t off,
    size_t size)
{
    auto uc = reinterpret_cast<UploadContext *>(cls);
    int i;

    if (0 != strcmp(key, "upload")) {
        fprintf(stderr, "Ignoring unexpected form value `%s'\n", key);
        return MHD_YES; /* ignore */
    }
    if (nullptr == filename) {
        fprintf(stderr, "No filename, aborting upload\n");
        return MHD_NO; /* no filename, error */
    }
    if (nullptr == uc->fd) {
        char fn[1024];

        if ((nullptr != strstr(filename, "..")) ||
            (nullptr != strchr(filename, '/')) ||
            (nullptr != strchr(filename, '\\'))) {
            uc->response = request_refused_response;
            return MHD_NO;
        }
        /* create directories -- if they don't exist already */
#ifdef _WIN32
        CreateDirectory(TEXT("data"), nullptr);
#else
        (void)mkdir(uc->language, S_IRWXU);
#endif
        snprintf(fn, sizeof(fn), "data/%s", filename);
        for (i = strlen(fn) - 1; i >= 0; i--)
            if (!isprint(static_cast<int>(fn[i])))
                fn[i] = '_';
        uc->fd = fopen(fn, "w");
        if (nullptr == uc->fd) {
            fprintf(stderr,
                "Error opening file `%s' for upload: %s\n",
                fn,
                strerror(errno));
            uc->response = request_refused_response;
            return MHD_NO;
        }
        uc->filename = strdup(fn);
        sha256_init(&uc->ctx);
    }
    
    /**FIXME: Исправить, должен работать не с целым куском */
    if (0 != size) { 
        sha256_update(&uc->ctx, reinterpret_cast<unsigned char *>(const_cast<char *>(data)), size);
        if ((size != static_cast<size_t>(fwrite(data, 1, size, uc->fd)))) {
            fprintf(stderr,
                "Error writing to file `%s': %s\n",
                uc->filename,
                strerror(errno));
            uc->response = internal_error_response;
            fclose(uc->fd);
            uc->fd = nullptr;
            if (nullptr != uc->filename) {
                unlink(uc->filename);
                free(uc->filename);
                uc->filename = nullptr;
            }
            return MHD_NO;
        }
    }
    return MHD_YES;
}


static void
response_completed_callback(void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe)
{
    unsigned char hash[SHA256_BLOCK_SIZE];
    auto uc = reinterpret_cast<UploadContext *>(*con_cls);

    if (nullptr == uc)
        return; /* this request wasn't an upload request */
    if (nullptr != uc->pp) {
        MHD_destroy_post_processor(uc->pp);
        uc->pp = nullptr;
    }
    if (nullptr != uc->fd) {
        fclose(uc->fd);
        sha256_final(&uc->ctx, hash);
        memset(uc->hash, 0, sizeof(uc->hash));
        to_hexstring(hash, uc->hash);

        fprintf(stdout, "Hash: %s\n", uc->hash);
        copy_file(uc->filename, uc->hash);
        if (nullptr != uc->filename) {
            fprintf(stderr,
                "Upload of file `%s' failed (incomplete or aborted), removing file.\n",
                uc->filename);
            unlink(uc->filename);
        }
    }
    if (nullptr != uc->filename)
        free(uc->filename);
    free(uc);
}


/**
* Main callback from MHD, used to generate the page.
*
* @param cls NULL
* @param connection connection handle
* @param url requested URL
* @param method GET, PUT, POST, etc.
* @param version HTTP version
* @param upload_data data from upload (PUT/POST)
* @param upload_data_size number of bytes in "upload_data"
* @param ptr our context
* @return MHD_YES on success, MHD_NO to drop connection
*/
static int
generate_page(void *cls,
struct MHD_Connection *connection,
    const char *url,
    const char *method,
    const char *version,
    const char *upload_data,
    size_t *upload_data_size, void **ptr)
{
    struct MHD_Response *response;
    auto ret = MHD_NO;

    if (0 == strcmp(url, "/favicon.ico")) {
        if ((0 != strcmp(method, MHD_HTTP_METHOD_GET)) &&
            (0 != strcmp(method, MHD_HTTP_METHOD_HEAD)))
            return MHD_NO;  /* unexpected method (we're not polite...) */
        auto fd = fopen("favicon.png", "rb");
        if (nullptr == (response = MHD_create_response_from_fd(fd))) {
            fclose(fd);
            return MHD_NO;
        }
        (void)MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "image/png");
        ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    }

    if (0 == strcmp(method, MHD_HTTP_METHOD_POST) && 0 == strcmp(url, "/new")) {
        /* upload! */
        auto uc = reinterpret_cast<UploadContext *>(*ptr);

        if (nullptr == uc) {
            if (nullptr == (uc = static_cast<UploadContext *>(malloc(sizeof(struct UploadContext)))))
                return MHD_NO; /* out of memory, close connection */
            memset(uc, 0, sizeof(struct UploadContext));
            uc->fd = nullptr;
            uc->connection = connection;
            uc->pp = MHD_create_post_processor(connection, 64 * 1024 /* buffer size */, &process_upload_data, uc);
            if (nullptr == uc->pp) {
                /* out of memory, close connection */
                free(uc);
                return MHD_NO;
            }
            *ptr = uc;
            return MHD_YES;
        }
        if (0 != *upload_data_size) {
            if (NULL == uc->response)
                (void)MHD_post_process(uc->pp, upload_data, *upload_data_size);
            *upload_data_size = 0;
            return MHD_YES;
        }
        /* end of upload, finish it! */
        MHD_destroy_post_processor(uc->pp);
        uc->pp = nullptr;
        if (nullptr != uc->fd) {
            fclose(uc->fd);
            uc->fd = nullptr;
        }
        if (nullptr != uc->response) {
            return MHD_queue_response(connection, MHD_HTTP_FORBIDDEN, uc->response);
        }
    }
    return MHD_queue_response(connection, MHD_HTTP_FORBIDDEN, request_refused_response);
}

int
main(int argc, char *const *argv)
{
    struct MHD_Daemon *d;
    struct timeval tv;
    struct timeval *tvp;
    fd_set rs;
    fd_set ws;
    fd_set es;
    MHD_socket max;
    MHD_UNSIGNED_LONG_LONG mhd_timeout;
    unsigned int port = 9090;

    file_not_found_response = MHD_create_response_from_buffer(strlen(FILE_NOT_FOUND_PAGE),
        (void *)FILE_NOT_FOUND_PAGE,
        MHD_RESPMEM_PERSISTENT);
    mark_as_html(file_not_found_response);
    request_refused_response = MHD_create_response_from_buffer(strlen(REQUEST_REFUSED_PAGE),
        (void *)REQUEST_REFUSED_PAGE,
        MHD_RESPMEM_PERSISTENT);
    mark_as_html(request_refused_response);
    internal_error_response = MHD_create_response_from_buffer(strlen(INTERNAL_ERROR_PAGE),
        (void *)INTERNAL_ERROR_PAGE,
        MHD_RESPMEM_PERSISTENT);
    mark_as_html(internal_error_response);
    update_directory();
    d = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG
#if EPOLL_SUPPORT
        | MHD_USE_EPOLL_LINUX_ONLY
#endif
        ,
        port,
        nullptr, nullptr,
        &generate_page, nullptr,
        MHD_OPTION_CONNECTION_MEMORY_LIMIT, static_cast<size_t>(256 * 1024),
#if PRODUCTION
        MHD_OPTION_PER_IP_CONNECTION_LIMIT, (unsigned int)(64),
#endif
        MHD_OPTION_CONNECTION_TIMEOUT, static_cast<unsigned int>(120),
        MHD_OPTION_THREAD_POOL_SIZE, static_cast<unsigned int>(NUMBER_OF_THREADS),
        MHD_OPTION_NOTIFY_COMPLETED, &response_completed_callback, NULL,
        MHD_OPTION_END);
    if (nullptr == d)
        return EXIT_FAILURE;
    while (true) {
        max = 0;
        FD_ZERO(&rs);
        FD_ZERO(&ws);
        FD_ZERO(&es);
        if (MHD_YES != MHD_get_fdset(d, &rs, &ws, &es, &max))
            break; /* fatal internal error */
        if (MHD_get_timeout(d, &mhd_timeout) == MHD_YES) {
            tv.tv_sec = mhd_timeout / 1000;
            tv.tv_usec = (mhd_timeout - (tv.tv_sec * 1000)) * 1000;
            tvp = &tv;
        } else {
            tvp = nullptr;
        }
        select(max + 1, &rs, &ws, &es, tvp);
        MHD_run(d);
    }
    MHD_stop_daemon(d);
    MHD_destroy_response(file_not_found_response);
    MHD_destroy_response(request_refused_response);
    MHD_destroy_response(internal_error_response);
    return EXIT_SUCCESS;
}
