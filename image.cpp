#include "image.h"
#include "imgui.h"
#include "debug.h"

GLuint shaderHandle = 0, vertHandle = 0, fragHandle = 0;

// this works with the TRIANGLE/RECTANGLE
const GLchar* vertex_shader =
    "in vec3 aPos;\n"           // attributes 0
    "in vec2 aTexCoords;\n"     // attributes 1
    "out vec2 TexCoords;\n"     // for fragment shader
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(aPos, 1.0);\n"
    "    TexCoords = aTexCoords;\n"
    "}\n";

// https://gamedev.stackexchange.com/questions/43294/creating-a-retro-style-palette-swapping-effect-in-opengl
const GLchar* fragment_shader =
    "out vec4 FragColor;\n"
    "in vec2 TexCoords;\n"                  // from vertex shader
    "uniform sampler2D screenTexture;\n"    // our texture (grayscale)
    "uniform sampler1D ColorTable;\n"       // our colormap 256 colors
    "void main()\n"
    "{\n"
    // Pick up a color index
    "    vec4 index = texture2D(screenTexture, TexCoords);\n"
    // Retrieve the actual color from the palette
    "    vec4 texel = texture1D(ColorTable, index.x);\n"
    // Output the color
    "    FragColor = texel;"
    "}\n";


// If you get an error please report on github. You may try different GL context version or GLSL version. See GL<>GLSL version table at the top of this file.
static bool CheckShader(GLuint handle, const char* desc)
{
    GLint status = 0, log_length = 0;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);
    if ((GLboolean)status == GL_FALSE)
        E("failed to compile %s!\n", desc);
    if (log_length > 1)
    {
        ImVector<char> buf;
        buf.resize((int)(log_length + 1));
        glGetShaderInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
        E("%s\n", buf.begin());
    }
    return (GLboolean)status == GL_TRUE;
}

const char* g_glsl_version = "#version 130\n";

// If you get an error please report on GitHub. You may try different GL context version or GLSL version.
static bool CheckProgram(GLuint handle, const char* desc)
{
    GLint status = 0, log_length = 0;
    glGetProgramiv(handle, GL_LINK_STATUS, &status);
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length);
    if ((GLboolean)status == GL_FALSE)
        E("failed to link %s! (with GLSL '%s')\n", desc, g_glsl_version);
    if (log_length > 1)
    {
        ImVector<char> buf;
        buf.resize((int)(log_length + 1));
        glGetProgramInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
        E("%s\n", buf.begin());
    }
    return (GLboolean)status == GL_TRUE;
}

// from https://github.com/kbinani/colormap-shaders
// Matlab JET colormap
struct vec4
{
    vec4(float a0, float a1, float a2, float a3)
        : x(a0)
        , y(a1)
        , z(a2)
        , w(a3)
    {
    }

    union {
        double r;
        double x;
    };
    union {
        double g;
        double y;
    };
    union {
        double b;
        double z;
    };
    union {
        double a;
        double w;
    };
};

float colormap_red(float x) {
    if (x < 0.7) {
        return 4.0 * x - 1.5;
    } else {
        return -4.0 * x + 4.5;
    }
}

float colormap_green(float x) {
    if (x < 0.5) {
        return 4.0 * x - 0.5;
    } else {
        return -4.0 * x + 3.5;
    }
}

float colormap_blue(float x) {
    if (x < 0.3) {
       return 4.0 * x + 0.5;
    } else {
       return -4.0 * x + 2.5;
    }
}

float clamp(float v, float min, float max) {
    if (v < min) {
        return min;
    } else if (max < v) {
        return max;
    } else {
        return v;
    }
}

vec4 colormap(float x) {
    float r = clamp(colormap_red(x), 0.0, 1.0);
    float g = clamp(colormap_green(x), 0.0, 1.0);
    float b = clamp(colormap_blue(x), 0.0, 1.0);
    return vec4(r, g, b, 1.0);
}

void Image::initColorMap(void) {
    float v;
    for (int i = 0; i < 256; i++) {
        v = (float)i / 255.0;
        // make JET colomap
        colorMap[i][0] = (unsigned char)(clamp(colormap_red(v), 0.0, 1.0) * 255.0);
        colorMap[i][1] = (unsigned char)(clamp(colormap_green(v), 0.0, 1.0) * 255.0);
        colorMap[i][2] = (unsigned char)(clamp(colormap_blue(v), 0.0, 1.0) * 255.0);
        colorMap[i][3] = 255;
        // fprintf(stderr, "RGBA %3d %3d %3d %3d\n", colorMap[i][0], colorMap[i][1], colorMap[i][2], colorMap[i][3]);
    }
}
// Matlab JET colormap

Image::Image() {

    rawTexture = 0;

    //RECTANGLE CREATION//

    // half size
//    float vertices0[] = {
//         0.5f,  0.5f, 0.0f,  // top right
//         0.5f, -0.5f, 0.0f,  // bottom right
//        -0.5f, -0.5f, 0.0f,  // bottom left
//        -0.5f,  0.5f, 0.0f   // top left
//    };

    // full size
    float vertices[] = {
         1.0f,  1.0f, 0.0f,  // top right
         1.0f, -1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.0f,  // bottom left
        -1.0f,  1.0f, 0.0f   // top left
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    // texture
    float Tvertices2[] = {
        1.0f,   1.0f,
        1.0f,   0.0f,
        0.0f,   0.0f,
        0.0f,   1.0f
    };

    unsigned int vbo2, vbo;
//    unsigned int EBO;
    glGenBuffers(1, &ebo);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vbo2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Tvertices2), Tvertices2, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Create shaders
    const GLchar* vertex_shader_with_version[2] = { g_glsl_version, vertex_shader };
    vertHandle = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertHandle, 2, vertex_shader_with_version, NULL);
    glCompileShader(vertHandle);
    CheckShader(vertHandle, "vertex shader");

    const GLchar* fragment_shader_with_version[2] = { g_glsl_version, fragment_shader };
    fragHandle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragHandle, 2, fragment_shader_with_version, NULL);
    glCompileShader(fragHandle);
    CheckShader(fragHandle, "fragment shader");

    shaderHandle = glCreateProgram();
    glAttachShader(shaderHandle, vertHandle);
    glAttachShader(shaderHandle, fragHandle);
    glLinkProgram(shaderHandle);
    CheckProgram(shaderHandle, "shader program");

    // framebuffer configuration
    // -------------------------
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    // create a color attachment texture
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1280, 720, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        E("Framebuffer is not complete!");
    }

    GLuint glError = glGetError();
    if (glError != GL_NO_ERROR) {
        fprintf(stderr, "glGetError() returned 0x%04X\n", glError);
    }
    assert(glError == 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    initColorMap();

    // Create a OpenGL texture identifier for color map
    glGenTextures(1, &paletteTexture);
    glBindTexture(GL_TEXTURE_1D, paletteTexture);
    // glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    // GL_RGBA8 = 8 bits got each color and alpha
    // XXX: To change texels in an already existing 2d texture, use glTexSubImage2D
    //      https://www.khronos.org/opengl/wiki/Common_Mistakes#Updating_a_texture
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, colorMap);
    glError = glGetError();
    if (glError != GL_NO_ERROR) {
        E("glGetError() returned 0x%04X\n", glError);
    }
    assert(glError == 0);

    // Create a OpenGL texture identifier for raw image
    glGenTextures(1, &rawTexture);
    glBindTexture(GL_TEXTURE_2D, rawTexture);
    // Setup filtering parameters for display
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    glError = glGetError();
    if (glError != GL_NO_ERROR) {
        fprintf(stderr, "glGetError() returned 0x%04X\n", glError);
    }
    assert(glError == 0);
}

Image::~Image() {

}

void Image::updateImage(const unsigned int _width, const unsigned int _height, const void *_data) {
    assert(rawTexture != 0);

    GLuint glError;

    // render to FBO
    // bind to framebuffer and draw scene as we normally would to color texture
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // make sure we clear the framebuffer's content
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderHandle);

    // we expect a R8 type of pixel data wo/ alpha
    // GL_RED means single channels
    // GL_R8 is for 8 bit indexed images
    // XXX: make this work for 16 bit images as well
    // XXX: To change texels in an already existing 2d texture, use glTexSubImage2D
    //      https://www.khronos.org/opengl/wiki/Common_Mistakes#Updating_a_texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, _width, _height, 0, GL_RED, GL_UNSIGNED_BYTE, _data);
    glError = glGetError();
    if (glError != GL_NO_ERROR) {
       fprintf(stderr, "ERROR: glGetError() returned 0x%04X\n", glError);
    }
    assert(glError == 0);

    imageWidth = _width;
    imageHeight = _height;

    // set shader uniform index for indexed image: 0
    glUniform1i(glGetUniformLocation(shaderHandle, "screenTexture"), 0);
    glError = glGetError();
    if (glError != GL_NO_ERROR) {
        E("glGetError() returned 0x%04X\n", glError);
    }
    assert(glError == 0);

    // set shader uniform index for colomap array: 1
    glUniform1i(glGetUniformLocation(shaderHandle, "ColorTable"), 1);
    glError = glGetError();
    if (glError != GL_NO_ERROR) {
        E("glGetError() returned 0x%04X\n", glError);
    }
    assert(glError == 0);

    // rectangle vertex array
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    // indexed image 2D texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rawTexture);
    // colormap 1D texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, paletteTexture);
    // GL_RGBA8 = 8 bits got each color and alpha
    // XXX: To change texels in an already existing 2d texture, use glTexSubImage2D
    //      https://www.khronos.org/opengl/wiki/Common_Mistakes#Updating_a_texture
    //glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, colorMap);

    glError = glGetError();
    if (glError != GL_NO_ERROR) {
        E("glGetError() returned 0x%04X\n", glError);
    }
    assert(glError == 0);

    // render the image to fbo
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // now bind back to default framebuffer (for ImGui)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
}

void Image::render(void) {
    assert(rawTexture != 0);

//    ImGui::Image((void*)(intptr_t)rawTexture, ImVec2(imageWidth, imageHeight));
    ImGui::Image((void*)(intptr_t)colorTexture, ImVec2(imageWidth, imageHeight));
}
