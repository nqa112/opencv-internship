#include <iostream>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/highgui.hpp>

// Implement detecting face in each frame
cv::Mat faceDetect(cv::Mat frame, std::string model) {

    // Resize image
    float factor = 480.0f / (float) frame.cols;
    cv::resize(frame, frame, cv::Size(480, (int) (factor * frame.rows)), cv::INTER_AREA);
    // Convert to grayscale
    cv::Mat grayFrame;
    cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);

    // Call Haar Cascades model and detect face
    cv::CascadeClassifier detector;
    // "haarcascade_frontalface_alt2" is chosen
    detector.load(model);
    // scaleFactor=1.1, minNeighbors=7, minSize=(30, 30)
    std::vector<cv::Rect> face;
    detector.detectMultiScale(grayFrame, face, 1.1, 7, cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30));

    // Draw detected rectangular bounding box
    for ( size_t i = 0; i < face.size(); i++ ) {
        int x = face[i].x; 
        int y = face[i].y;
        int h = face[i].height;
        int w = face[i].width;
        cv::rectangle(frame, cv::Point(x, y - (int) (0.05 * h)), 
                             cv::Point(x + w, y + h + (int) (0.05 * h)), cv::Scalar(0, 255, 0), 2);
    }

    return frame;
}


// Read and process frame by frame
int main(int argc, char **argv) {

    // Declare Haar Cascade .xml file located in OpenCV install location
    // Use cv::samples::findFile to scan file in build or install directory
    std::string haarCascade_model = cv::samples::findFile("haarcascades/haarcascade_frontalface_alt2.xml");

    // Define video capture object with camera device index
    cv::VideoCapture cap(0);

    // If camera fails to open, exit program
    if (!cap.isOpened()) {
        std::cout << "Camera not found." << std::endl;
        return -1;
    }

    while (true) {
        cv::Mat frame;
        // read a new frame from video 
        bool ret = cap.read(frame); 

        if (!ret) {
            std::cout << "Can't receive frame (stream end?). Exiting ..." << std::endl;
            break;
        }
        else {     
            // Process frame here
            // Call faceDetect() defined earlier
            cv::Mat output = faceDetect(frame, haarCascade_model);
            
            // Create display window
            std::string window_name = "Face detection";
            cv::namedWindow(window_name);
            cv::imshow(window_name, output);

            // If Spacebar is pressed, stop capturing
            if (cv::waitKey(1) == 32) {
                std::cout << "Quit button is pressed, closing program..." << std::endl;
                break;
            }
        } 
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}