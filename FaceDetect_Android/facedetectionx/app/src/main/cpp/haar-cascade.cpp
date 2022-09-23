//
// Created by Nguyen Quoc Anh on 19/05/2022.
//

#include "haar-cascade.h"

// Implement detecting face in each frame
std::vector<int> faceDetect(const char* haarCascade_model, const cv::Mat& frame) {

    // Call Haar Cascades model and detect face
    cv::CascadeClassifier detector;
    // "haarcascade_frontalface_alt2" is chosen
    detector.load(haarCascade_model);
    std::vector<cv::Rect> face;
    detector.detectMultiScale(frame, face, 1.1, 3, cv::CASCADE_SCALE_IMAGE, cv::Size(250, 250));

    std::vector<int> faceInfo;
    if (!face.empty()) {
        int left = face[0].x;
        int top = face[0].y - (int) (0.05 * face[0].height);
        int right = face[0].x + face[0].width;
        int bottom = face[0].y + face[0].height + (int) (0.05 * face[0].height);
        faceInfo = {left, top, right, bottom};
    }
    return faceInfo;
}