#ifndef GENICAM_H
#define GENICAM_H

#include <arv.h>

struct Genicam {
    ArvDevice *device;
    ArvGc *genicam;

    Genicam();
    ~Genicam();
    void initialize(ArvCamera *_camera);
    void listFeatures(void);
    void showFeature(const char *feature, int level);
};

#endif // GENICAM_H

