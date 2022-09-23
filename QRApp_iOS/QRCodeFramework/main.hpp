//
//  main.hpp
//  QRCodeFramework
//
//  Created by Nguyen Quoc Anh on 11/12/2021.
//

#ifndef main_hpp
#define main_hpp

#import <opencv2/opencv.hpp>
#import "ZBarSDK.h"

class QRDecoder {
    
public:
    /* Return decoded data after detect QR code */
    std::string decodedData(cv::Mat image);
    
private:
    /* Detect QR code in image */
    cv::Mat detectQR(cv::Mat image);

    /* Preprocess detected QR code to improve decoding accuracy */
    cv::Mat preprocess(cv::Mat image);
};

std::string QRDecoder::decodedData(cv::Mat image) {
    cv::Mat crop = detectQR(image);
    cv::Mat bin = preprocess(crop);
    
    // create zbar scanner
    zbar::ImageScanner scanner;
        
    // configure scanner
    scanner.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);
        
    // wrap image data
    Image wrap(bin.cols, bin.rows, "Y800", (uchar *) bin.data, bin.cols * bin.rows);
        
    // scan image
    scanner.scan(wrap);
        
    // extract results
    Image::SymbolIterator symbol = wrap.symbol_begin();
    std::cout << "\n" << symbol->get_data() << std::endl;
    
    return symbol->get_data();
}

cv::Mat QRDecoder::detectQR(cv::Mat image) {
    cv::Mat grayMat;
    cvtColor(image, grayMat, cv::COLOR_BGR2GRAY);
    
    // detect horizontal and vertical edges
    // compute the Scharr gradient magnitude representation of the images
    cv::Mat gradX, gradY, gradient;
    Sobel(grayMat, gradX, CV_32F, 1, 0, -1);
    Sobel(grayMat, gradY, CV_32F, 0, 1, -1);
    
    // subtract the y-gradient from the x-gradient
    subtract(gradX, gradY, gradient);
    // convert to 8 bit unsigned int because negative pixels equal 0
    convertScaleAbs(gradient, gradient);
    
    // smooth out noise and binarize the image
    cv::Mat blurred, thresh;
    blur(gradient, blurred, cv::Size(5, 5));
    threshold(blurred, thresh, 240, 250, cv::THRESH_BINARY);
    
    // close small gaps in object by using closing kernel
    // image width is larger then height, so use wider kernel
    cv::Mat close, kernel;
    kernel = getStructuringElement(cv::MORPH_RECT, cv::Size(27, 15));
    morphologyEx(thresh, close, cv::MORPH_CLOSE, kernel);

    // perform a series of erosions and dilations
    cv::Mat erosion, expand;
    int iterations;
    erode(close, erosion, iterations = 4);
    dilate(erosion, expand, iterations = 4);
    
    std::vector<std::vector<cv::Point>> contours;
    findContours(expand, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // since QR code is square
    // find biggest square object and crop
    float maxArea = 0;
    cv::Point2f vertices[4];

    for (int i = 0; i < contours.size(); i++){
      cv::RotatedRect box = cv::minAreaRect(contours[i]);
      float ratio = box.size.width / box.size.height;

      if ((ratio > 0.9) && (ratio < 1.1) && (cv::contourArea(contours[i]) > maxArea)){
          maxArea = cv::contourArea(contours[i]);
          box.points(vertices);
      }
    }
    // expand each side of bounding box by 10 pixels
    cv::Mat crop;
    if (maxArea == 0) return image;
    else {
        crop = grayMat(cv::Range(vertices[1].y - 10, vertices[3].y + 10),
        cv::Range(vertices[0].x - 10, vertices[2].x + 10));
    }
    
    return crop;
}

cv::Mat QRDecoder::preprocess(cv::Mat image) {
    cv::Mat res;
    float factor = 500 / image.cols;
    cv::resize(image, res, cv::Size(500, round(factor * image.rows)), cv::INTER_AREA);

    // deblur
    cv::Mat gaussian, unsharp;
    GaussianBlur(res, gaussian, cv::Size(9, 9), 10.0);
    addWeighted(res, 8, gaussian, -7, 0, unsharp);

    // brightness
    cv::Mat brighten;
    float brightness = sum(unsharp)[0] / (255 * unsharp.rows * unsharp.cols);
    float brRatio =  brightness / 0.7; // set minimumBrightness = 70%

    if (brRatio < 1){
      convertScaleAbs(unsharp, brighten, 1.0 / brRatio, 0);
    }
    else brighten = unsharp;

    // contrast
    cv::Mat contrast;
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    clahe->apply(brighten, contrast);

    // binarization
    cv::Mat bin;
    threshold(contrast, bin, 0, 255, cv::THRESH_OTSU);
    
    return bin;
}



#endif /* main_hpp */
