#include <GLES2/gl2.h>
#include <quickjs.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>


// Shared context structure
typedef struct {
    GLuint texture_id;
    int width;
    int height;
} SharedTextureContext;

// JS: libavInstance.bindToWebGLTexture(webglContext)
static JSValue js_libav_bind_to_webgl(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    LibavInstance *dec = JS_GetOpaque(this_val, libav_class_id);
    
    // Create texture if not already allocated
    if (dec->gl_texture_id == 0) {
        glGenTextures(1, &dec->gl_texture_id);
        glBindTexture(GL_TEXTURE_2D, dec->gl_texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dec->width, dec->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    // Return the raw texture ID to the JS WebGL wrapper context
    return JS_NewInt32(ctx, dec->gl_texture_id);
}

// Optimized video frame advancement pushing directly to GPU memory
static JSValue js_libav_update_gpu_texture(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    LibavInstance *dec = JS_GetOpaque(this_val, libav_class_id);
    
    if (decode_next_frame(dec) == 0) { // standard libav read/decode frame routine
        glBindTexture(GL_TEXTURE_2D, dec->gl_texture_id);
        // Direct zero-copy texture submission to GPU memory map
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, dec->width, dec->height, GL_RGBA, GL_UNSIGNED_BYTE, dec->buffer);
        return JS_TRUE;
    }
    return JS_FALSE; // EOF reached
}

// Struct tracking decoder context instances bound to individual JS objects
typedef struct {
    AVFormatContext *fmt_ctx;
    AVCodecContext *codec_ctx;
    AVFrame *frame;
    AVFrame *frame_rgb;
    struct SwsContext *sws_ctx;
    int stream_idx;
    uint8_t *buffer;
} LibavInstance;

// JS constructor callback: new LibavDecoder(path, width, height)
static JSValue js_libav_init(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    const char *path = JS_ToCString(ctx, argv[0]);
    int target_w, target_h;
    JS_ToInt32(ctx, &target_w, argv[1]);
    JS_ToInt32(ctx, &target_h, argv[2]);

    LibavInstance *dec = malloc(sizeof(LibavInstance));
    memset(dec, 0, sizeof(LibavInstance));

    // Open video or image format via libav
    avformat_open_input(&dec->fmt_ctx, path, NULL, NULL);
    avformat_find_stream_info(dec->fmt_ctx, NULL);

    // Find the primary video/image track
    dec->stream_idx = av_find_best_stream(dec->fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    AVStream *stream = dec->fmt_ctx->streams[dec->stream_idx];
    const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
    
    dec->codec_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(dec->codec_ctx, stream->codecpar);
    avcodec_open2(dec->codec_ctx, codec, NULL);

    dec->frame = av_frame_alloc();
    dec->frame_rgb = av_frame_alloc();

    // Prepare uncompressed memory buffer for LVGL Canvas mapping (ARGB8888 match)
    int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_BGRA, target_w, target_h, 1);
    dec->buffer = (uint8_t *)av_malloc(num_bytes * sizeof(uint8_t));
    av_image_fill_arrays(dec->frame_rgb->data, dec->frame_rgb->linesize, dec->buffer, AV_PIX_FMT_BGRA, target_w, target_h, 1);

    // Initialize resolution scaler and color space translator converter context
    dec->sws_ctx = sws_getContext(dec->codec_ctx->width, dec->codec_ctx->height, dec->codec_ctx->pix_fmt,
                                  target_w, target_h, AV_PIX_FMT_BGRA, SWS_BILINEAR, NULL, NULL, NULL);

    JS_FreeCString(ctx, path);
    
    // Wrap pointer safe into a QuickJS External Object asset
    JSValue obj = JS_NewObjectClass(ctx, libav_class_id);
    JS_SetOpaque(obj, dec);
    return obj;
}

// Extraction API callback: instance.nextFrame() -> Returns pointer to raw Uint8Array buffer
static JSValue js_libav_next_frame(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    LibavInstance *dec = JS_GetOpaque(this_val, libav_class_id);
    AVPacket packet;
    int response = 0;

    // Read blocks until we decode a valid packet matrix frame
    while (av_read_frame(dec->fmt_ctx, &packet) >= 0) {
        if (packet.stream_index == dec->stream_idx) {
            response = avcodec_send_packet(dec->codec_ctx, &packet);
            response = avcodec_receive_frame(dec->codec_ctx, dec->frame);
            if (response == 0) {
                // Convert native format (YUV420p, NV12, PNG-RGB) directly into display ARGB canvas structure
                sws_scale(dec->sws_ctx, (uint8_t const * const *)dec->frame->data, dec->frame->linesize, 0,
                          dec->codec_ctx->height, dec->frame_rgb->data, dec->frame_rgb->linesize);
                av_packet_unref(&packet);
                
                // Return buffer contents wrapped into a fast JS ArrayBuffer reference view
                int target_w = dec->codec_ctx->width; // or target parameters
                int target_h = dec->codec_ctx->height;
                size_t buf_size = target_w * target_h * 4;
                return JS_NewArrayBuffer(ctx, dec->buffer, buf_size, NULL, NULL, 0);
            }
        }
        av_packet_unref(&packet);
    }
    return JS_NULL; // Returns NULL if End-Of-File or static image parsing finishes
}
