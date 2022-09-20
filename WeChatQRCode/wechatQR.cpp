#include <opencv2/opencv.hpp>
#include <opencv2/wechat_qrcode.hpp>
#include <iostream>
#include <chrono>

using namespace cv;
using namespace std;
using namespace wechat_qrcode;
using namespace chrono;

int main(int argc, char **argv) {
  // open the default video camera
  VideoCapture cap(0);

  // if not success, exit program
  if (!cap.isOpened()) {
      cout << "\nCamera not found.\n" << endl;
      return -1;
  } 

  while (true) {
    Mat frame, gray;
    // read a new frame from video 
    bool bSuccess = cap.read(frame); 

    if (!bSuccess) {
      cout << "\nCamera is disconnected.\n" << endl;
      break;
    }

    else {     
      try {
        auto start = high_resolution_clock::now();
        // Read gray image for faster decoding
        cvtColor(frame, gray, COLOR_BGR2GRAY);
        
        // detect vertical and horizontal edges
        // compute the Scharr gradient magnitude representation of the images
        Mat gradX, gradY, gradient;
        Sobel(gray, gradX, CV_32F, 1, 0, -1);
        Sobel(gray, gradY, CV_32F, 0, 1, -1);

        // subtract the y-gradient from the x-gradient
        subtract(gradX, gradY, gradient);
        // convert to 8 bit unsigned int because negative pixels equal 0
        convertScaleAbs(gradient, gradient);

        // smooth out noise and binarize the image
        Mat blurred, thresh;
        blur(gradient, blurred, Size(5, 5));
        threshold(blurred, thresh, 240, 250, THRESH_BINARY);

        // close small gaps in object by using closing kernel
        // image width is larger then height, so use wider kernel
        Mat close, kernel;
        kernel = getStructuringElement(MORPH_RECT, Size(27, 15));
        morphologyEx(thresh, close, MORPH_CLOSE, kernel);

        // perform a series of erosions and dilations
        Mat erosion, expand; 
        int iterations;
        erode(close, erosion, iterations = 4);
        dilate(erosion, expand, iterations = 4);

        vector<vector<Point>> contours;
        findContours(expand, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        // since QR code is square 
        // find biggest square object and crop
        float maxArea = 0;
        Point2f vertices[4];

        for (int i = 0; i < contours.size(); i++){
          RotatedRect box = minAreaRect(contours[i]);
          float ratio = box.size.width / box.size.height;

          if ((ratio > 0.9) && (ratio < 1.1) && (contourArea(contours[i]) > maxArea)){
              maxArea = contourArea(contours[i]);
              box.points(vertices);
          }
        }
        // expand each side of bounding box by 10 pixels
        Mat crop;
        if (maxArea != 0){
          crop = gray(Range(vertices[1].y - 10, vertices[3].y + 10), 
                      Range(vertices[0].x - 10, vertices[2].x + 10));
        }
        else{
          cout << "\nDetect failed.\n" << endl;
        }
        
        // resize for faster and more precise decoding
        Mat res;
        float factor = 190.0 / crop.cols;
        resize(crop, res, Size(190, round(factor * crop.rows)), INTER_AREA);

        // deblur 
        Mat gaussian, unsharp;
        GaussianBlur(res, gaussian, Size(9, 9), 10.0);
        addWeighted(res, 8, gaussian, -7, 0, unsharp);

        // brightness
        Mat brighten;
        float brightness = sum(unsharp)[0] / (255 * unsharp.rows * unsharp.cols);
        float brRatio =  brightness / 0.7; // set minimumBrightness = 70%

        if (brRatio < 1){
          convertScaleAbs(unsharp, brighten, 1.0 / brRatio, 0);
        }
        else brighten = unsharp;

        // contrast
        Mat contrast;
        Ptr<CLAHE> clahe = createCLAHE(2.0, Size(8, 8));
        clahe->apply(brighten, contrast);

        // binarization
        Mat bin;
        threshold(contrast, bin, 0, 255, THRESH_OTSU);

        // decode using built-in OpenCV WeChatQRCode
        Ptr<WeChatQRCode> detector;
        vector<Mat> points;
        detector = makePtr<WeChatQRCode>("../detect.prototxt", 
                                        "../detect.caffemodel",
                                        "../sr.prototxt", 
                                        "../sr.caffemodel");

        vector<String> data = detector->detectAndDecode(bin, points);

        if (!data.empty()) {
          for (const string& t : data) cout << "\n" << t << "\n" << endl;

          // display decode time and draw bounding box in captured frame
          auto stop = high_resolution_clock::now();
          duration<double> diff = (stop - start);
          double sec = static_cast<double>(diff.count());
          stringstream stream;
          stream << fixed << setprecision(3) << sec;

          string detectTime = "Time: " + stream.str() + " s";
          putText(frame, detectTime, Point(vertices[0].x - 10, vertices[1].y - 10), FONT_HERSHEY_TRIPLEX, 1, Scalar(190, 80, 40), 2, LINE_AA);

          for (int i = 0; i < 4; i++){
            line(frame, vertices[i], vertices[(i + 1) % 4], Scalar(190, 80, 40), 3, LINE_AA);
          }
        } 
        else cout << "\nDecode failed.\n" << endl;
      }
      catch (...) {continue;}

      // create display window
      string window_name = "QR detector";
      namedWindow(window_name);
      imshow(window_name, frame);

      // if Spacebar is pressed, stop capturing
      if (waitKey(1) == 32){
        cout << "USER QUIT. EXIT PROGRAM...\n" << endl;
        break;
      }
    }
  }

  cap.release();
  destroyAllWindows();

  return 0;
}