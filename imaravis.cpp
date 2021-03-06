#include "imaravis.h"

#include <arv.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

static char *arv_option_camera_name = NULL;
static char *arv_option_debug_domains = NULL;
//static bool arv_option_snaphot = false;
static char *arv_option_trigger = NULL;
static double arv_option_software_trigger = -1;
static double arv_option_frequency = -1.0;
static int arv_option_width = -1;
static int arv_option_height = -1;
static int arv_option_horizontal_binning = -1;
static int arv_option_vertical_binning = -1;
static double arv_option_exposure_time_us = -1;
static int arv_option_gain = -1;
static bool arv_option_auto_socket_buffer = false;
static bool arv_option_no_packet_resend = false;
static double arv_option_packet_request_ratio = -1.0;
static unsigned int arv_option_packet_timeout = 20;
static unsigned int arv_option_frame_retention = 100;
static int arv_option_gv_stream_channel = -1;
static int arv_option_gv_packet_delay = -1;
static int arv_option_gv_packet_size = -1;
static bool arv_option_realtime = false;
static bool arv_option_high_priority = false;
static bool arv_option_no_packet_socket = false;
static char *arv_option_chunks = NULL;
//static unsigned int arv_option_bandwidth_limit = -1;



//typedef struct {
//	GMainLoop *main_loop;

//	int buffer_count;
//	int error_count;
//	size_t transferred;

//	ArvChunkParser *chunk_parser;
//	char **chunks;
//} ApplicationData;

//static bool cancel = false;

//static void set_cancel (int signal)
//{
//	cancel = true;
//}

// from https://github.com/kbinani/colormap-shaders
// Matlab JET colormap
struct vec4
{
    vec4(float a0, float a1, float a2, float a3)
        : x(a0)
        , y(a1)
        , z(a2)
        , w(a3)
    {
    }

    union {
        double r;
        double x;
    };
    union {
        double g;
        double y;
    };
    union {
        double b;
        double z;
    };
    union {
        double a;
        double w;
    };
};

float colormap_red(float x) {
    if (x < 0.7) {
        return 4.0 * x - 1.5;
    } else {
        return -4.0 * x + 4.5;
    }
}

float colormap_green(float x) {
    if (x < 0.5) {
        return 4.0 * x - 0.5;
    } else {
        return -4.0 * x + 3.5;
    }
}

float colormap_blue(float x) {
    if (x < 0.3) {
       return 4.0 * x + 0.5;
    } else {
       return -4.0 * x + 2.5;
    }
}

float clamp(float v, float min, float max) {
    if (v < min) {
        return min;
    } else if (max < v) {
        return max;
    } else {
        return v;
    }
}

vec4 colormap(float x) {
    float r = clamp(colormap_red(x), 0.0, 1.0);
    float g = clamp(colormap_green(x), 0.0, 1.0);
    float b = clamp(colormap_blue(x), 0.0, 1.0);
    return vec4(r, g, b, 1.0);
}

// Matlab JET colormap


imAravis::imAravis() {
    imageUpdated = false;
    imageData = NULL;
    imageSize = 0;

    initColorMap();

    bool ret = initialize();
    assert(ret == true);
}

imAravis::~imAravis() {
    destroy();
}

void imAravis::initColorMap(void) {
    float v;
    for (int i = 0; i < 256; i++) {
        v = (float)i / 255.0;
        // make JET colomap
        colorMap[i][0] = (unsigned char)(clamp(colormap_red(v), 0.0, 1.0) * 255.0);
        colorMap[i][1] = (unsigned char)(clamp(colormap_green(v), 0.0, 1.0) * 255.0);
        colorMap[i][2] = (unsigned char)(clamp(colormap_blue(v), 0.0, 1.0) * 255.0);
        colorMap[i][3] = 255;
        // fprintf(stderr, "RGBA %3d %3d %3d %3d\n", colorMap[i][0], colorMap[i][1], colorMap[i][2], colorMap[i][3]);
    }
}

// Called by aravis when a new buffer is produced
void imAravis::new_buffer_cb(ArvStream *_stream, void *_arg)
{
	ArvBuffer *buffer;

    imAravis *me = (imAravis *)_arg;

	buffer = arv_stream_try_pop_buffer(_stream);
    assert(buffer != NULL);
    if (buffer == NULL) {
        return;
    }
    if (arv_buffer_get_status(buffer) != ARV_BUFFER_STATUS_SUCCESS) {
        me->errorCount++;
        fprintf(stderr, "buffer status: %d\n", arv_buffer_get_status(buffer));
        return;
    }

    // buffer contains our image

    int imageWidth = arv_buffer_get_image_width(buffer);
    assert(imageWidth > 0);
    int imageHeight = arv_buffer_get_image_height(buffer);
    assert(imageHeight > 0);
    size_t size = 0;
    const void *raw = arv_buffer_get_data(buffer, &size);
    assert(raw != NULL);
    int pixelFormat = arv_buffer_get_image_pixel_format(buffer);
    // fprintf(stderr, "raw image %lu bytes, pixel format %08X\n", payload, pixelFormat);

    int imageDepth = 0;
    switch (pixelFormat) {
    case ARV_PIXEL_FORMAT_MONO_16:
        // fprintf(stderr, "pixel format ARV_PIXEL_FORMAT_MONO_16\n");
        imageDepth = 2;
        break;
    case ARV_PIXEL_FORMAT_MONO_8:
        // fprintf(stderr, "pixel format ARV_PIXEL_FORMAT_MONO_8\n");
        imageDepth = 1;
        break;
    default:
        fprintf(stderr, "unhandled pixel format 0x%X\n", pixelFormat);
        break;
    }
    assert(imageDepth != 0);
    // fprintf(stderr, "RGB image %lu bytes, pixel format RGB\n", size);

    //struct timeval tv;
    //gettimeofday(&tv, NULL);
    //char filename[256];
    //sprintf(filename, "%ld_%ld.dat", tv.tv_sec, tv.tv_usec);
    //int fd = open(filename, O_WRONLY | O_CREAT, 0666);
    //size_t ret = write(fd, raw, size);
    //g_assert(size == ret);
    //fprintf(stderr, "wrote %lu bytes to file %s\n", ret, filename);
    //close(fd);

    if (me->imageData == NULL) {
        // allocate room for a RGB image, aravis provided payload data may be
        // in other pixel formats (8 bit, 10 bit, 12 bit, 16 bit,..)
        me->imageData = malloc(size);
    }
    // do we need to reallocate the RGB buffer?
    if (me->imageSize < size) {
        me->imageData = realloc(me->imageData, size);
    }
    me->imageSize = size;

    assert(me->imageData != NULL);
    assert(me->imageSize > 0);

    memcpy(me->imageData, raw, size);
    me->imageWidth = imageWidth;
    me->imageHeight = imageHeight;
    me->bufferCount++;
    me->transferred += size;

    // main loop will pick the new frame data
    me->imageUpdated = true;

    // always return the buffer to the stream
    arv_stream_push_buffer(_stream, buffer);
}

void imAravis::stream_cb(void *user_data, ArvStreamCallbackType type, ArvBuffer *buffer)
{
    (void)user_data;
    (void)type;
    (void)buffer;

	if (type == ARV_STREAM_CALLBACK_TYPE_INIT) {
		if (arv_option_realtime) {
			if (!arv_make_thread_realtime(10))
				fprintf(stderr, "Failed to make stream thread realtime\n");
		} else if (arv_option_high_priority) {
			if (!arv_make_thread_high_priority(-10))
				fprintf(stderr, "Failed to make stream thread high priority\n");
		}
	}
}

bool imAravis::periodic_task_cb(void)
{
	fprintf(stderr, "%3d frame%s - %7.3g MiB/s",
		bufferCount,
		bufferCount > 1 ? "s/s" : "/s ",
		(double) transferred / 1e6);
	if (errorCount > 0)
		fprintf(stderr, " - %d error%s\n", errorCount, errorCount > 1 ? "s" : "");
	else
		fprintf(stderr, "\n");
	bufferCount = 0;
	errorCount = 0;
	transferred = 0;

//	if (cancel) {
//		g_main_loop_quit (data->main_loop);
//		return false;
//	}

	return true;
}

bool imAravis::emit_software_trigger(void)
{
//	ArvCamera *camera = (ArvCamera *)abstract_data;

	arv_camera_software_trigger(camera, NULL);

	return true;
}

void imAravis::control_lost_cb(ArvGvDevice *gv_device)
{
    (void)gv_device;
	fprintf(stderr, "Control lost\n");

    // XXX: handle this more gracefully!
    assert(1 == 0);

//	cancel = true;
}


bool imAravis::initialize(void) {
	GError *error = NULL;
	int i;

	bufferCount = 0;
	errorCount = 0;
	transferred = 0;

	arv_enable_interface("Fake");

	arv_debug_enable(arv_option_debug_domains);

	if (arv_option_camera_name == NULL)
		fprintf(stderr, "Looking for the first available camera\n");
	else
		fprintf(stderr, "Looking for camera '%s'\n", arv_option_camera_name);

	camera = arv_camera_new(arv_option_camera_name, &error);
	if (! camera) {
        fprintf(stderr, "No camera found%s%s\n",
			error != NULL ? ": " : "",
			error != NULL ? error->message : "");
		g_clear_error(&error);
        return false;
    }

    gint payload;
    gint x, y;
    gint dx, dy;
    double exposure;
    int gain;

    arv_camera_set_chunks(camera, arv_option_chunks, NULL);
    arv_camera_set_region(camera, 0, 0, arv_option_width, arv_option_height, NULL);
    arv_camera_set_binning(camera, arv_option_horizontal_binning, arv_option_vertical_binning, NULL);
    arv_camera_set_exposure_time(camera, arv_option_exposure_time_us, NULL);
    arv_camera_set_gain(camera, arv_option_gain, NULL);

    if (arv_camera_is_gv_device(camera)) {
        arv_camera_gv_select_stream_channel(camera, arv_option_gv_stream_channel, NULL);
        arv_camera_gv_set_packet_delay(camera, arv_option_gv_packet_delay, NULL);
        arv_camera_gv_set_packet_size(camera, arv_option_gv_packet_size, NULL);
        arv_camera_gv_set_stream_options(camera, arv_option_no_packet_socket ?
                          ARV_GV_STREAM_OPTION_PACKET_SOCKET_DISABLED :
                          ARV_GV_STREAM_OPTION_NONE);
    }

    int width, height;
    arv_camera_get_region(camera, &x, &y, &width, &height, NULL);
    arv_camera_get_binning(camera, &dx, &dy, NULL);
    exposure = arv_camera_get_exposure_time(camera, NULL);
    payload = arv_camera_get_payload(camera, NULL);
    gain = arv_camera_get_gain(camera, NULL);

    vendor = strdup(arv_camera_get_vendor_name(camera, NULL));
    model = strdup(arv_camera_get_model_name(camera, NULL));
    device = strdup(arv_camera_get_device_id(camera, NULL));

    // will allocate when image is received
    imageData = NULL;
    imageSize = 0;

    fprintf(stderr, "vendor name           = %s\n", vendor);
    fprintf(stderr, "model name            = %s\n", model);
    fprintf(stderr, "device id             = %s\n", device);
    fprintf(stderr, "image width           = %d\n", width);
    fprintf(stderr, "image height          = %d\n", height);
    fprintf(stderr, "horizontal binning    = %d\n", dx);
    fprintf(stderr, "vertical binning      = %d\n", dy);
    fprintf(stderr, "payload               = %d bytes\n", payload);
    fprintf(stderr, "exposure              = %g µs\n", exposure);
    fprintf(stderr, "gain                  = %d dB\n", gain);

    if (arv_camera_is_gv_device(camera)) {
        fprintf(stderr, "gv n_stream channels  = %d\n", arv_camera_gv_get_n_stream_channels(camera, NULL));
        fprintf(stderr, "gv current channel    = %d\n", arv_camera_gv_get_current_stream_channel(camera, NULL));
        fprintf(stderr, "gv packet delay       = %" G_GINT64_FORMAT " ns\n", arv_camera_gv_get_packet_delay(camera, NULL));
        fprintf(stderr, "gv packet size        = %d bytes\n", arv_camera_gv_get_packet_size(camera, NULL));
    }

    stream = arv_camera_create_stream(camera, imAravis::stream_cb, NULL, &error);
    if (ARV_IS_STREAM(stream)) {
        if (ARV_IS_GV_STREAM(stream)) {
            if (arv_option_auto_socket_buffer)
                g_object_set(stream,
                          "socket-buffer", ARV_GV_STREAM_SOCKET_BUFFER_AUTO,
                          "socket-buffer-size", 0,
                          NULL);
            if (arv_option_no_packet_resend)
                g_object_set(stream,
                          "packet-resend", ARV_GV_STREAM_PACKET_RESEND_NEVER,
                          NULL);
            if (arv_option_packet_request_ratio >= 0.0)
                g_object_set(stream,
                          "packet-request-ratio", arv_option_packet_request_ratio,
                          NULL);

            g_object_set(stream,
                      "packet-timeout", (unsigned) arv_option_packet_timeout * 1000,
                      "frame-retention", (unsigned) arv_option_frame_retention * 1000,
                      NULL);
        }

        // creation of new frame buffers should be moved just before starting the acquisition
        // the payload size might be different due to HW ROI..
        for (i = 0; i < 50; i++)
            arv_stream_push_buffer(stream, arv_buffer_new(payload, NULL));
        // should be moved to just before starting the acquisition
        arv_camera_set_acquisition_mode(camera, ARV_ACQUISITION_MODE_CONTINUOUS, NULL);

        if (arv_option_frequency > 0.0)
            arv_camera_set_frame_rate(camera, arv_option_frequency, NULL);

        if (arv_option_trigger != NULL)
            arv_camera_set_trigger(camera, arv_option_trigger, NULL);

        if (arv_option_software_trigger > 0.0) {
            arv_camera_set_trigger(camera, "Software", NULL);
//            software_trigger_source = g_timeout_add((double) (0.5 + 1000.0 /
//                                       arv_option_software_trigger),
//                                 emit_software_trigger, camera);
        }

        arv_camera_stop_acquisition(camera, NULL);
        acquiring = false;

        g_signal_connect(stream, "new-buffer", G_CALLBACK(imAravis::new_buffer_cb), this);
        arv_stream_set_emit_signals(stream, true);

        g_signal_connect(arv_camera_get_device(camera), "control-lost",
                  G_CALLBACK(imAravis::control_lost_cb), NULL);
    }

    return true;
}

void imAravis::destroy(void) {
//    if (software_trigger_source > 0)
//        g_source_remove(software_trigger_source);

    if (! ARV_IS_CAMERA(camera)) {
        return;
    }

    if (ARV_IS_STREAM(stream)) {
        arv_stream_set_emit_signals(stream, false);
    }

    arv_camera_stop_acquisition(camera, NULL);

    guint64 n_completed_buffers;
    guint64 n_failures;
    guint64 n_underruns;
    arv_stream_get_statistics(stream, &n_completed_buffers, &n_failures, &n_underruns);
    fprintf(stderr, "Completed buffers = %llu\n", (unsigned long long) n_completed_buffers);
    fprintf(stderr, "Failures          = %llu\n", (unsigned long long) n_failures);
    fprintf(stderr, "Underruns         = %llu\n", (unsigned long long) n_underruns);

    g_clear_object(&stream);
    g_clear_object(&camera);

    free(model);
    free(vendor);
    free(device);

    arv_shutdown();
}
