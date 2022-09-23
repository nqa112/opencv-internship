//
//  HaarCascade.cpp
//  FaceDetection
//
//  Created by Nguyen Quoc Anh on 13/09/2022.
//

#import "HaarCascade.hpp"

cv::Mat faceDetect(std::string haarCascadePath, const cv::Mat& frame) {
    // Convert image to grayscale for detection
    cv::Mat grayFrame;
    cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);
    
    // Convert image to correct colorspace for drawing
    cv::Mat drawFrame;
    cv::cvtColor(frame, drawFrame, cv::COLOR_RGBA2RGB);
    
    // Load "haarcascade_frontalface_alt2.xml" file
    cv::CascadeClassifier detector;
    detector.load(haarCascadePath);
    
    // Do detection
    // and return results to face instance
    std::vector<cv::Rect> face;
    detector.detectMultiScale(grayFrame, face, 1.15, 6, cv::CASCADE_SCALE_IMAGE, cv::Size(200, 200));
    // Release Mat to set memory free
    grayFrame.release();

    // Draw detected rectangular bounding box
    if (!face.empty()) {
        for ( size_t i = 0; i < face.size(); i++ ) {
            int x = face[i].x;
            int y = face[i].y;
            int h = face[i].height;
            int w = face[i].width;
            cv::rectangle(drawFrame,
                          cv::Point(x + (int) (0.08 * w), y - (int) (0.05 * h)),
                          cv::Point(x + w - (int) (0.08 * w) , y + h + (int) (0.05 * h)),
                          cv::Scalar(0, 255, 0), 2);
        }
    }
    // Mirror frame to get natural display
    cv::flip(drawFrame, drawFrame, 1);
    return drawFrame;
}
