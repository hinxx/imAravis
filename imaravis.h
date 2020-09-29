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

    ArvCamera *camera;
	ArvStream *stream;
    int bufferCount;
	int errorCount;
	size_t transferred;
    char *vendor;
    char *model;
    char *device;
    int imageWidth;
    int imageHeight;
    bool acquiring;
    size_t imageSize;
    bool imageUpdated;
    void *imageData;
    unsigned char colorMap[256][4];
};

#endif // IMARAVIS_H
