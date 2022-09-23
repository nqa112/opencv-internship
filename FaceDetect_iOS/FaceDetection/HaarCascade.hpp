//
//  HaarCascade.hpp
//  FaceDetection
//
//  Created by Nguyen Quoc Anh on 13/09/2022.
//

#ifndef HaarCascade_hpp
#define HaarCascade_hpp

#import <opencv2/opencv.hpp>
#include <stdio.h>

// Declare C++ function
cv::Mat faceDetect(std::string haarCascadePath, const cv::Mat& frame);

#endif /* HaarCascade_hpp */
