//
// Created by Nguyen Quoc Anh on 19/05/2022.
//
#pragma once

#include <iostream>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

std::vector<int> faceDetect(const char* haarCascade_model, const cv::Mat& frame);