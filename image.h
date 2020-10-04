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
    unsigned char colorMap[256][4];

    Image();
    ~Image();

    void initColorMap(void);
    void updateImage(const unsigned int _width, const unsigned int _height, const void *_data);
    void render(void);
};

#endif // IMAGE_H
