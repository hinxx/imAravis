#ifndef VIEWER_H
#define VIEWER_H

#include "camera.h"

#include <vector>

struct Viewer {
    std::vector<Camera *> cameras;
    Camera *camera;
    char *selectedCamera;

    Viewer(void);
    ~Viewer(void);

    void addCamera(const unsigned int _index, const char *_protocol, const char *_deviceId, const char *_vendor, const char *_model, const char *_serialNumber, const char *_physicalId);
    void clearCameraList(void);
    void selectCamera(const unsigned int _index);
    void stopCamera(void);
    void startCamera(void);
};

#endif // VIEWER_H
