#include "debug.h"
#include "viewer.h"
#include "camera.h"

#include <assert.h>
#include <stdlib.h>

Viewer::Viewer() {
//    selectedCamera = -1;
    camera = NULL;
    selectedCamera = NULL;
    cameras.clear();
}

Viewer::~Viewer() {
    if (camera) {
        camera->stop();
    }
    clearCameraList();
    free(selectedCamera);
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

    if (selectedCamera) {
        free(selectedCamera);
    }
    selectedCamera = (char *)calloc(1, strlen(cam->deviceId) + strlen(cam->serialNumber) + 5);
    sprintf(selectedCamera, "%s - %s", cam->deviceId, cam->serialNumber);

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
