//
// Created by Nguyen Quoc Anh on 28/04/2022.
//
#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include "zbar.h"

std::string QRDecoder(cv::Mat image);
