//
// Created by Darko on 2020/9/21.
//

#include <android/bitmap.h>

#include <opencv/cv.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

#define ASSERT(status, ret)     if (!(status)) { return ret; }
#define ASSERT_FALSE(status)    ASSERT(status, false)

bool BitmapToMatrix(JNIEnv * env, jobject obj_bitmap, cv::Mat & matrix) {
    void * bitmapPixels;                                            // 保存图片像素数据
    AndroidBitmapInfo bitmapInfo;                                   // 保存图片参数

    ASSERT_FALSE( AndroidBitmap_getInfo(env, obj_bitmap, &bitmapInfo) >= 0);        // 获取图片参数
    ASSERT_FALSE( bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888
                  || bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGB_565 );          // 只支持 ARGB_8888 和 RGB_565
    ASSERT_FALSE( AndroidBitmap_lockPixels(env, obj_bitmap, &bitmapPixels) >= 0 );  // 获取图片像素（锁定内存块）
    ASSERT_FALSE( bitmapPixels );

    if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC4, bitmapPixels);    // 建立临时 mat
        tmp.copyTo(matrix);                                                         // 拷贝到目标 matrix
    } else {
        cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC2, bitmapPixels);
        cv::cvtColor(tmp, matrix, cv::COLOR_BGR5652RGB);
    }

    AndroidBitmap_unlockPixels(env, obj_bitmap);            // 解锁
    return true;
}



bool MatrixToBitmap(JNIEnv * env, cv::Mat & matrix, jobject obj_bitmap) {
    void * bitmapPixels;                                            // 保存图片像素数据
    AndroidBitmapInfo bitmapInfo;                                   // 保存图片参数

    ASSERT_FALSE( AndroidBitmap_getInfo(env, obj_bitmap, &bitmapInfo) >= 0);        // 获取图片参数
    ASSERT_FALSE( bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888
                  || bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGB_565 );          // 只支持 ARGB_8888 和 RGB_565
    ASSERT_FALSE( matrix.dims == 2
                  && bitmapInfo.height == (uint32_t)matrix.rows
                  && bitmapInfo.width == (uint32_t)matrix.cols );                   // 必须是 2 维矩阵，长宽一致
    ASSERT_FALSE( matrix.type() == CV_8UC1 || matrix.type() == CV_8UC3 || matrix.type() == CV_8UC4 );
    ASSERT_FALSE( AndroidBitmap_lockPixels(env, obj_bitmap, &bitmapPixels) >= 0 );  // 获取图片像素（锁定内存块）
    ASSERT_FALSE( bitmapPixels );

    if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC4, bitmapPixels);
        switch (matrix.type()) {
            case CV_8UC1:   cv::cvtColor(matrix, tmp, cv::COLOR_GRAY2RGBA);     break;
            case CV_8UC3:   cv::cvtColor(matrix, tmp, cv::COLOR_RGB2RGBA);      break;
            case CV_8UC4:   matrix.copyTo(tmp);                                 break;
            default:        AndroidBitmap_unlockPixels(env, obj_bitmap);        return false;
        }
    } else {
        cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC2, bitmapPixels);
        switch (matrix.type()) {
            case CV_8UC1:   cv::cvtColor(matrix, tmp, cv::COLOR_GRAY2BGR565);   break;
            case CV_8UC3:   cv::cvtColor(matrix, tmp, cv::COLOR_RGB2BGR565);    break;
            case CV_8UC4:   cv::cvtColor(matrix, tmp, cv::COLOR_RGBA2BGR565);   break;
            default:        AndroidBitmap_unlockPixels(env, obj_bitmap);        return false;
        }
    }
    AndroidBitmap_unlockPixels(env, obj_bitmap);                // 解锁
    return true;
}

JNIEXPORT void JNICALL
Java_com_example_MainActivity_JniBitmapExec(JNIEnv * env, jobject /* this */,
jobject obj_bitmap, jobject obj_bitmapOut)
{
cv::Mat matBitmap;
bool ret = BitmapToMatrix(env, obj_bitmap, matBitmap);          // Bitmap 转 cv::Mat
if (ret == false) {
return;
}

// 对 mat 进行 opencv 处理

ret = MatrixToBitmap(env, matBitmap, obj_bitmapOut);       // Bitmap 转 cv::Mat
if (ret == false) {
return;
}
}