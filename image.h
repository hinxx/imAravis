#ifndef IMAGE_H
#define IMAGE_H

#include <GL/gl3w.h>

struct Image {
    GLuint rawTexture;
    GLuint paletteTexture;
    GLuint colorTexture;
    GLuint fbo;
    GLuint ebo;
    GLuint vao;

    unsigned int imageWidth;
    unsigned int imageHeight;
    unsigned int imageDepth;
//    unsigned char colorMap[256][4];
    double scaleWidth;
    double scaleHeight;

    Image();
    ~Image();

    void initColorMap(void);
    void updateImage(const unsigned int _width, const unsigned int _height, const unsigned int _depth, const void *_data);
    void updateScale(const double _scaleWidth, const double _scaleHeight);
    void updatePalette(const unsigned int _depth, const void *_data);
    void render(void);
};

#endif // IMAGE_H
