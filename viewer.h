#ifndef VIEWER_H
#define VIEWER_H

#include <arv.h>
#include <vector>

struct Parameter {
    int value;
    int min;
    int max;
    int step;
};

struct Camera {
    unsigned int index;
    char *protocol;
    char *deviceId;
    char *vendor;
    char *model;
    char *serialNumber;
    char *physicalId;

    bool connected;
    ArvCamera *camera;
    ArvRegisterCachePolicy cachePolicy;
	ArvStream *stream;

    bool binningAvailable;
    Parameter xBinning;
    Parameter yBinning;
    Parameter xOffset;
    Parameter yOffset;
    Parameter xSize;
    Parameter ySize;
    unsigned int numPixelFormats;
    long *pixelFormats;
    const char **pixelFormatStrings;
    const char *pixelFormatString;
    ArvPixelFormat pixelFormat;

    bool autoSocketBuffer;
    bool packetResend;
    // in milli seconds
    unsigned int packetTimeout;
    unsigned int frameRetention;
    double packetRequestRatio;

    unsigned int numImages;
    unsigned int numBytes;
    unsigned int numErrors;

    Camera(const unsigned int _index, const char *_protocol, const char *_deviceId, const char *_vendor, const char *_model, const char *_serialNumber, const char *_physicalId);
    ~Camera(void);
    void stop(void);
    void start(void);
    void startVideo(void);
    void stopVideo(void);
    static void controlLostCallback(void *_userData);
    static void streamCallback(void *_userData, ArvStreamCallbackType _type, ArvBuffer *_buffer);
    static void newBufferCallback(ArvStream *_stream, void *_userData);
};

struct Viewer {
    std::vector<Camera *> cameras;
    Camera *camera;

    Viewer(void);
    ~Viewer(void);
    void addCamera(const unsigned int _index, const char *_protocol, const char *_deviceId, const char *_vendor, const char *_model, const char *_serialNumber, const char *_physicalId);
    void clearCameraList(void);
    void selectCamera(const unsigned int _index);
    void stopCamera(void);
    void startCamera(void);
};


#endif // VIEWER_H
