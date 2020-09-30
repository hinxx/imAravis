#ifndef CAMERA_H
#define CAMERA_H

#include <arv.h>

struct Parameter {
    int value;
    int min;
    int max;
    int step;
};

struct ParameterF {
    float value;
    float min;
    float max;
    float step;
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
    ParameterF exposure;
    ParameterF gain;
    bool frameRateAvailable;
    ParameterF frameRate;

    bool gainAvailable;
    bool gainAuto;
    bool gainAutoAvailable;
    bool exposureAvailable;
    bool exposureAuto;
    bool exposureAutoAvailable;

    unsigned int numPixelFormats;
    long *pixelFormats;
    const char **pixelFormatStrings;
    const char *pixelFormatString;
    ArvPixelFormat pixelFormat;
    int pixelFormatCurrent;

    bool autoSocketBuffer;
    bool packetResend;
    // in milli seconds
    unsigned int packetTimeout;
    unsigned int frameRetention;
    double packetRequestRatio;

    unsigned int imagePayload;
    unsigned int imageWidth;
    unsigned int imageHeight;

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
    bool infoQuery(void);
};

#endif // CAMERA_H
