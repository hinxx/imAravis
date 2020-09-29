#include "viewer.h"
#include "debug.h"
#include <assert.h>

Viewer::Viewer() {
//    selectedCamera = -1;
    camera = NULL;
    cameras.clear();
}

Viewer::~Viewer() {
    if (camera) {
        // stop the activity..
    }
    clearCameraList();
}

void Viewer::addCamera(const unsigned int _index, const char *_protocol, const char *_deviceId, const char *_vendor, const char *_model, const char *_serialNumber, const char *_physicalId) {
    for (size_t i = 0; i < cameras.size(); i++) {
        Camera *cam = cameras[i];
        if (((strncmp(cam->serialNumber, _serialNumber, strlen(cam->serialNumber))) == 0) &&
                (strlen(_serialNumber) == strlen(cam->serialNumber))) {
            // already in the list
            return;
        }
    }
    cameras.push_back(new Camera(_index, _protocol, _deviceId, _vendor, _model, _serialNumber, _physicalId));
}

void Viewer::clearCameraList(void) {
    for (size_t i = 0; i < cameras.size(); i++) {
        delete cameras[i];
    }
    cameras.clear();
    camera = NULL;
}

void Viewer::selectCamera(const unsigned int _index) {
    Camera *cam = NULL;
    bool found = false;
    for (size_t i = 0; i < cameras.size(); i++) {
        cam = cameras[i];
        if (cam->index == _index) {
            found = true;
            break;
        }
    }
    assert(found == true);
    D("selected camera index %d\n", cam->index);

    if (camera && (camera->index == cam->index)) {
        // already selected
        return;
    }

    // stop the current device
    stopCamera();
    camera = cam;
    // start selected the current device
    startCamera();
}

void Viewer::startCamera(void) {
    D("\n");
    if (camera == NULL) {
        E("no camera.. nothing to do..\n");
        return;
    }
    camera->start();
}

void Viewer::stopCamera(void) {
    D("\n");
    if (camera == NULL) {
        D("no camera.. nothing to do..\n");
        return;
    }
    camera->stop();
}

void Camera::controlLostCallback(void *_arg) {

    Camera *camera = (Camera *)_arg;
	E("camera %s control lost\n", camera->deviceId);

    camera->connected = false;

    // WHAT TO DO HERE?!?
    // XXX: handle this more gracefully!
    assert(1 == 0);

//	cancel = true;
}





Camera::Camera(const unsigned int _index, const char *_protocol, const char *_deviceId, const char *_vendor, const char *_model, const char *_serialNumber, const char *_physicalId) {
    index = _index;
    protocol = strdup(_protocol);
    deviceId = strdup(_deviceId);
    vendor = strdup(_vendor);
    model = strdup(_model);
    serialNumber = strdup(_serialNumber);
    physicalId = strdup(_physicalId);

    camera = NULL;
    stream = NULL;
    connected = false;
}

Camera::~Camera(void) {
    stop();
    index = 0;
    free(protocol);
    free(deviceId);
    free(vendor);
    free(model);
    free(serialNumber);
    free(physicalId);
    camera = NULL;
    stream = NULL;
}

void Camera::start(void) {
    D("camera name %s\n", deviceId);

    assert(camera == NULL);

    GError *error = NULL;
    camera = arv_camera_new(deviceId, &error);
    if (! ARV_IS_CAMERA(camera)) {
        E("No camera found%s%s\n",
			error != NULL ? ": " : "",
			error != NULL ? error->message : "");
		g_clear_error(&error);
        return;
    }

	arv_device_set_register_cache_policy(arv_camera_get_device(camera), cachePolicy);

    arv_camera_set_chunk_mode(camera, FALSE, &error);
    if ((error != NULL) && (error->code != ARV_DEVICE_ERROR_FEATURE_NOT_FOUND)  ) {
        E("arv_camera_set_chunk_mode() failed : %s\n",
			error != NULL ? error->message : "???");
		g_clear_error(&error);
        return;
    }
    // arv_camera_set_chunk_mode() might have set the error if the feature is missing
    g_clear_error(&error);

    int x, y, width, height;
	int dx, dy;
	int min, max;
	int step;

    // XXX: handle possible error state!
    arv_camera_get_region(camera, &x, &y, &width, &height, &error);
    assert(error == NULL);
	arv_camera_get_binning(camera, &dx, &dy, &error);
    assert(error == NULL);
    // binning
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
    // offset
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
    // size
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

    // pixels formats
    unsigned int pixelFormatCnt = 0;
    pixelFormats = arv_camera_dup_available_pixel_formats(camera, &pixelFormatCnt, &error);
    assert(error == NULL);
    unsigned int numValidFormats = 0;
    unsigned int pixelFormatStringCnt = 0;
    pixelFormatStrings = arv_camera_dup_available_pixel_formats_as_strings(camera, &pixelFormatStringCnt, &error);
    assert(error == NULL);
	g_assert (pixelFormatCnt == pixelFormatStringCnt);
//    pixelFormatString = strdup(arv_camera_get_pixel_format_as_string(camera, &error));
    pixelFormatString = arv_camera_get_pixel_format_as_string(camera, &error);
    assert(error == NULL);
    for (unsigned int i = 0; i < pixelFormatCnt; i++) {
		if (arv_pixel_format_to_gst_caps_string(pixelFormats[i]) != NULL) {
            D("[%d] pixel format string %s\n", i, pixelFormatStrings[i]);
            numValidFormats++;
		}
	}
    D("selected pixel format string %s\n", pixelFormatString);
    numPixelFormats = numValidFormats;

    binningAvailable = arv_camera_is_binning_available(camera, &error);
    assert(error == NULL);

    //g_signal_connect (arv_camera_get_device (viewer->camera), "control-lost", G_CALLBACK (control_lost_cb), viewer);

    g_signal_connect(arv_camera_get_device(camera), "control-lost", G_CALLBACK(Camera::controlLostCallback), this);

//    arv_camera_set_region(camera, 0, 0, arv_option_width, arv_option_height, NULL);
//    arv_camera_set_binning(camera, arv_option_horizontal_binning, arv_option_vertical_binning, NULL);
//    arv_camera_set_exposure_time(camera, arv_option_exposure_time_us, NULL);
//    arv_camera_set_gain(camera, arv_option_gain, NULL);

//    if (arv_camera_is_gv_device(camera)) {
//        arv_camera_gv_select_stream_channel(camera, arv_option_gv_stream_channel, NULL);
//        arv_camera_gv_set_packet_delay(camera, arv_option_gv_packet_delay, NULL);
//        arv_camera_gv_set_packet_size(camera, arv_option_gv_packet_size, NULL);
//        arv_camera_gv_set_stream_options(camera, arv_option_no_packet_socket ?
//                          ARV_GV_STREAM_OPTION_PACKET_SOCKET_DISABLED :
//                          ARV_GV_STREAM_OPTION_NONE);
//    }

//    int width, height;
//    arv_camera_get_region(camera, &x, &y, &width, &height, NULL);
//    arv_camera_get_binning(camera, &dx, &dy, NULL);
//    exposure = arv_camera_get_exposure_time(camera, NULL);
//    payload = arv_camera_get_payload(camera, NULL);
//    gain = arv_camera_get_gain(camera, NULL);

    connected = true;
}

void Camera::stop(void) {
    D("camera name %s\n", deviceId);

//    assert(camera != NULL);
    if (camera == NULL) {
        return;
    }

//    free(pixelFormatString);
    g_free(pixelFormatStrings);
    g_free(pixelFormats);
    numPixelFormats = 0;

    connected = false;
    stopVideo();

    g_clear_object(&camera);
    camera = NULL;
}

void Camera::startVideo(void) {
    D("camera name %s\n", deviceId);

    assert(camera != NULL);
}

void Camera::stopVideo(void) {
    D("camera name %s\n", deviceId);

    assert(camera != NULL);
}
