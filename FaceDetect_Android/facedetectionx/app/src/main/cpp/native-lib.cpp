#include <jni.h>
#include <string>
#include <android/log.h>
#include "haar-cascade.h"

extern "C"
JNIEXPORT jobjectArray * JNICALL
Java_com_example_facedetectionx_MainActivity_00024FaceAnalyzer_faceDetect(JNIEnv *env, jobject thiz,
                                     jstring haarCascadePathJstring, jlong matAdd) {
    // Convert jstring to C++ string in modified UTF-8 encoding
    jboolean isCopy;
    const char* modelPath = env->GetStringUTFChars(haarCascadePathJstring, &isCopy);

    // Get Mat from raw address
    cv::Mat &frame = *(cv::Mat*) matAdd;

    // Implement detecting face in each frame and return detected face
    std::vector<int> faceInfo = faceDetect(modelPath, frame);

    // Initialize faceArray from faceInfo in JNI and pass to Kotlin array
    // Create integer jobject
    jclass intClass = env->FindClass("java/lang/Integer");
    jmethodID midInit = env->GetMethodID(intClass, "<init>", "(I)V");
    if (NULL == midInit) return NULL;
    // Initialize faceArray
    jobjectArray faceArray = env->NewObjectArray(faceInfo.size(), intClass, NULL);
    for ( int i = 0; i < faceInfo.size() ; i++ ) {
        jint num = faceInfo[i];
        jobject numObject = env->NewObject(intClass, midInit, num);
        env->SetObjectArrayElement(faceArray, i, numObject);
    }

    // After using string, the ReleaseString-Chars call is necessary, it either frees the copy or unpins the instance
    if (isCopy == JNI_TRUE) {
        env->ReleaseStringUTFChars(haarCascadePathJstring, modelPath);
    }

    return reinterpret_cast<jobjectArray *>(faceArray);
}