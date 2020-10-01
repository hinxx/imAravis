// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)


#include "imgui.h"
// for PushItemFlag(), ImGuiItemFlags_Disabled
#include "imgui_internal.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>

#include "imaravis.h"
#include "viewer.h"

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h>            // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

// If you get an error please report on github. You may try different GL context version or GLSL version. See GL<>GLSL version table at the top of this file.
static bool CheckShader(GLuint handle, const char* desc)
{
    GLint status = 0, log_length = 0;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);
    if ((GLboolean)status == GL_FALSE)
        fprintf(stderr, "ERROR: ImGui_ImplOpenGL3_CreateDeviceObjects: failed to compile %s!\n", desc);
    if (log_length > 1)
    {
        ImVector<char> buf;
        buf.resize((int)(log_length + 1));
        glGetShaderInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
        fprintf(stderr, "%s\n", buf.begin());
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
        fprintf(stderr, "ERROR: ImGui_ImplOpenGL3_CreateDeviceObjects: failed to link %s! (with GLSL '%s')\n", desc, g_glsl_version);
    if (log_length > 1)
    {
        ImVector<char> buf;
        buf.resize((int)(log_length + 1));
        glGetProgramInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
        fprintf(stderr, "%s\n", buf.begin());
    }
    return (GLboolean)status == GL_TRUE;
}

int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "ImAravis", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
    bool err = gladLoadGL(glfwGetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
    bool err = false;
    glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
    bool err = false;
    glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

//    GLint ExtensionCount;
//    glGetIntegerv(GL_NUM_EXTENSIONS, &ExtensionCount);
//    fprintf(stderr, "GL_NUM_EXTENSIONS: %d\n", ExtensionCount);
//    for (int i = 0; i < ExtensionCount; i++) {
//        fprintf(stderr, "[%d] %s\n", i, glGetStringi( GL_EXTENSIONS, i));
//    }

    int preferred_format = 0;
    glGetInternalformativ(GL_TEXTURE_2D, GL_RGBA, GL_TEXTURE_IMAGE_FORMAT, 1, &preferred_format);
    fprintf(stderr, "preferred internal format %X\n", preferred_format);
    // preferred internal format 1908 == GL_RGBA

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

    unsigned int vbo2, vbo, vao;
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
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
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a color attachment texture
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1280, 720, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLuint imageTexture = 0;
    GLuint colormapTexture = 0;

    // Create a OpenGL texture identifier
    glGenTextures(1, &imageTexture);
    glBindTexture(GL_TEXTURE_2D, imageTexture);

    // Setup filtering parameters for display
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    glGenTextures(1, &colormapTexture);
    glBindTexture(GL_TEXTURE_1D, colormapTexture);
    // glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);


    // aravis
//    imAravis *cam = new imAravis();
//    int glError = glGetError();
//    double timeout = ImGui::GetTime() + 1.0;
//    static float frameRate = arv_camera_get_frame_rate(cam->camera, NULL);
    Viewer *viewer = new Viewer();

    ImGui::StyleColorsLight();

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // list of detected cameras
        {
            viewer->showCameraList();
        }

        // selected camera info
        {
            viewer->showCameraInfo();
        }
#if 0
        {
            ImGui::Begin("Image Window");

            ImGui::Text("Device info   : %s %s %s", cam->vendor, cam->model, cam->device);
            ImGui::Text("Image size    : %d x %d px, %d bpp", cam->imageWidth, cam->imageHeight, 24);
            ImGui::Text("Image payload : %zu bytes", cam->imageSize);

            if (ImGui::Button("Start")) {
                arv_camera_start_acquisition(cam->camera, NULL);
                cam->acquiring = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Stop")) {
                arv_camera_stop_acquisition(cam->camera, NULL);
                cam->acquiring = false;
            }

            if (cam->imageUpdated) {

                // render to FBO
                // bind to framebuffer and draw scene as we normally would to color texture
                glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

                // make sure we clear the framebuffer's content
                glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
                glClear(GL_COLOR_BUFFER_BIT);

                glUseProgram(shaderHandle);

                // we expect a R8 type of pixel data wo/ alpha
                // GL_RED means single channels
                // GL_R8 is for 8 bit indexed images
                // XXX: make this work for 16 bit images as well
                // XXX: To change texels in an already existing 2d texture, use glTexSubImage2D
                //      https://www.khronos.org/opengl/wiki/Common_Mistakes#Updating_a_texture
                glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, cam->imageWidth, cam->imageHeight, 0, GL_RED, GL_UNSIGNED_BYTE, cam->imageData);
                glError = glGetError();
                if (glError != GL_NO_ERROR) {
                    fprintf(stderr, "glGetError() returned 0x%04X\n", glError);
                }
                assert(glError == 0);

                // set shader uniform index for indexed image: 0
                glUniform1i(glGetUniformLocation(shaderHandle, "screenTexture"), 0);
                glError = glGetError();
                if (glError != GL_NO_ERROR) {
                    fprintf(stderr, "glGetError() returned 0x%04X\n", glError);
                }
                assert(glError == 0);

                // set shader uniform index for colomap array: 1
                glUniform1i(glGetUniformLocation(shaderHandle, "ColorTable"), 1);
                glError = glGetError();
                if (glError != GL_NO_ERROR) {
                    fprintf(stderr, "glGetError() returned 0x%04X\n", glError);
                }
                assert(glError == 0);

                // rectangle vertex array
                glBindVertexArray(vao);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                // indexed image 2D texture
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, imageTexture);
                // colormap 1D texture
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_1D, colormapTexture);
                // GL_RGBA8 = 8 bits got each color and alpha
                // XXX: To change texels in an already existing 2d texture, use glTexSubImage2D
                //      https://www.khronos.org/opengl/wiki/Common_Mistakes#Updating_a_texture
                glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, cam->colorMap);

                glError = glGetError();
                if (glError != GL_NO_ERROR) {
                    fprintf(stderr, "glGetError() returned 0x%04X\n", glError);
                }
                assert(glError == 0);

                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                // now bind back to default framebuffer (for ImGui)
                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                glActiveTexture(GL_TEXTURE0);
                cam->imageUpdated = false;
            }

            ImGui::Text("pointer = %p", (void*)(intptr_t)imageTexture);
            static int bufferCount = 0;
            static int totalBufferCount = 0;
            static int errorCount = 0;
            static double transferred = 0;
            if (timeout <= ImGui::GetTime()) {
                bufferCount = cam->bufferCount;
                totalBufferCount += bufferCount;
                errorCount += cam->errorCount;
                transferred = (double) cam->transferred / 1e6;
                //cam->periodic_task_cb();
                // reset the stats until next timeout (1 s)
                cam->bufferCount = 0;
                cam->errorCount = 0;
                cam->transferred = 0;
                timeout = ImGui::GetTime() + 1.0;
            }
            ImGui::Text("frames/s  %d", bufferCount);
            ImGui::Text("transfer  %.3g MiB/s", transferred);
            ImGui::Text("completed %d", totalBufferCount);
            ImGui::Text("error     %d", errorCount);

            if (ImGui::SliderFloat("rate", &frameRate, 0.1f, 60.0f)) {
                arv_camera_set_frame_rate(cam->camera, frameRate, NULL);
            }

            // use FBO rendered texture
            ImGui::Image((void*)(intptr_t)textureColorbuffer, ImVec2(cam->imageWidth, cam->imageHeight));

            ImGui::End();
        }
#endif

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // aravis
//    delete cam;
    delete viewer;

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
