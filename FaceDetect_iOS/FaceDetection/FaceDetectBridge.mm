//
//  FaceDetectBridge.mm
//  FaceDetection
//
//  Created by Nguyen Quoc Anh on 09/09/2022.
//

#import <opencv2/opencv.hpp>
#import <opencv2/imgcodecs/ios.h>
#import "FaceDetectBridge.h"
#import "HaarCascade.hpp"

@implementation FaceDetectBridge

+ (NSString *) openCVVersionString {
    return [NSString stringWithFormat:@"Initializing OpenCV Version %s ...",  CV_VERSION];
}

// Process UIImage for face detection
+ (UIImage *) detectFace: (UIImage *) image {
    // Transform UIImage to cv::Mat
    cv::Mat imgMat;
    UIImageToMat(image, imgMat);
    
    // Access HaarCascade file in "res" folder
    // and return its directory
    NSString *haarCascadeFile = [[NSBundle mainBundle] pathForResource:@"haarcascade_frontalface_alt2" ofType:@"xml"];
    
    // If file is found successfully, process the frame
    // else report the error
    if (haarCascadeFile.length != 0) {
        std::string haarCascadePath;
        haarCascadePath = [haarCascadeFile UTF8String];
        
        // Detect face and draw bounding box onto the frame
        cv::Mat faceBoundingBox = faceDetect(haarCascadePath, imgMat);
        return MatToUIImage(faceBoundingBox);
    }
    else {
        return MatToUIImage(imgMat);
    }
}

@end
