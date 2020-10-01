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
    unsigned int pixelFormatCurrent;

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

    Camera(const unsigned int _index);
    ~Camera(void);
    void stop(void);
    void start(void);
    void startVideo(void);
    void stopVideo(void);
    static void controlLostCallback(void *_userData);
    static void streamCallback(void *_userData, ArvStreamCallbackType _type, ArvBuffer *_buffer);
    static void newBufferCallback(ArvStream *_stream, void *_userData);
    bool infoQuery(void);

    void setImageSize(const int _x, const int _y);
    void setImageOffset(const int _x, const int _y);
    void setImageBinning(const int _x, const int _y);
    void setPixelFormat(const unsigned int _value);
    void setFrameRate(const float _value);
    void setGain(const float _value);
    void setGainAuto(const bool _value);
    void setExposure(const float _value);
    void setExposureAuto(const bool _value);

};

#endif // CAMERA_H
