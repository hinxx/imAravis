#ifndef VIEWER_H
#define VIEWER_H

#include "camera.h"
#include "image.h"
#include "palette.h"
#include "genicam.h"

#include <vector>

struct Viewer {
    std::vector<Camera *> cameras;
    Camera *camera;
    char *selectedCamera;
    Image *image;
    double timeout;
    int numImages;
    int numAllImages;
    int numErrors;
    double numBytes;
    float scaleWidth;
    float scaleHeight;
    PaletteList paletteList;
    int paletteCurrentIndex;
    Genicam *genicam;

    Viewer(void);
    ~Viewer(void);

    void clearCameraList(void);
    void selectCamera(const unsigned int _index);
    void stopCamera(void);
    void startCamera(void);

    void showCameraList(void);
    void showCameraInfo(void);
    void showCameraImage(void);
    void showCameraFeatures(void);

    void handleImageBinning(void);
    void handleImageOffset(void);
    void handleImageSize(void);
    void handleImagePixelFormat(void);
    void handleImageFrameRate(void);
    void handleGain(void);
    void handleExposure(void);

};

#endif // VIEWER_H
