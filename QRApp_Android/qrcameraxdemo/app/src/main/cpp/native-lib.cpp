#include <jni.h>
#include <string>
#include <android/log.h>
#include "opencv-zbar.h"

extern "C"
JNIEXPORT void JNICALL
Java_com_example_qrcameraxdemo_MainActivity_00024QRAnalyzer_qrDecoder (
JNIEnv *env, jobject /* this */, jlong matAddr) {
    //    get Mat from raw address
    cv::Mat &mat = *(cv::Mat *) matAddr;
    std::string result = QRDecoder(mat);
    __android_log_print(ANDROID_LOG_INFO, "Result", "%s", result.c_str());
}
