#include "CRSDK/CrImageDataBlock.h"
#include "CRSDK/CrTypes.h"
#include "OpenCVWrapper.h"
#include <vector>
#include <opencv2/opencv.hpp>

using namespace cv;

OpenCVWrapper::OpenCVWrapper(void)
{}

OpenCVWrapper::~OpenCVWrapper(void)
{}

bool OpenCVWrapper::CompositeImage(const std::vector<CrInt8u>& lvdata, SCRSDK::CrOSDImageDataBlock* osddata, unsigned char* outputdata, CrInt32u* outSize)
{
    using namespace std;

    SCRSDK::CrOSDImageMetaInfo metainfo = osddata->GetMetaInfo();

    if (metainfo.isLvPosExist == SCRSDK::CrIsLvPosExist_Enable) {
        vector<CrInt8u> lvdata_buf(lvdata.size());
        vector<CrInt8u> osddata_buf(osddata->GetImageSize());

        copy(lvdata.data(), lvdata.data() + lvdata.size(), lvdata_buf.begin());
        copy(osddata->GetImageData(), osddata->GetImageData() + osddata->GetImageSize(), osddata_buf.begin());

        Mat lvdata_mat = imdecode(Mat(lvdata_buf), IMREAD_UNCHANGED);
        Mat osddata_mat = imdecode(Mat(osddata_buf), IMREAD_UNCHANGED);

        // UnRotate the OSD image
        switch (metainfo.degree) {
        case 0:
            break;
        case 90:
            cv::rotate(osddata_mat, osddata_mat, ROTATE_90_COUNTERCLOCKWISE); // -90 degree
            break;
        case 270:
            cv::rotate(osddata_mat, osddata_mat, ROTATE_90_CLOCKWISE); // -270 degree = +90 degree
            break;
        }

        int top = 0, bottom = 0, left = 0, right = 0;

        // Resize to the size of the meta information.
        cv::resize(osddata_mat, osddata_mat, cv::Size(metainfo.osdWidth, metainfo.osdHeight));
        cv::resize(lvdata_mat, lvdata_mat, cv::Size(metainfo.lvWidth, metainfo.lvHeight));

        // Match the size of LiveView and OSD images.
        if( osddata_mat.rows != lvdata_mat.rows ) {
            if( osddata_mat.rows > lvdata_mat.rows ) {
                top = (metainfo.lvPosY - (lvdata_mat.rows / 2));
                bottom = ((osddata_mat.rows - metainfo.lvPosY) - ((lvdata_mat.rows / 2) + (lvdata_mat.rows % 2)));
                if (top >= 0 && bottom >= 0) {
                    copyMakeBorder(lvdata_mat, lvdata_mat, top, bottom, 0, 0, BORDER_CONSTANT, Scalar(0, 0, 0));
                }else{
                    return false;
                }
            }else{
                top = (metainfo.lvPosY - (osddata_mat.rows / 2));
                bottom = ((lvdata_mat.rows - metainfo.lvPosY) - ((osddata_mat.rows / 2) + (osddata_mat.rows % 2)));
                if (top >= 0 && bottom >= 0) {
                    copyMakeBorder(osddata_mat, osddata_mat, top, bottom, 0, 0, BORDER_CONSTANT, Scalar(0, 0, 0));
                }else{
                    return false;
                }
            }
        }

        if( osddata_mat.cols != lvdata_mat.cols ) {
            if( osddata_mat.cols > lvdata_mat.cols ) {
                left = (metainfo.lvPosX - (lvdata_mat.cols / 2));
                right = ((osddata_mat.cols - metainfo.lvPosX) - ((lvdata_mat.cols / 2) + (lvdata_mat.cols % 2)));
                if (left >= 0 && right >= 0) {
                    copyMakeBorder(lvdata_mat, lvdata_mat, 0, 0, left, right, BORDER_CONSTANT, Scalar(0, 0, 0));
                }else{
                    return false;
                }
            }else{
                left = (metainfo.lvPosX - (osddata_mat.cols / 2));
                right = ((lvdata_mat.cols - metainfo.lvPosX) - ((osddata_mat.cols / 2) + (osddata_mat.cols % 2)));
                if (left >= 0 && right >= 0) {
                    copyMakeBorder(osddata_mat, osddata_mat, 0, 0, left, right, BORDER_CONSTANT, Scalar(0, 0, 0));
                }else{
                    return false;
                }
            }
        }

        if (4 != osddata_mat.channels()) {
            return false;
        }

        // Composite processing.
        lvdata_mat = OverlayImage(lvdata_mat, osddata_mat);

        switch (metainfo.degree) {
        case 0:
            break;
        case 90:
            cv::rotate(lvdata_mat, lvdata_mat, ROTATE_90_CLOCKWISE);
            break;
        case 270:
            cv::rotate(lvdata_mat, lvdata_mat, ROTATE_90_COUNTERCLOCKWISE);
            break;
        default:
            return false;
        }

        // Convert to JPEG format.
        std::vector<unsigned char> composite_data;
        std::vector<int> params(2);
        params[0] = IMWRITE_JPEG_QUALITY;
        params[1] = 95;
        imencode(".jpg", lvdata_mat, composite_data, params);

        copy(composite_data.begin(), composite_data.end(), outputdata);
        *outSize = static_cast<CrInt32u>(composite_data.size());
    }
    else {
        vector<CrInt8u> osddata_buf(osddata->GetImageSize());
        copy(osddata->GetImageData(), osddata->GetImageData() + osddata->GetImageSize(), osddata_buf.begin());
        Mat osddata_mat = imdecode(Mat(osddata_buf), IMREAD_UNCHANGED);

        // UnRotate the OSD image
        switch (metainfo.degree) {
        case 0:
            break;
        case 90:
            cv::rotate(osddata_mat, osddata_mat, ROTATE_90_COUNTERCLOCKWISE); // -90 degree
            break;
        case 270:
            cv::rotate(osddata_mat, osddata_mat, ROTATE_90_CLOCKWISE); // -270 degree = +90 degree
            break;
        }

        // Resize by OSD Meta Infomation.
        cv::resize(osddata_mat, osddata_mat, cv::Size(metainfo.osdWidth, metainfo.osdHeight));

        // Composite OSD and Filled Black Image.
        Mat black_mat(metainfo.osdHeight, metainfo.osdWidth, CV_8UC3);
        cv::rectangle(black_mat, cv::Rect(0,0, metainfo.osdWidth, metainfo.osdHeight), Scalar(0,0,0), cv::FILLED);
        
        Mat dst;
        dst = OverlayImage(black_mat, osddata_mat);
        osddata_mat = dst;

        switch (metainfo.degree) {
        case 0:
            break;
        case 90:
            cv::rotate(osddata_mat, osddata_mat, ROTATE_90_CLOCKWISE);
            break;
        case 270:
            cv::rotate(osddata_mat, osddata_mat, ROTATE_90_COUNTERCLOCKWISE);
            break;
        default:
            return false;
        }

        // Convert to JPEG format.
        std::vector<unsigned char> jpeg_data;
        std::vector<int> params(2);
        params[0] = IMWRITE_JPEG_QUALITY;
        params[1] = 95;
        imencode(".jpg", osddata_mat, jpeg_data, params);

        copy(jpeg_data.begin(), jpeg_data.end(), outputdata);
        *outSize = static_cast<CrInt32u>(jpeg_data.size());
    }

    return true;
}

cv::Mat OpenCVWrapper::OverlayImage(const cv::Mat& src, const cv::Mat& overlay)
{
    cv::Mat result(src.rows, src.cols, CV_8UC3);

    // Extract the alpha channel
    std::vector<Mat> overlay_mat;
    cv::split(overlay, overlay_mat);
    Mat overlay_alpha = overlay_mat[3];

    int rows = src.rows;
    int cols = src.cols;
    constexpr int channels = 3; // RGB

    const size_t step_overlay_alpha = overlay_alpha.step;
    const size_t elemsize_overlay_alpha = overlay_alpha.elemSize();
    const size_t step_src = src.step;
    const size_t elemsize_src = src.elemSize();
    const size_t step_overlay = overlay.step;
    const size_t elemsize_overlay = overlay.elemSize();
    const size_t step_result = result.step;
    const size_t elemsize_result = result.elemSize();

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            // Manipulate pixels held in a 1D array
            // (vertical offset) + (horizontal offset) + (channel offset)
            uchar alpha = overlay_alpha.data[(r * step_overlay_alpha) + (c * elemsize_overlay_alpha) + 0];
            // Normalize 0~255 values to 0~1
            float alpha_norm = (float)alpha / 255.f;
            // Operates on each RGB channel
            for (int ch = 0; ch < channels; ++ch) {
                uchar spix = src.data[(r * step_src) + (c * elemsize_src) + ch];
                uchar opix = overlay.data[(r * step_overlay) + (c * elemsize_overlay) + ch];
                // If overlay's alpha is 0, then pixel is src's color.
                // Else If overlay's alpha is full, then pixel is overlay's color.
                float dpixf = (float)spix * (1.0f - alpha_norm) + (float)opix * alpha_norm;
                uchar dpix = (dpixf >= 255.0f) ? 255 : (uchar)dpixf;
                result.data[(r * step_result) + (c * elemsize_result) + ch] = dpix;
            }
        }
    }

    return result;
}

void OpenCVWrapper::CreateFillImage(int width, int height, int r, int g, int b, std::vector<CrInt8u>* imgdata)
{
    Mat img_mat(height,width, CV_8UC3);
    cv::rectangle(img_mat, cv::Rect(0, 0, img_mat.cols, img_mat.rows), Scalar(b, g, r), cv::FILLED);

    std::vector<int> params(2);
    params[0] = IMWRITE_JPEG_QUALITY;
    params[1] = 95;
    imencode(".jpg", img_mat, *imgdata, params);

    return;
}
