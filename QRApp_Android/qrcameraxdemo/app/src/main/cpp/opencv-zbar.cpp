//
// Created by Nguyen Quoc Anh on 28/04/2022.
//
#include "opencv-zbar.h"

class QRDetect_Decode {

public:
    /* Return decoded data after detect QR code */
    std::string decodedData(cv::Mat image);

private:
    /* Detect QR code in image */
    cv::Mat detectQR(cv::Mat grayMat);

    /* Preprocess detected QR code to improve decoding accuracy */
    cv::Mat preprocess(cv::Mat crop);
};

// Suppress OpenCV error message
// If errors from cv::exception are handled, displaying errors is not necessary
// https://stackoverflow.com/questions/17567808/how-to-suppress-opencv-error-message
int handleError( int status, const char* func_name,
                 const char* err_msg, const char* file_name,
                 int line, void* userdata )
{
    //Do nothing -- will suppress console output
    return 0;   //Return value is not used
}

cv::Mat QRDetect_Decode::detectQR(cv::Mat grayMat) {
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

    // perform a series of erosion and dilation
    cv::Mat erosion, expand;
    erode(close, erosion, 4);
    dilate(erosion, expand, 4);

    std::vector<std::vector<cv::Point>> contours;
    findContours(expand, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // since QR code is square
    // find biggest square object and crop
    float maxArea = 0.0f;
    cv::Point2f vertices[4];

    for (int i = 0; i < contours.size(); i++){
        cv::RotatedRect box = cv::minAreaRect(contours[i]);
        float ratio = box.size.width / box.size.height;

        if ((ratio > 0.9f) && (ratio < 1.1f) && (cv::contourArea(contours[i]) > maxArea)){
            maxArea = cv::contourArea(contours[i]);
            box.points(vertices);
        }
    }

    // expand each side of bounding box by 10 pixels
    cv::Mat crop;
    // Suppress assertion errors, particularly matrix dimensions errors
    // which are due to OpenCV failed to detect QR bounding box
    cv::redirectError(handleError);
    try {
        if (maxArea == 0) return grayMat;
        else {
            crop = grayMat(cv::Range(vertices[1].y - 10, vertices[3].y + 10),
                           cv::Range(vertices[0].x - 10, vertices[2].x + 10));
        }
        return crop;
    }
    catch (cv::Exception & e) {
        return grayMat;
    }
}


cv::Mat QRDetect_Decode::preprocess(cv::Mat crop) {
    cv::Mat res;
    float factor = 500.0f / (float) crop.cols;
    cv::resize(crop, res, cv::Size(500, (int)(factor * crop.rows)), cv::INTER_AREA);

    // deblur
    cv::Mat gaussian, unsharp;
    GaussianBlur(res, gaussian, cv::Size(9, 9), 10.0);
    addWeighted(res, 8, gaussian, -7, 0, unsharp);

    // brightness
    cv::Mat brighten;
    float brightness = sum(unsharp)[0] / (255 * unsharp.rows * unsharp.cols);
    float brRatio =  brightness / 0.7f; // set minimumBrightness = 70%

    if (brRatio < 1) {
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


std::string QRDetect_Decode::decodedData(cv::Mat image) {
    cv::Mat crop = detectQR(image);
    cv::Mat bin = preprocess(crop);

    // create zbar scanner
    zbar::ImageScanner scanner;

    // configure scanner
    scanner.set_config(zbar::ZBAR_QRCODE, zbar::ZBAR_CFG_ENABLE, 1);

    // wrap image data
    zbar::Image wrap(bin.cols, bin.rows, "Y800", (uchar *) bin.data, bin.cols * bin.rows);

    // scan image
    scanner.scan(wrap);

    // extract results
    zbar::Image::SymbolIterator symbol = wrap.symbol_begin();

    return symbol->get_data();
}

/** FUNCTIONS */
std::string QRDecoder(cv::Mat image) {
    QRDetect_Decode detectAndDecode;
    std::string result = detectAndDecode.decodedData(image);
    return result;
}