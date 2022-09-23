//
//  QRReader.m
//  QRCodeFramework
//
//  Created by Nguyen Quoc Anh on 11/12/2021.
//
#import <opencv2/opencv.hpp>
#import <opencv2/imgcodecs/ios.h>
#import "QRReaderWrapper.h"
#import "main.hpp"

@implementation QRReaderWrapper

+ (UIImage *) QRReader:(UIImage *) image {
    // transform UIImage to cv::Mat
    cv::Mat imgMat;
    UIImageToMat(image, imgMat);
    
    // detect and decode QR code
    try {
        QRDecoder qrdecoder;
        std::string data = qrdecoder.decodedData(imgMat);
        
        // transform frame back to UIImage
        return MatToUIImage(imgMat);
    }
    
    // if detect errors exist, ignore and read the next frame
    catch (...) { return MatToUIImage(imgMat); }
    
    return 0;
}

@end
