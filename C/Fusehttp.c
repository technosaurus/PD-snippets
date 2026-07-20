#define FUSE_USE_VERSION 31
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <curl/curl.h>

// Configuration: Replace with your actual remote EROFS link and its exact size in bytes
#define REMOTE_URL "http://example.com"
#define FILE_SIZE  104857600  // e.g., 100 MB (Get exact size via: curl -sI URL)
#define VIRT_FILE  "/remote_image.erofs"

// Struct to pass memory buffer directly to cURL's write callback
struct BufferState {
    char *ptr;
    size_t requested;
    size_t written;
};

// cURL callback: appends incoming HTTP bytes to our FUSE read buffer
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct BufferState *mem = (struct BufferState *)userp;

    if (mem->written + realsize > mem->requested) {
        realsize = mem->requested - mem->written; // Clip if server sends extra
    }
    
    memcpy(&(mem->ptr[mem->written]), contents, realsize);
    mem->written += realsize;
    return size * nmemb;
}

// FUSE getattr: Defines the virtual file's permissions and size
static int http_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    (void) fi;
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0555; // Read/Execute Directory
        stbuf->st_nlink = 2;
        return 0;
    } else if (strcmp(path, VIRT_FILE) == 0) {
        stbuf->st_mode = S_IFREG | 0444; // Read-Only Regular File
        stbuf->st_nlink = 1;
        stbuf->st_size = FILE_SIZE;     // Critical for EROFS to calculate boundaries
        return 0;
    }
    return -ENOENT;
}

// FUSE readdir: Lists the single virtual file when opening the directory
static int http_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
    (void) offset; (void) fi; (void) flags;

    if (strcmp(path, "/") != 0) return -ENOENT;

    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);
    filler(buf, VIRT_FILE + 1, NULL, 0, 0); // Strip leading slash to show "remote_image.erofs"

    return 0;
}

// FUSE read: Triggers on random chunk reads. Fetches precise byte ranges.
static int http_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    (void) fi;
    if (strcmp(path, VIRT_FILE) != 0) return -ENOENT;
    if (offset >= FILE_SIZE) return 0;
    if (offset + size > FILE_SIZE) size = FILE_SIZE - offset;

    CURL *curl_handle = curl_easy_init();
    if (!curl_handle) return -EIO;

    char range_header[64];
    snprintf(range_header, sizeof(range_header), "%lld-%lld", (long long)offset, (long long)(offset + size - 1));

    struct BufferState chunk = { .ptr = buf, .requested = size, .written = 0 };

    curl_easy_setopt(curl_handle, CURLOPT_URL, REMOTE_URL);
    curl_easy_setopt(curl_handle, CURLOPT_RANGE, range_header); // HTTP Range Request
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    
    // Fail explicitly if HTTP status code >= 400
    curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1L); 

    CURLcode res = curl_easy_perform(curl_handle);
    curl_easy_cleanup(curl_handle);

    if (res != CURLE_OK) {
        fprintf(stderr, "cURL range request failed: %s\n", curl_easy_strerror(res));
        return -EIO;
    }

    return chunk.written; // Returns exact bytes read back to the FUSE driver
}

// Map FUSE actions to our HTTP functions
static const struct fuse_operations http_oper = {
    .getattr = http_getattr,
    .readdir = http_readdir,
    .read    = http_read,
};

int main(int argc, char *argv[]) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    int fuse_stat = fuse_main(argc, argv, &http_oper, NULL);
    curl_global_cleanup();
    return fuse_stat;
}
