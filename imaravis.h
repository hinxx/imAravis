#ifndef IMARAVIS_H
#define IMARAVIS_H

#include <arv.h>

struct imAravis
{
    imAravis();
    ~imAravis();

    bool initialize(void);
    void destroy(void);
    static void stream_cb (void *user_data, ArvStreamCallbackType type, ArvBuffer *buffer);
    static void control_lost_cb (ArvGvDevice *gv_device);
    static void new_buffer_cb(ArvStream *_stream, void *_arg);
    bool periodic_task_cb(void);
    bool emit_software_trigger(void);

    void initColorMap(void);

//    void startAcquisition(void) {
//        arv_camera_start_acquisition(camera, NULL);
//    }
//    void stopAcquisition(void) {
//        arv_camera_stop_acquisition(camera, NULL);
//    }

    // vars
    ArvCamera *camera;
	ArvStream *stream;

    int bufferCount;
	int errorCount;
	size_t transferred;

//	ArvChunkParser *chunk_parser;
//	char **chunks;

    char *vendor;
    char *model;
    char *device;
//    size_t payload;
//    int pixel_format;
    int imageWidth;
    int imageHeight;
//    int image_depth;
    bool acquiring;
//    void *imageDataRaw;
    size_t imageSize;
    bool imageUpdated;
    void *imageData;
    unsigned char colorMap8[256][3];
    bool applyColorMap;
    float colorMapFloat[256][3];
    unsigned char colorMap[256][4];
};

#endif // IMARAVIS_H
