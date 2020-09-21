//
// Created by Darko on 2020/9/19.
//

#include <jni.h>
#include <android/bitmap.h>
#include "WatermarkAlgorithm.h"
#include <opencv/cv.hpp>
//extern "C" JNIEXPORT jstring JNICALL

//#include "bitmap/NImgproc.h"
//#include "bitmap/NatvieUtils.h"
//#include "bitmap/opencvHelp.h"

#import "opencv2/opencv.hpp"
#import <opencv2/core.hpp>
#import "opencv2/highgui.hpp"
#include "WatermarkAlgorithm.h"

using namespace std;
using namespace cv;

#define ASSERT(status, ret)     if (!(status)) { return ret; }
#define ASSERT_FALSE(status)    ASSERT(status, false)


constexpr std::uint32_t hash_str_to_uint32(const char* data)
{
    std::uint32_t h(0);
    for (int i = 0; data && ('\0' != data[i]); i++)
        h = (h << 6) ^ (h >> 26) ^ data[i];
    return h;
}

void Initialize()
{
    s_mapStringValues["algo1"] = algo1;
    s_mapStringValues["algo2"] = algo2;
    s_mapStringValues["algo3"] = algo3;
    s_mapStringValues["algo4"] = algo4;
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

jstring charTojstring(JNIEnv* env, const char* pat) {
    //定义java String类 strClass
    jclass strClass = (env)->FindClass("Ljava/lang/String;");
    //获取String(byte[],String)的构造器,用于将本地byte[]数组转换为一个新String
    jmethodID ctorID = (env)->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    //建立byte数组
    jbyteArray bytes = (env)->NewByteArray(strlen(pat));
    //将char* 转换为byte数组
    (env)->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte*) pat);
    // 设置String, 保存语言类型,用于byte数组转换至String时的参数
    jstring encoding = (env)->NewStringUTF("GB2312");
    //将byte数组转换为java String,并输出
    return (jstring) (env)->NewObject(strClass, ctorID, bytes, encoding);
}

char* jstringToChar(JNIEnv* env, jstring jstr) {
    char* rtn = NULL;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("GB2312");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
    jsize alen = env->GetArrayLength(barr);
    jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);
    if (alen > 0) {
        rtn = (char*) malloc(alen + 1);
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    return rtn;
}

// ------------------------- LSB ------------------------
template<typename _Tp>
vector<_Tp> convertMatToVector(const Mat& mat) {
    return (vector<_Tp>)(mat.reshape(1, 1));
}

template<typename _Tp>
cv::Mat convertVectorToMat(vector<_Tp> v, int channels, int rows) {
    cv::Mat mat = cv::Mat(v);
    cv::Mat dest = mat.reshape(channels, rows).clone();
    dest.convertTo(dest, CV_8U);
    return dest;
}

cv::Mat showImageLSBWatermark(cv::Mat image, int num) {
    cv::Mat dst_img;
    if (num > 7 || num < 0) num = 0;
    vector<int> v = convertMatToVector<int>(image);
    vector<int> u;
    int series = pow(2, num);
    int j = 0, k = 0;
    for (int i = 0; i < v.size() / 3; ++i) {
        j = 0, k = 0;
        v[3 * i] / series % 2 == 0 ? ++j : ++k;
        v[3 * i + 1] / series % 2 == 0 ? ++j : ++k;
        v[3 * i + 2] / series % 2 == 0 ? ++j : ++k;
        u.push_back(j > k ? 0 : 255);
    }
    dst_img = convertVectorToMat(u, 1, image.rows);
    cv::imshow("dst_img", dst_img);
    return dst_img;
}

template<typename _Tp>
vector<_Tp> drawWatermarkOnImage(vector<_Tp> v, vector<_Tp> w, int num) {
    if (num > 7 || num < 0) return v;
    int series = pow(2, num);
    for (int i = 0; i < v.size(); ++i) {
        if (v[i] / series % 2 != 0) v[i] -= series;
        v[i] += pow(2, num) * w[i];
    }
    return v;
}

cv::Mat imageLSB(cv::Mat src_img, cv::Mat mrk_img, int num) {
    if (src_img.size() != mrk_img.size()) {
        cv::copyMakeBorder(mrk_img, mrk_img,
                           0, src_img.rows - mrk_img.rows,
                           0, src_img.cols - mrk_img.cols,
                           cv::BORDER_WRAP, Scalar::all(0));
    }
    cv::Mat dst_img;

    vector<int> src_v = convertMatToVector<int>(src_img);
    vector<int> mrk_v = convertMatToVector<int>(mrk_img);

    for (int i = 0; i < mrk_v.size(); ++i) {
        mrk_v[i] = mrk_v[i] < 120 ? 0 : 1;
    }

    src_v = drawWatermarkOnImage(src_v, mrk_v, num);
    dst_img = convertVectorToMat(src_v, 1, src_img.rows);

    return dst_img;
}

////extern "C" JNIEXPORT CvMat JNICALL
//cv::Mat addWatermark(cv::Mat mat_image_src, cv::Mat watermark, string algo)
//{
//    Initialize();
//
//    Mat mat_image_dst;
//
//    switch (s_mapStringValues[algo]) {
//        case algo1: {
//            int width = mat_image_src.cols;
//            int height = mat_image_src.rows;
//            int level = 20;
//            Mat mat_image_dst;
//            cvtColor(mat_image_src, mat_image_dst, CV_RGBA2RGB, 3);
//            Mat mat_image_clone = mat_image_dst.clone();
//            int xMax = width - level;
//            int yMax = height - level;
//            for (int y = 0; y <= yMax; y += level) {
//                for (int x = 0; x <= xMax; x += level) {
//                    Scalar scalar = Scalar(
//                                           mat_image_clone.at<Vec3b>(y, x)[0],
//                                           mat_image_clone.at<Vec3b>(y, x)[1],
//                                           mat_image_clone.at<Vec3b>(y, x)[2]);
//                    Rect2i mosaicRect = Rect2i(x, y, level, level);
//                    Mat roi = mat_image_dst(mosaicRect);
//                    Mat roiCopy = Mat(mosaicRect.size(), CV_8UC3, scalar);
//                    roiCopy.copyTo(roi);
//                }
//            }
//            return mat_image_dst;
//            break;
//        }
//        case algo2: {
//            Mat mat_image_dst;
//            resize(watermark, watermark, Size(mat_image_src.cols, mat_image_src.rows));
//            addWeighted(mat_image_src, 0.5, watermark, 0.5, 0.0, mat_image_dst);
//            return mat_image_dst;
//            break;
//        }
//        case algo3: {
//            cv::Mat imageROI;
////            resize(watermark, watermark, Size(mat_image_src.cols, mat_image_src.rows));
//            imageROI = mat_image_src(cv::Rect(10,10,watermark.cols,watermark.rows));
//            watermark.copyTo(imageROI);
////            cv::addWeighted(imageROI, 1.0, watermark, 0.3, 0, imageROI);
//            mat_image_dst = mat_image_src;
//            return mat_image_dst;
//        }
//        case algo4: {
//            cv::Mat imageROI;
////            resize(watermark, watermark, Size(mat_image_src.cols, mat_image_src.rows));
//            imageROI = mat_image_src(cv::Rect(10,10,watermark.cols,watermark.rows));
////            watermark.copyTo(imageROI);
//            cv::addWeighted(imageROI, 1.0, watermark, 0.3, 0, imageROI);
//            mat_image_dst = mat_image_src;
//            return mat_image_dst;
//        }
//        default: {
//            cout<<"default!"<<endl;
//            return mat_image_dst;
//            break;
//        }
//    }
//
//    cout<<"end coding."<<endl;
//
//    //    return mat_image_dst;
//}

extern "C" JNIEXPORT void JNICALL
Java_darko_watermark_MainActivity_addWatermark(
                                        JNIEnv* env,
                                        jobject,
                                        jobject jobject_image_src,
                                        jobject jobject_watermark,
                                        jobject jobject_image_dst,
                                        jstring jalgo);

void Java_darko_watermark_MainActivity_addWatermark(JNIEnv *env, jobject, jobject jobject_image_src,
                                                    jobject jobject_watermark,
                                                    jobject jobject_image_dst, jstring jalgo) {

    Initialize();
//    Mat mat_image_dst;

    char* charData = jstringToChar(env, jalgo);
    std::string strAlgo = charData;

    cv::Mat mat_image_src;
    cv::Mat mat_watermark;
    cv::Mat mat_image_dst;

    BitmapToMatrix(env, jobject_image_src, mat_image_src);
    BitmapToMatrix(env, jobject_watermark, mat_watermark);

    switch (s_mapStringValues[strAlgo]) {
        case algo1: {
            int width = mat_image_src.cols;
            int height = mat_image_src.rows;
            int level = 20;
            Mat mat_image_dst;
            cvtColor(mat_image_src, mat_image_dst, CV_RGBA2RGB, 3);
            Mat mat_image_clone = mat_image_dst.clone();
            int xMax = width - level;
            int yMax = height - level;
            for (int y = 0; y <= yMax; y += level) {
                for (int x = 0; x <= xMax; x += level) {
                    Scalar scalar = Scalar(
                            mat_image_clone.at<Vec3b>(y, x)[0],
                            mat_image_clone.at<Vec3b>(y, x)[1],
                            mat_image_clone.at<Vec3b>(y, x)[2]);
                    Rect2i mosaicRect = Rect2i(x, y, level, level);
                    Mat roi = mat_image_dst(mosaicRect);
                    Mat roiCopy = Mat(mosaicRect.size(), CV_8UC3, scalar);
                    roiCopy.copyTo(roi);
                }
            }
//            return mat_image_dst;
            MatrixToBitmap(env, mat_image_dst, jobject_image_dst);
            break;
        }
        case algo2: {
            Mat mat_image_dst;
            resize(mat_watermark, mat_watermark, Size(mat_image_src.cols, mat_image_src.rows));
            addWeighted(mat_image_src, 0.5, mat_watermark, 0.5, 0.0, mat_image_dst);
//            return mat_image_dst;
//            break;

            MatrixToBitmap(env, mat_image_dst, jobject_image_dst);
            break;

        }
        case algo3: {
//            cv::Mat imageROI;
////            resize(watermark, watermark, Size(mat_image_src.cols, mat_image_src.rows));
//            imageROI = mat_image_src(cv::Rect(10,10,mat_watermark.cols,mat_watermark.rows));
//            mat_watermark.copyTo(imageROI);
////            cv::addWeighted(imageROI, 1.0, watermark, 0.3, 0, imageROI);
//            mat_image_dst = mat_image_src;
//
//            MatrixToBitmap(env, mat_image_dst, jobject_image_dst);
            cv::Mat img_src = mat_image_src;
            cv::Mat img_mrk = mat_watermark;
            int num = 1;
            std::vector<cv::Mat> imgs;
            cv::split(img_src, imgs);
            /*∏¯»˝∏ˆÕº≤„∑÷±ÃÌº”£¨“≤ø…“ª—°‘Ò∆‰÷–“ª∏ˆº”ÀÆ”°*/
            for (int i = 0; i < 3; ++i) {
                imgs[i] = imageLSB(imgs[i], img_mrk, num);
            }

            cv::Mat img_dst;
            cv::merge(imgs, img_dst);
            MatrixToBitmap(env, img_dst, jobject_image_dst);
//            return mat_image_dst;
            break;
        }
        case algo4: {
            cv::Mat imageROI;
//            resize(watermark, watermark, Size(mat_image_src.cols, mat_image_src.rows));
            imageROI = mat_image_src(cv::Rect(10,10,mat_watermark.cols,mat_watermark.rows));
//            watermark.copyTo(imageROI);
            cv::addWeighted(imageROI, 1.0, mat_watermark, 0.3, 0, imageROI);
            mat_image_dst = mat_image_src;
//            return mat_image_dst;
            MatrixToBitmap(env, mat_image_dst, jobject_image_dst);
        }
        default: {
            cout<<"default!"<<endl;
//            return mat_image_dst;
            break;
        }
    }

    cout<<"end coding."<<endl;

    //    return mat_image_dst;
}

extern "C"
JNIEXPORT void JNICALL
Java_darko_watermark_MainActivity_nmat2Bitmap(JNIEnv *env, jclass clazz, jlong mnative_ptr,
                                              jobject bitmap) {
    // TODO: implement nmat2Bitmap()
    Mat *mat = reinterpret_cast<Mat *>(mnative_ptr);

    MatrixToBitmap(env, *mat, bitmap);
}extern "C"
JNIEXPORT void JNICALL
Java_darko_watermark_MainActivity_nbitmap2Mat(JNIEnv *env, jclass clazz, jobject bitmap,
                                              jlong native_ptr) {
    // TODO: implement nbitmap2Mat()
    Mat *mat = reinterpret_cast<Mat *>(native_ptr);

    BitmapToMatrix(env, bitmap, *mat);
}