#include "debug.h"
#include "camera.h"

#include <assert.h>
#include <stdlib.h>

Camera::Camera(const unsigned int _index) {
    D("\n");

    index = _index;

    protocol = strdup(arv_get_device_protocol(index));
    deviceId = strdup(arv_get_device_id(index));
    vendor = strdup(arv_get_device_vendor(index));
    model = strdup(arv_get_device_model(index));
    serialNumber = strdup(arv_get_device_serial_nbr(index));
    physicalId = strdup(arv_get_device_physical_id(index));
    D("camera at index %d\n", index);
    D(".. protocol  %s\n", protocol);
    D(".. device    %s\n", deviceId);
    D(".. vendor    %s\n", vendor);
    D(".. model     %s\n", model);
    D(".. serial #  %s\n", serialNumber);
    D(".. phys. ID  %s\n", physicalId);

    camera = NULL;
    stream = NULL;
    connected = false;
    autoSocketBuffer = false;
    packetResend = true;
    // in milli seconds
    packetTimeout = 20;
    frameRetention = 100;
    packetRequestRatio = -1.0;

    // image stats
    numImages = 0;
    numBytes = 0;
    numErrors = 0;
    // current image
    imageData = NULL;
    imageUpdate = false;
    imagePayload = 0;
    imageSize = 0;
    imageWidth = 0;
    imageHeight = 0;
}

Camera::~Camera(void) {
    D("\n");

    stop();

    free(protocol);
    free(deviceId);
    free(vendor);
    free(model);
    free(serialNumber);
    free(physicalId);
}

void Camera::start(void) {
    D("camera name %s\n", deviceId);

    assert(camera == NULL);

    GError *error = NULL;
    camera = arv_camera_new(deviceId, &error);
    if (! ARV_IS_CAMERA(camera)) {
        E("arv_camera_new() failed : %s\n", (error != NULL) ? error->message : "");
		g_clear_error(&error);
        return;
    }

	arv_device_set_register_cache_policy(arv_camera_get_device(camera), cachePolicy);

    arv_camera_set_chunk_mode(camera, FALSE, &error);
    if ((error != NULL) && (error->code != ARV_DEVICE_ERROR_FEATURE_NOT_FOUND)  ) {
        E("arv_camera_set_chunk_mode() failed : %s\n", (error != NULL) ? error->message : "???");
		g_clear_error(&error);
        return;
    }
    // arv_camera_set_chunk_mode() might have set the error if the feature is missing
    g_clear_error(&error);

    if (! infoQuery()) {
        stop();
        return;
    }

    g_signal_connect(arv_camera_get_device(camera), "control-lost", G_CALLBACK(Camera::controlLostCallback), this);

    connected = true;
}

void Camera::stop(void) {
    D("camera name %s\n", deviceId);

    if (! ARV_IS_CAMERA(camera)) {
        return;
    }

    g_free(pixelFormatStrings);
    g_free(pixelFormats);
    numPixelFormats = 0;

    connected = false;
    stopVideo();

    g_clear_object(&camera);
}

void Camera::startVideo(void) {
    D("camera name %s\n", deviceId);

    if (! ARV_IS_CAMERA(camera)){
        E("camera not connected %s\n", deviceId);
        return;
    }
    assert(camera != NULL);

    GError *error = NULL;
    stream = arv_camera_create_stream(camera, Camera::streamCallback, NULL, &error);
	if (! ARV_IS_STREAM(stream)) {
        E("arv_camera_create_stream() failed : %s\n", (error != NULL) ? error->message : "???");
        assert(error == NULL);
		g_clear_error(&error);
        // XXX do this elsewhere?!
        //		g_object_unref(camera);
        //		camera = NULL;
		return;
	}

    if (ARV_IS_GV_STREAM(stream)) {
		if (autoSocketBuffer)
			g_object_set(stream,
				      "socket-buffer", ARV_GV_STREAM_SOCKET_BUFFER_AUTO,
				      "socket-buffer-size", 0,
				      NULL);
		if (! packetResend)
			g_object_set(stream,
				      "packet-resend", ARV_GV_STREAM_PACKET_RESEND_NEVER,
				      NULL);
        if (packetRequestRatio >= 0.0)
            g_object_set(stream,
                      "packet-request-ratio", packetRequestRatio,
                      NULL);
		g_object_set(stream,
			      "packet-timeout", (unsigned) packetTimeout * 1000,
			      "frame-retention", (unsigned) frameRetention * 1000,
			      NULL);
	}

	arv_stream_set_emit_signals(stream, TRUE);
    unsigned int payload;
	payload = arv_camera_get_payload(camera, &error);
    assert(error == NULL);
    for (unsigned int i = 0; i < 5; i++) {
        arv_stream_push_buffer(stream, arv_buffer_new(payload, NULL));
    }

    pixelFormatString = arv_camera_get_pixel_format_as_string(camera, &error);
    assert(error == NULL);
    pixelFormat = arv_camera_get_pixel_format(camera, &error);
    assert(error == NULL);

    // XXX start acquisition on users request!
    arv_camera_start_acquisition(camera, &error);
//    arv_camera_stop_acquisition(camera, &error);
    assert(error == NULL);

    g_signal_connect(stream, "new-buffer", G_CALLBACK(Camera::newBufferCallback), this);

}

void Camera::stopVideo(void) {
    D("camera name %s\n", deviceId);

    assert(camera != NULL);

    if (ARV_IS_STREAM(stream)) {
        arv_stream_set_emit_signals(stream, FALSE);
    }

	g_clear_object(&stream);

//	g_clear_object (&viewer->last_buffer);

    GError *error = NULL;
    if (ARV_IS_CAMERA(camera)) {
        arv_camera_stop_acquisition(camera, &error);
        if (error != NULL) {
            E("arv_camera_stop_acquisition() failed : %s\n", (error != NULL) ? error->message : "???");
            assert(error == NULL);
            g_clear_error(&error);
        }
    }
}

bool Camera::infoQuery(void) {
    D("\n");

    GError *error = NULL;
    int x, y, width, height;
	int dx, dy;
	int min, max;
	int step;

    // XXX: handle possible error state!

    arv_camera_get_region(camera, &x, &y, &width, &height, &error);
    assert(error == NULL);
    arv_camera_get_width_bounds(camera, &min, &max, &error);
    assert(error == NULL);
	step = arv_camera_get_width_increment(camera, &error);
    assert(error == NULL);
    xSize.value = width;
    xSize.min = min;
    xSize.max = max;
    xSize.step = step;
    arv_camera_get_height_bounds(camera, &min, &max, &error);
    assert(error == NULL);
	step = arv_camera_get_height_increment(camera, &error);
    assert(error == NULL);
    ySize.value = height;
    ySize.min = min;
    ySize.max = max;
    ySize.step = step;

    arv_camera_get_x_offset_bounds(camera, &min, &max, &error);
    assert(error == NULL);
	step = arv_camera_get_x_offset_increment(camera, &error);
    assert(error == NULL);
    xOffset.value = x;
    xOffset.min = min;
    xOffset.max = max;
    xOffset.step = step;
    arv_camera_get_y_offset_bounds(camera, &min, &max, &error);
    assert(error == NULL);
	step = arv_camera_get_y_offset_increment(camera, &error);
    assert(error == NULL);
    yOffset.value = y;
    yOffset.min = min;
    yOffset.max = max;
    yOffset.step = step;

    binningAvailable = arv_camera_is_binning_available(camera, &error);
    assert(error == NULL);
    arv_camera_get_binning(camera, &dx, &dy, &error);
    assert(error == NULL);
    arv_camera_get_x_binning_bounds(camera, &min, &max, &error);
    assert(error == NULL);
	step = arv_camera_get_x_binning_increment(camera, &error);
    assert(error == NULL);
    xBinning.value = dx;
    xBinning.min = min;
    xBinning.max = max;
    xBinning.step = step;
    arv_camera_get_y_binning_bounds(camera, &min, &max, &error);
    assert(error == NULL);
	step = arv_camera_get_y_binning_increment(camera,  &error);
    assert(error == NULL);
    yBinning.value = dy;
    yBinning.min = min;
    yBinning.max = max;
    yBinning.step = step;

    unsigned int pixelFormatCnt = 0;
    pixelFormats = arv_camera_dup_available_pixel_formats(camera, &pixelFormatCnt, &error);
    assert(error == NULL);
    unsigned int numValidFormats = 0;
    pixelFormatCurrent = 0;
    unsigned int pixelFormatStringCnt = 0;
    pixelFormatStrings = arv_camera_dup_available_pixel_formats_as_strings(camera, &pixelFormatStringCnt, &error);
    assert(error == NULL);
	g_assert (pixelFormatCnt == pixelFormatStringCnt);
    pixelFormatString = arv_camera_get_pixel_format_as_string(camera, &error);
    assert(error == NULL);
    D("current pixel format string %s\n", pixelFormatString);
    D("available pixel formats:\n");
    for (unsigned int i = 0; i < pixelFormatCnt; i++) {
		if (arv_pixel_format_to_gst_caps_string(pixelFormats[i]) != NULL) {
            D("[%d] pixel format string %s\n", i, pixelFormatStrings[i]);
            if (strncmp(pixelFormatString, pixelFormatStrings[i], strlen(pixelFormatString)) == 0) {
                pixelFormatCurrent = i;
            }
            numValidFormats++;
		}
	}
    numPixelFormats = numValidFormats;
    pixelFormat = arv_camera_get_pixel_format(camera, &error);
    assert(error == NULL);

    double minf, maxf;
    arv_camera_get_exposure_time_bounds(camera, &minf, &maxf, &error);
    assert(error == NULL);
    double _exposure = arv_camera_get_exposure_time(camera, &error);
    assert(error == NULL);
    exposure.value = _exposure;
    exposure.min = minf;
    exposure.max = maxf;
    // set 10 ms step
    exposure.step = 10.0;

    arv_camera_get_gain_bounds(camera, &minf, &maxf, &error);
    assert(error == NULL);
    double _gain = arv_camera_get_gain(camera, &error);
    assert(error == NULL);
    gain.value = _gain;
    gain.min = minf;
    gain.max = maxf;
    gain.step = 0.001;

    frameRateAvailable = arv_camera_is_frame_rate_available(camera, &error);
    assert(error == NULL);
    arv_camera_get_frame_rate_bounds(camera, &minf, &maxf, &error);
    assert(error == NULL);
    double _frameRate = arv_camera_get_frame_rate(camera, &error);
    assert(error == NULL);
    frameRate.value = _frameRate;
    frameRate.min = minf;
    frameRate.max = maxf;
    frameRate.step = 0.001;

    gainAvailable = arv_camera_is_gain_available(camera, &error);
    assert(error == NULL);
    gainAuto = arv_camera_get_gain_auto(camera, &error) != ARV_AUTO_OFF;
    assert(error == NULL);
    gainAutoAvailable = arv_camera_is_gain_auto_available(camera, &error);
    assert(error == NULL);

    exposureAvailable = arv_camera_is_exposure_time_available(camera, &error);
    assert(error == NULL);
    exposureAuto = arv_camera_get_exposure_time_auto(camera, &error) != ARV_AUTO_OFF;
    // might not be available
    if ((error != NULL) && (error->code != ARV_DEVICE_ERROR_FEATURE_NOT_FOUND)) {
        E("arv_camera_get_exposure_time_auto() failed : %s\n", (error != NULL) ? error->message : "");
        assert(error == NULL);
    }
    g_clear_error(&error);
    exposureAutoAvailable = arv_camera_is_exposure_auto_available(camera, &error);
    assert(error == NULL);

    // expected image size in bytes
    imagePayload = arv_camera_get_payload(camera, &error);
    assert(error == NULL);


    // XXX not setting anything here!
    //    if (arv_camera_is_gv_device(camera)) {
    //        arv_camera_gv_select_stream_channel(camera, arv_option_gv_stream_channel, NULL);
    //        arv_camera_gv_set_packet_delay(camera, arv_option_gv_packet_delay, NULL);
    //        arv_camera_gv_set_packet_size(camera, arv_option_gv_packet_size, NULL);
    //        arv_camera_gv_set_stream_options(camera, arv_option_no_packet_socket ?
    //                          ARV_GV_STREAM_OPTION_PACKET_SOCKET_DISABLED :
    //                          ARV_GV_STREAM_OPTION_NONE);
    //    }


    return true;
}


void Camera::controlLostCallback(void *_userData) {
    D("\n");

    assert(_userData != NULL);
    Camera *camera = (Camera *)_userData;

    E("camera %s control lost\n", camera->deviceId);
    camera->connected = false;

    // WHAT TO DO HERE?!?
    // XXX: handle this more gracefully!
    assert(1 == 0);

//	cancel = true;
}

void Camera::streamCallback(void *_userData, ArvStreamCallbackType _type, ArvBuffer *_buffer) {
    (void)_userData;
    (void)_buffer;
    if (_type == ARV_STREAM_CALLBACK_TYPE_INIT) {
        D("ARV_STREAM_CALLBACK_TYPE_INIT\n");
		if (! arv_make_thread_realtime (10) && ! arv_make_thread_high_priority (-10)) {
            E("Failed to make stream thread high priority");
        }
	} else if (_type == ARV_STREAM_CALLBACK_TYPE_EXIT) {
        D("ARV_STREAM_CALLBACK_TYPE_EXIT\n");
    } else if (_type == ARV_STREAM_CALLBACK_TYPE_START_BUFFER) {
        D("ARV_STREAM_CALLBACK_TYPE_START_BUFFER\n");
    } else if (_type == ARV_STREAM_CALLBACK_TYPE_BUFFER_DONE) {
        D("ARV_STREAM_CALLBACK_TYPE_BUFFER_DONE\n");
    } else {
        assert(1 != 1);
    }
}

void Camera::newBufferCallback(ArvStream *_stream, void *_userData) {
    ArvBuffer *buffer;
	gint n_input_buffers, n_output_buffers;

    assert(_stream != NULL);
    assert(_userData != NULL);
    Camera *camera = (Camera *)_userData;

    // do not block when trying to get an image
    buffer = arv_stream_try_pop_buffer(_stream);
    assert(buffer != NULL);
    if (buffer == NULL) {
        E("arv_stream_try_pop_buffer() returned no buffer: %d\n", arv_buffer_get_status(buffer));
        return;
    }

	arv_stream_get_n_buffers(_stream, &n_input_buffers, &n_output_buffers);
	D("have %d in, %d out buffers \n", n_input_buffers, n_output_buffers);

    if (arv_buffer_get_status(buffer) != ARV_BUFFER_STATUS_SUCCESS) {
        camera->numErrors++;
        E("arv_buffer_get_status() failed: %d\n", arv_buffer_get_status(buffer));
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
    D("raw image %lu bytes, pixel format %08X\n", size, pixelFormat);

    int imageDepth = 0;
    switch (pixelFormat) {
    case ARV_PIXEL_FORMAT_MONO_16:
        // D("pixel format ARV_PIXEL_FORMAT_MONO_16\n");
        imageDepth = 2;
        break;
    case ARV_PIXEL_FORMAT_MONO_8:
        // D("pixel format ARV_PIXEL_FORMAT_MONO_8\n");
        imageDepth = 1;
        break;
    default:
        E("unhandled pixel format 0x%X\n", pixelFormat);
        break;
    }
    assert(imageDepth != 0);
    // D("RGB image %lu bytes, pixel format RGB\n", size);

    if (camera->imageData == NULL) {
        // allocate room for a RGB image, aravis provided payload data may be
        // in other pixel formats (8 bit, 10 bit, 12 bit, 16 bit,..)
        camera->imageData = malloc(size);
        D("allocated %zu bytes for image data\n", size);
    } else if (camera->imageData != NULL) {
        // do we need to reallocate the RGB buffer?
        assert(camera->imageSize > 0);
        if (camera->imageSize < size) {
            camera->imageData = realloc(camera->imageData, size);
            D("re-allocated %zu bytes for image data\n", size);
        }
    }
    camera->imageSize = size;
    assert(camera->imageSize > 0);
    assert(camera->imageData != NULL);

    memcpy(camera->imageData, raw, size);
    camera->imageWidth = imageWidth;
    camera->imageHeight = imageHeight;
    camera->numImages++;
    camera->numBytes += size;

    // main loop will pick the new frame data
    camera->imageUpdate = true;
    D("new image available!\n");

    // always return the buffer to the stream
    arv_stream_push_buffer(_stream, buffer);
}



void Camera::setImageSize(const int _x, const int _y) {
    GError *error = NULL;

    arv_camera_set_region(camera, -1, -1, _x, _y, &error);
    if (error != NULL) {
        E("arv_camera_set_region() failed : %s\n", (error != NULL) ? error->message : "???");
        assert(error == NULL);
        g_clear_error(&error);
    }
    if (_x != -1) {
        xSize.value = _x;
    }
    if (_y != -1) {
        ySize.value = _y;
    }

    // image payload might have changed
    imagePayload = arv_camera_get_payload(camera, &error);
    if (error != NULL) {
        E("arv_camera_get_payload() failed : %s\n", (error != NULL) ? error->message : "???");
        assert(error == NULL);
        g_clear_error(&error);
    }
}

void Camera::setImageOffset(const int _x, const int _y) {
    GError *error = NULL;

    arv_camera_set_region(camera, _x, _y, -1, -1, &error);
    if (error != NULL) {
        E("arv_camera_set_region() failed : %s\n", (error != NULL) ? error->message : "???");
        assert(error == NULL);
        g_clear_error(&error);
    }
    if (_x != -1) {
        xOffset.value = _x;
    }
    if (_y != -1) {
        yOffset.value = _y;
    }

    // image payload might have changed
    imagePayload = arv_camera_get_payload(camera, &error);
    if (error != NULL) {
        E("arv_camera_get_payload() failed : %s\n", (error != NULL) ? error->message : "???");
        assert(error == NULL);
        g_clear_error(&error);
    }
}

void Camera::setImageBinning(const int _x, const int _y) {
    GError *error = NULL;

    arv_camera_set_binning(camera, _x, _y, &error);
    if (error != NULL) {
        E("arv_camera_set_binning() failed : %s\n", (error != NULL) ? error->message : "???");
        assert(error == NULL);
        g_clear_error(&error);
    }
    if (_x != -1) {
        xBinning.value = _x;
    }
    if (_y != -1) {
        yBinning.value = _y;
    }

    // image payload might have changed
    imagePayload = arv_camera_get_payload(camera, &error);
    if (error != NULL) {
        E("arv_camera_get_payload() failed : %s\n", (error != NULL) ? error->message : "???");
        assert(error == NULL);
        g_clear_error(&error);
    }
}

void Camera::setPixelFormat(const unsigned int _value) {
    GError *error = NULL;

    assert(_value < numPixelFormats);

    arv_camera_set_pixel_format(camera, pixelFormats[_value], &error);
    if (error != NULL) {
        E("arv_camera_set_pixel_format() failed : %s\n", (error != NULL) ? error->message : "???");
        assert(error == NULL);
        g_clear_error(&error);
    }
    pixelFormatCurrent = _value;
    pixelFormat = pixelFormats[pixelFormatCurrent];
    pixelFormatString = pixelFormatStrings[pixelFormatCurrent];

    // image payload might have changed with pixel format change
    imagePayload = arv_camera_get_payload(camera, &error);
    if (error != NULL) {
        E("arv_camera_get_payload() failed : %s\n", (error != NULL) ? error->message : "???");
        assert(error == NULL);
        g_clear_error(&error);
    }
}

void Camera::setFrameRate(const float _value) {
    GError *error = NULL;

    if (! frameRateAvailable) {
        return;
    }

    arv_camera_set_frame_rate(camera, _value, &error);
    if (error != NULL) {
        E("arv_camera_set_frame_rate() failed : %s\n", (error != NULL) ? error->message : "???");
        assert(error == NULL);
        g_clear_error(&error);
    }
    frameRate.value = _value;
}

void Camera::setGainAuto(const bool _value) {
    GError *error = NULL;

    if (! gainAutoAvailable) {
        return;
    }

    // XXX: handle ARV_AUTO_ONCE
    ArvAuto value = ARV_AUTO_OFF;
    if (_value) {
        value = ARV_AUTO_CONTINUOUS;
    }
    arv_camera_set_gain_auto(camera, value, &error);
    if (error != NULL) {
        E("arv_camera_set_exposure_time_auto() failed : %s\n", (error != NULL) ? error->message : "???");
        assert(error == NULL);
        g_clear_error(&error);
    }
    gainAuto = _value;
}

void Camera::setGain(const float _value) {
    GError *error = NULL;

    if (gainAutoAvailable && gainAuto) {
        return;
    }

    arv_camera_set_gain(camera, _value, &error);
    if (error != NULL) {
        E("arv_camera_set_gain() failed : %s\n", (error != NULL) ? error->message : "???");
        assert(error == NULL);
        g_clear_error(&error);
    }
    gain.value = _value;
}

void Camera::setExposureAuto(const bool _value) {
    GError *error = NULL;

    if (! exposureAutoAvailable) {
        return;
    }

    // XXX: handle ARV_AUTO_ONCE
    ArvAuto value = ARV_AUTO_OFF;
    if (_value) {
        value = ARV_AUTO_CONTINUOUS;
    }
    arv_camera_set_exposure_time_auto(camera, value, &error);
    if (error != NULL) {
        E("arv_camera_set_exposure_time_auto() failed : %s\n", (error != NULL) ? error->message : "???");
        assert(error == NULL);
        g_clear_error(&error);
    }
    exposureAuto = _value;
}

void Camera::setExposure(const float _value) {
    GError *error = NULL;

    if (exposureAutoAvailable && exposureAuto) {
        return;
    }

    arv_camera_set_exposure_time(camera, _value, &error);
    if (error != NULL) {
        E("arv_camera_set_exposure_time() failed : %s\n", (error != NULL) ? error->message : "???");
        assert(error == NULL);
        g_clear_error(&error);
    }
    exposure.value = _value;
}
