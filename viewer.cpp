#include "debug.h"
#include "viewer.h"
#include "camera.h"

#include "imgui.h"
// for PushItemFlag(), ImGuiItemFlags_Disabled
#include "imgui_internal.h"

#include <assert.h>
#include <stdlib.h>

Viewer::Viewer() {
    camera = NULL;
    selectedCamera = NULL;
    cameras.clear();

    image = new Image();
    numImages = 0;
    numAllImages = 0;
    numErrors = 0;
    numBytes = 0;

    scaleWidth = 1.0f;
    scaleHeight = 1.0f;
}

Viewer::~Viewer() {
    delete image;

    if (camera) {
        camera->stop();
    }
    clearCameraList();
}

void Viewer::clearCameraList(void) {
    for (size_t i = 0; i < cameras.size(); i++) {
        delete cameras[i];
    }
    cameras.clear();
    camera = NULL;
    free(selectedCamera);
    selectedCamera = NULL;
}

void Viewer::selectCamera(const unsigned int _index) {
    Camera *cam = NULL;
    bool found = false;
    for (size_t i = 0; i < cameras.size(); i++) {
        cam = cameras[i];
        if (cam->index == _index) {
            found = true;
            break;
        }
    }
    assert(found == true);
    D("selected camera index %d\n", cam->index);

    if (camera && (camera->index == cam->index)) {
        // already selected
        return;
    }

    if (selectedCamera) {
        free(selectedCamera);
    }
    selectedCamera = (char *)calloc(1, strlen(cam->deviceId) + strlen(cam->serialNumber) + 5);
    sprintf(selectedCamera, "%s - %s", cam->deviceId, cam->serialNumber);

    // stop the current device
    stopCamera();
    camera = cam;
    // start selected the current device
    startCamera();
}

void Viewer::startCamera(void) {
    D("\n");
    if (camera == NULL) {
        E("no camera.. nothing to do..\n");
        return;
    }
    camera->start();
}

void Viewer::stopCamera(void) {
    D("\n");
    if (camera == NULL) {
        D("no camera.. nothing to do..\n");
        return;
    }
    camera->stop();
}

void Viewer::showCameraList(void) {

    ImGui::Begin("Camera list");

    // generate list of cameras
    if (ImGui::Button("update device list")) {
        clearCameraList();

        arv_update_device_list();
        unsigned int n_devices = arv_get_n_devices();
        for (unsigned int i = 0; i < n_devices; i++) {
            cameras.push_back(new Camera(i));
        }
    }

    // show list of cameras
    if (! cameras.empty()) {
        ImGui::Text("Found %zu devices", cameras.size());
        ImGui::Columns(7, "cameras");
        ImGui::Separator();
        ImGui::Text("ID"); ImGui::NextColumn();
        ImGui::Text("Device"); ImGui::NextColumn();
        ImGui::Text("Vendor"); ImGui::NextColumn();
        ImGui::Text("Model"); ImGui::NextColumn();
        ImGui::Text("Serial nr."); ImGui::NextColumn();
        ImGui::Text("Protocol"); ImGui::NextColumn();
        ImGui::Text("Physical ID"); ImGui::NextColumn();
        ImGui::Separator();
        unsigned int selected = (camera != NULL) ? camera->index : -1;
        for (size_t i = 0; i < cameras.size(); i++) {
            Camera *cam = cameras[i];
            char label[32];
            sprintf(label, "%04u", cam->index);
            if (ImGui::Selectable(label, selected == cam->index, ImGuiSelectableFlags_SpanAllColumns)){
                D("clicked on index %d\n", cam->index);
                selectCamera(cam->index);
            }
            ImGui::NextColumn();
            ImGui::Text("%s", cam->deviceId); ImGui::NextColumn();
            ImGui::Text("%s", cam->vendor); ImGui::NextColumn();
            ImGui::Text("%s", cam->model); ImGui::NextColumn();
            ImGui::Text("%s", cam->serialNumber); ImGui::NextColumn();
            ImGui::Text("%s", cam->protocol); ImGui::NextColumn();
            ImGui::Text("%s", cam->physicalId); ImGui::NextColumn();
        }
        ImGui::Columns(1);
    } else {
        ImGui::Text("No cameras.");
    }
    // ImGui::Separator();

    ImGui::End();
}


void Viewer::showCameraInfo(void) {
    // show this window only if a camera is selected
    if (! selectedCamera) {
        return;
    }

    ImGui::Begin(selectedCamera);

    if (! camera->connected) {
        ImGui::Text("Camera %s not connected.", camera->deviceId);
        return;
    }

    ImGui::Text("Device name %s", camera->deviceId);
    ImGui::Text("Image payload %d bytes", camera->imagePayload);

    handleImageSize();
    handleImageOffset();
    handleImageBinning();
    handleImagePixelFormat();
    ImGui::Separator();

    handleImageFrameRate();
    handleExposure();
    handleGain();
    ImGui::Separator();

    if (ImGui::Button("Start")) {
        camera->startVideo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
        camera->stopVideo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset stats")) {
        numImages = 0;
        numAllImages = 0;
        numErrors = 0;
        numBytes = 0;
    }
    ImGui::Separator();
    if (ImGui::SliderFloat("Width scale", &scaleWidth, 0.0f, 1.0f)) {
        image->updateScale(scaleWidth, scaleHeight);
    }
    if (ImGui::SliderFloat("Height scale", &scaleHeight, 0.0f, 1.0f)) {
        image->updateScale(scaleWidth, scaleHeight);
    }
    ImGui::Separator();

    // statistics are updated once per second
    if (timeout <= ImGui::GetTime()) {
        numImages = camera->numImages;
        numAllImages += numImages;
        numErrors += camera->numErrors;
        numBytes = (double) camera->numBytes / 1e6;
        // reset the stats until next timeout (1 s)
        camera->numImages = 0;
        camera->numErrors = 0;
        camera->numBytes = 0;
        timeout = ImGui::GetTime() + 1.0;
    }
    ImGui::Text("rate           %d images/s", numImages);
    ImGui::Text("transferred    %.3g MiB/s", numBytes);
    ImGui::Text("# OK images    %d", numAllImages);
    ImGui::Text("# error images %d", numErrors);
    ImGui::Separator();

    ImGui::End();
}

void Viewer::showCameraImage(void) {
    // show this window only if a camera is selected
    if (! selectedCamera) {
        return;
    }

    ImGui::Begin("Image Window");

    if (camera->imageUpdate) {
        image->updateImage(camera->imageWidth, camera->imageHeight, camera->imageData);
        camera->imageUpdate = false;
    }

    image->render();

    ImGui::End();
}


void Viewer::handleImageSize(void) {

    ImGui::AlignTextToFramePadding();
    ImGui::SetNextItemWidth(200);
    int x = camera->xSize.value;
    if (ImGui::SliderInt("Size X", &x, camera->xSize.min, camera->xSize.max, "%u", ImGuiSliderFlags_AlwaysClamp)) {
        D("X size changed to %d\n", x);
        camera->setImageSize(x, -1);
    }
    ImGui::SameLine();
    ImGui::Text("(%u - %u, +/- %u)", camera->xSize.min, camera->xSize.max, camera->xSize.step);

    ImGui::AlignTextToFramePadding();
    ImGui::SetNextItemWidth(200);
    int y = camera->ySize.value;
    if (ImGui::SliderInt("Size Y", &y, camera->ySize.min, camera->ySize.max, "%u", ImGuiSliderFlags_AlwaysClamp)) {
        D("Y size changed to %d\n", y);
        camera->setImageSize(-1, y);
    }
    ImGui::SameLine();
    ImGui::Text("(%u - %u, +/- %u)", camera->ySize.min, camera->ySize.max, camera->ySize.step);
}

void Viewer::handleImageOffset(void) {

    ImGui::AlignTextToFramePadding();
    ImGui::SetNextItemWidth(200);
    int x = camera->xOffset.value;
    if (ImGui::SliderInt("Offset X", &x, camera->xOffset.min, camera->xOffset.max, "%u", ImGuiSliderFlags_AlwaysClamp)) {
        D("X offset changed to %d\n", x);
        camera->setImageOffset(x, -1);
    }
    ImGui::SameLine();
    ImGui::Text("(%u - %u, +/- %u)", camera->xOffset.min, camera->xOffset.max, camera->xOffset.step);

    ImGui::AlignTextToFramePadding();
    ImGui::SetNextItemWidth(200);
    int y = camera->yOffset.value;
    if (ImGui::SliderInt("Offset Y", &y, camera->yOffset.min, camera->yOffset.max, "%u", ImGuiSliderFlags_AlwaysClamp)) {
        D("Y offset changed to %d\n", y);
        camera->setImageOffset(-1, y);
    }
    ImGui::SameLine();
    ImGui::Text("(%u - %u, +/- %u)", camera->yOffset.min, camera->yOffset.max, camera->yOffset.step);
}

void Viewer::handleImageBinning(void) {

    if (! camera->binningAvailable) {
        ImGui::Text("Binning is not supported");
        return;
    }

    ImGui::AlignTextToFramePadding();
    ImGui::SetNextItemWidth(200);
    int x = camera->xBinning.value;
    if (ImGui::SliderInt("Binning X", &x, camera->xBinning.min, camera->xBinning.max, "%u", ImGuiSliderFlags_AlwaysClamp)) {
        D("X binning changed to %d\n", x);
        camera->setImageBinning(x, -1);
    }
    ImGui::SameLine();
    ImGui::Text("(%u - %u, +/- %u)", camera->xBinning.min, camera->xBinning.max, camera->xBinning.step);

    ImGui::AlignTextToFramePadding();
    ImGui::SetNextItemWidth(200);
    int y = camera->yBinning.value;
    if (ImGui::SliderInt("Binning Y", &y, camera->yBinning.min, camera->yBinning.max, "%u", ImGuiSliderFlags_AlwaysClamp)) {
        D("Y binning changed to %d\n", y);
        camera->setImageBinning(-1, y);
    }
    ImGui::SameLine();
    ImGui::Text("(%u - %u, +/- %u)", camera->yBinning.min, camera->yBinning.max, camera->yBinning.step);
}

void Viewer::handleImagePixelFormat(void) {

    // ImGui::Text("Number of pixels formats %d", camera->numPixelFormats);
    ImGui::AlignTextToFramePadding();
    ImGui::SetNextItemWidth(200);
    int value = camera->pixelFormatCurrent;
    if (ImGui::Combo("Pixel format", &value, camera->pixelFormatStrings, camera->numPixelFormats)) {
        D("pixel format changed to %s (0x%08X)\n", camera->pixelFormatString, camera->pixelFormat);
        camera->setPixelFormat(value);
    }
}

void Viewer::handleImageFrameRate(void) {

    if (! camera->frameRateAvailable) {
        ImGui::Text("Frame rate control is not supported");
        return;
    }

    ImGui::AlignTextToFramePadding();
    ImGui::SetNextItemWidth(200);
    float value = camera->frameRate.value;
    if (ImGui::SliderFloat("Frame rate", &value, camera->frameRate.min, camera->frameRate.max, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
        D("frame rate changed to %f\n", value);
        camera->setFrameRate(value);
    }
    ImGui::SameLine();
    ImGui::Text("(%.3f - %.3f, +/- %.3f)", camera->frameRate.min, camera->frameRate.max, camera->frameRate.step);
}

void Viewer::handleGain(void) {

    if (! camera->gainAvailable) {
        ImGui::Text("Gain control is not supported");
        return;
    }

    // manual gain control
    ImGui::AlignTextToFramePadding();
    ImGui::SetNextItemWidth(200);
    if (camera->gainAutoAvailable && camera->gainAuto) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    }
    float value = camera->gain.value;
    if (ImGui::SliderFloat("Gain", &value, camera->gain.min, camera->gain.max, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
        D("gain changed to %f\n", value);
        camera->setGain(value);
    }
    ImGui::SameLine();
    ImGui::Text("(%.3f - %.3f, +/- %.3f)", camera->gain.min, camera->gain.max, camera->gain.step);
    if (camera->gainAutoAvailable && camera->gainAuto) {
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
    }

    // auto gain control
    if (camera->gainAutoAvailable) {
        ImGui::SameLine();
        bool value = camera->gainAuto;
        if (ImGui::Checkbox("Gain auto", &value)) {
            D("auto gain changed to %d\n", value);
            camera->setGainAuto(value);
        }
    }
}

void Viewer::handleExposure(void) {

    if (! camera->exposureAvailable) {
        ImGui::Text("Exposure time control is not supported");
        return;
    }

    // manual exposure time control
    ImGui::AlignTextToFramePadding();
    ImGui::SetNextItemWidth(200);
    if (camera->exposureAutoAvailable && camera->exposureAuto) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    }
    float value = camera->exposure.value;
    if (ImGui::SliderFloat("Exposure", &value, camera->exposure.min, camera->exposure.max, "%.3f ms", ImGuiSliderFlags_AlwaysClamp)) {
        D("exposure changed to %f\n", value);
        camera->setExposure(value);
    }
    ImGui::SameLine();
    ImGui::Text("(%.3f - %.3f, +/- %.3f)", camera->exposure.min, camera->exposure.max, camera->exposure.step);
    if (camera->exposureAutoAvailable && camera->exposureAuto) {
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
    }

    // auto exposure time control
    if (camera->exposureAutoAvailable) {
        ImGui::SameLine();
        bool value = camera->exposureAuto;
        if (ImGui::Checkbox("Exposure auto", &value)) {
            D("auto exposure changed to %d\n", value);
            camera->setExposureAuto(value);
        }
    }
}
