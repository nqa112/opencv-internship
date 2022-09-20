#include <iostream>
#include <filesystem>
#include <string>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;
using namespace __fs::filesystem;

Mat image, grayImage, LapImage;
int ddepth = CV_16S;
int kernel_size = 3;
int num = 0;
Scalar m, dev;

int main( int argc, char** argv )
{   
    string path = "/Users/Quoc_Anh/Downloads/extracted_frame";

    for (const auto & entry : directory_iterator(path))
    {
        // Read the image file
        string str = entry.path();

        image = imread(str);

        // If file extension is .jpg, calculate image variance
        int pos = str.find_last_of(".");

        string ext = str.substr(pos + 1);

        while (ext == "jpg")
        {
            // Check for image open failure
            if (image.empty()) 
            {
                cout << "Could not open or find the image" << endl;

                continue;
            }
            
            cvtColor(image, grayImage, COLOR_BGR2GRAY); //  convert image to gray scale

            Laplacian(grayImage, LapImage, ddepth, kernel_size);

            meanStdDev(LapImage, m, dev, Mat());

            double var = pow(dev[0], 2);

            // If image variance is greater than blur threshold, copy to a folder
            string dst = "/Users/Quoc_Anh/Downloads/not_blur_cpp/Frame" + to_string(num);

            // Threshold 1300 was chosen manually
            // depending on motion blur in video
            while (var >= 1300) 
            {
                copy_file(str, dst);
                
                num ++;

                break;
            }

            break;
        }
    }

    cout << "Extract " << num - 1 << " good frames successfully." << endl;
    
    return 0;
}