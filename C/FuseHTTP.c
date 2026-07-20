#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <curl/curl.h>

// Configuration for the virtual file
static const char *virtual_filename = "remote_file.txt";
static const char *remote_url = "https://w3.org";
static size_t remote_file_size = 0;

// Struct to track dynamic data during cURL memory writes
struct MemoryStruct {
    char *memory;
    size_t size;
};

// Callback to handle data streaming into memory from cURL
static size_t write_memory_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) return 0; // Out of memory

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

// Helper to fetch the remote file size using an HTTP HEAD request
static size_t get_remote_file_size(const char *url) {
    CURL *curl = curl_easy_init();
    if (!curl) return 0;

    double file_len = 0.0;
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // HEAD request

    if (curl_easy_perform(curl) == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &file_len);
    }
    curl_easy_cleanup(curl);
    return (size_t)file_len;
}

// 1. Get Attributes (Stat)
static int http_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    (void) fi;
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0555; // Read/execute directory
        stbuf->st_nlink = 2;
        return 0;
    } 
    
    char expected_path[128];
    snprintf(expected_path, sizeof(expected_path), "/%s", virtual_filename);
    
    if (strcmp(path, expected_path) == 0) {
        stbuf->st_mode = S_IFREG | 0444; // Read-only regular file
        stbuf->st_nlink = 1;
        stbuf->st_size = remote_file_size; // Extracted dynamically at boot
        return 0;
    }

    return -ENOENT;
}

// 2. Read Directory Listing
static int http_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
    (void) offset; (void) fi; (void) flags;

    if (strcmp(path, "/") != 0) return -ENOENT;

    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);
    filler(buf, virtual_filename, NULL, 0, 0);

    return 0;
}

// 3. Read File Bytes (Using HTTP Range Requests)
static int http_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    (void) fi;
    char expected_path[128];
    snprintf(expected_path, sizeof(expected_path), "/%s", virtual_filename);

    if (strcmp(path, expected_path) != 0) return -ENOENT;
    if (offset >= remote_file_size) return 0;
    if (offset + size > remote_file_size) size = remote_file_size - offset;

    CURL *curl = curl_easy_init();
    if (!curl) return -EIO;

    struct MemoryStruct chunk = { .memory = malloc(1), .size = 0 };
    char range_header[64];
    snprintf(range_header, sizeof(range_header), "%ld-%ld", (long)offset, (long)(offset + size - 1));

    curl_easy_setopt(curl, CURLOPT_URL, remote_url);
    curl_easy_setopt(curl, CURLOPT_RANGE, range_header);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    CURLcode res = curl_easy_perform(curl);
    long response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    curl_easy_cleanup(curl);

    int bytes_read = 0;
    if (res == CURLE_OK && (response_code == 206 || response_code == 200)) {
        bytes_read = chunk.size;
        memcpy(buf, chunk.memory, bytes_read);
    } else {
        bytes_read = -EIO;
    }

    free(chunk.memory);
    return bytes_read;
}

// Map implemented functions to FUSE operations struct
static const struct fuse_operations http_oper = {
    .getattr = http_getattr,
    .readdir = http_readdir,
    .read    = http_read,
};

int main(int argc, char *argv[]) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Fetch size of the target URL before entering the FUSE event loop
    remote_file_size = get_remote_file_size(remote_url);
    if (remote_file_size == 0) {
        fprintf(stderr, "Error: Could not retrieve target URL size.\n");
        return 1;
    }

    int fuse_stat = fuse_main(argc, argv, &http_oper, NULL);
    
    curl_global_cleanup();
    return fuse_stat;
}
