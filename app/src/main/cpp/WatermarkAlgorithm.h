//
// Created by Darko on 2020/9/19.
//

#ifndef WATERMARK_WATERMARKALGORITHM_H
#define WATERMARK_WATERMARKALGORITHM_H

//#endif //WATERMARK_WATERMARKALGORITHM_H

#import "opencv2/opencv.hpp"
#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;


//class AlgorithmList {
//    string algo1,
//    algo2,
//    algo3,
//    algo4;
//};
//enum AlgorithmList {
//    algo1,
//    algo2,
//    algo3,
//    algo4
//};
enum AlgorithmList
{
    defaultAlgo,
    algo1,
    algo2,
    algo3,
    algo4
};

static std::map<std::string, AlgorithmList> s_mapStringValues;
static void Initialize();

void addSomething(cv::Mat mat_image_src);

//cv::Mat addWatermark(cv::Mat mat_image_src, cv::Mat watermark, string algo);

#endif /* WatermarkAlgorithm_hpp */