#pragma once
#include "CrImageDataBlock.h"

#include <opencv2/opencv.hpp>

class OpenCVWrapper
{
public:
    OpenCVWrapper(void);
    ~OpenCVWrapper(void);

    static bool CompositeImage(const std::vector<CrInt8u>& lvdata, SCRSDK::CrOSDImageDataBlock* osddata, unsigned char* outputdata, CrInt32u* outSize);

    static cv::Mat OverlayImage(const cv::Mat& src, const cv::Mat& overlay);
    static void CreateFillImage(int width,int height, int r, int g, int b, std::vector<uchar>* imgdata);
};