#ifndef GENICAM_H
#define GENICAM_H

#include <arv.h>
#include <vector>

//struct Feature {
//    char *name;

//    Feature(const char *_name) {

//    }
//};

struct Genicam {
    ArvDevice *device;
    ArvGc *genicam;
//    bool update;
//    std::vector<Feature> features;

    Genicam();
    ~Genicam();
    void initialize(ArvCamera *_camera);
    void destroy(void);
    void listFeatures(void);
    void showFeature(const char *feature, int level);
    void traverse(const char *_name);

    void handleCategory(const char *_name);
    void handleFeature(const char *_name);
    void handleFeatureInteger(ArvGcNode *_node);
    void handleFeatureFloat(ArvGcNode *_node);
    void handleFeatureBoolean(ArvGcNode *_node);
    void handleFeatureString(ArvGcNode *_node);
    void handleFeatureCommand(ArvGcNode *_node);
};

#endif // GENICAM_H

