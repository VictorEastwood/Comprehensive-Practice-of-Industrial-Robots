/**------------------------------------------*
 * @file Vision_Detector.h
 * @brief 用于检测目标物的颜色和位置
 * @version 0.1
 * @date 2021-06-02
 * @Created by 東木
 *------------------------------------------*/

#ifndef PROJECT_FINAL_VISION_DETECTOR_H
#define PROJECT_FINAL_VISION_DETECTOR_H
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>


#include <set>
#include <vector>
#include <cmath>
using namespace std;
using namespace cv;

//图片来源路径
#define IMAGE_PATH "../Image.jpg"
//图片保存路径
#define IMAGE_SAVE_PATH "../saved_images/"

/**控制台区域，可以改变宏定义开关来选择程序的编译模式 */
  
//读取图片类型 0:摄像头 1:图片
#define READ_IMAGE_TYPE 0

//程序内显示图片 0:关闭 1:开启
#define IMAGE_SHOW 0

//调试模式 0:关闭 1:开启
#define HSV_TUNE 0
//颜色阈值调整 0:关闭 1:开启
#define RED_MASK_TRESHOLD_TUNE 0
#define GREEN_MASK_TRESHOLD_TUNE 0
#define BLUE_MASK_TRESHOLD_TUNE 0
#define YELLOW_MASK_TRESHOLD_TUNE 0
#define ORANGE_MASK_TRESHOLD_TUNE 0
#define PURPLE_MASK_TRESHOLD_TUNE 0
/********************************************** */


typedef enum{
    RED,//红色
    GREEN,//绿色
    BLUE,//蓝色
    YELLOW,//黄色
    ORANGE,//橙色
    PURPLE,//紫色
}cube_color;//方块的颜色(红、绿、蓝、黄、橙、紫)

//方块颜色名字
const string cube_color_name[6] = {
        "RED",//红色
        "GREEN",//绿色
        "BLUE",//蓝色
        "YELLOW",//黄色
        "ORANGE",//橙色
        "PURPLE",//紫色
};
vector<Point> adjustPoints(vector<Point> points);//调整四个点的顺序
float angle_format_90(float angle);//将角度格式化为-45~45度
typedef struct{
    vector<vector<Point>> contours;//轮廓
    vector<vector<Point>> box;//四边形边框
    vector<vector<Point>> hull;//凸包
    vector<Vec4i> hierarchy;//轮廓的层次结构
    cube_color color;//方块的颜色(红、绿、蓝、黄、橙、紫)
    string color_name;//方块的颜色(红、绿、蓝、黄、橙、紫)
    //hsv颜色空间的阈值
    int contour_num;//该颜色方块的数量
    vector<Point> position_pixel;//该颜色方块中心的像素位置(vector)
    vector<Point2f> position_world;//该颜色方块中心的实际位置(vector)
    vector<Point2f> position_base;//该颜色方块中心在jaka机器人基坐标系中的位置(vector)
    vector<vector<Point>> corner_pixel_coordinate;//该颜色方块的四个角点的像素位置
    vector<vector<Point2f>> corner_world_coordinate;//该颜色方块的四个角点的实际位置
    vector<vector<Point2f>> corner_world_coordinate_corrected;//该颜色方块的四个角点的实际位置(校正后)
    vector<Point2f> position_world_corrected;//该颜色方块中心的实际位置(校正后)
    vector<float> angle_world;//该颜色方块的角度
    float area;//该颜色方块的面积
    Mat mask_1;//颜色方块第一个阈值掩膜
    Mat mask_2;//颜色方块第二个阈值掩膜
    Mat mask;//颜色方块掩膜
    Mat morphological_mask;//颜色方块形态学掩膜
    Mat filled_mask;//颜色方块填充掩膜
    Mat last_operated_mask;//最后一次操作的掩膜
}cube_info;

class Vision_Detector{
public:
    //过程处理中的图像
    Mat srcImage;//原图像
    Mat maskedImage;//掩膜图像
    Mat hullImage;//在原图像上绘制凸包的图像
    Mat boxImage;//在原图像上绘制框的图像
    Mat pointImage;//在原图像上绘制点的图像
    Mat coordinateImage;//在原图像上绘制坐标的图像
    Mat correctedImage;//校正后的图像(俯视图)

    //矩阵
    Mat HomoMatrix;//单应性矩阵
    Mat InverseHomoMatrix;//单应性矩阵的逆矩阵

    Mat BaseHomoMatrix;//操作空间到jaka机器人基坐标系的单应性矩阵
    Mat InverseBaseHomoMatrix;//操作空间到jaka机器人基坐标系的单应性矩阵的逆矩阵




    Vision_Detector();
    ~Vision_Detector();
    void read_Image();

    /*Only For Cube Detector*/
    //颜色阈值 0:红色 1:绿色 2:蓝色 3:黄色 4:橙色 5:紫色
    //hsv颜色空间的阈值 0:h_min 1:h_max 2:s_min 3:s_max 4:v_min 5:v_max
//    int cube_hsv_threshold_0[6][6] = {
//            {0, 4, 124, 218, 132, 255},//红色
//            {54, 72, 87, 255, 80, 134},//绿色
//            {102, 110, 161, 217, 99, 158},//蓝色
//            {15, 31, 70, 255, 180, 227},//黄色
//            {9, 11, 121, 230, 135, 255},//橙色
//            {113, 137, 52, 115, 87, 237},//紫色
//    };
//    int cube_hsv_threshold_1[6][6] = {
//            {6, 255, 0, 255, 0, 255},//红色
//            {0, 103, 0, 77, 0, 255},//绿色
//            {0, 87, 0, 255, 0, 255},//蓝色
//            {26, 255, 0, 255, 0, 255},//黄色
//            {14, 255, 0, 255, 0, 255},//橙色
//            {0, 91, 0, 255, 0, 255},//紫色
//    };
    // int cube_hsv_threshold_0[6][6] = {
    //         {0, 4, 124, 218, 132, 255},//红色
    //         {56, 83, 72, 168, 92, 150},//绿色
    //         {99, 114, 130, 235, 115, 191},//蓝色
    //         {16, 60, 88, 190, 189, 235},//黄色
    //         {6, 15, 140, 230, 135, 255},//橙色
    //         {113, 137, 52, 115, 87, 237},//紫色
    // };
    // int cube_hsv_threshold_1[6][6] = {
    //         {6, 255, 0, 255, 0, 255},//红色
    //         {0, 103, 0, 40, 0, 255},//绿色
    //         {0, 85, 0, 255, 0, 255},//蓝色
    //         {32, 255, 0, 255, 0, 255},//黄色
    //         {21, 255, 0, 255, 0, 255},//橙色
    //         {0, 91, 0, 255, 0, 255},//紫色
    // };
    // int cube_hsv_threshold_0[6][6] = {
    //         {0, 5, 84, 255, 216, 255},//红色
    //         {43, 82, 54, 255, 110, 255},//绿色
    //         {95, 116, 115, 255, 134, 255},//蓝色
    //         {21, 60, 60, 255, 215, 255},//黄色
    //         {6, 15, 62, 255, 228, 255},//橙色
    //         {120, 130, 47, 255, 141, 215},//紫色
    // };
    // int cube_hsv_threshold_1[6][6] = {
    //         {14, 255, 0, 255, 0, 255},//红色
    //         {0, 103, 0, 40, 0, 255},//绿色
    //         {0, 85, 0, 255, 0, 255},//蓝色
    //         {32, 255, 0, 255, 0, 255},//黄色
    //         {21, 255, 0, 255, 0, 255},//橙色
    //         {0, 91, 0, 255, 0, 255},//紫色
    // };
    int cube_hsv_threshold_0[6][6] = {
            {0, 4, 26, 255, 218, 255},//红色
            {43, 82, 54, 255, 120, 255},//绿色
            {95, 116, 102, 255, 140, 255},//蓝色
            {21, 60, 60, 255, 199, 255},//黄色
            {6, 15, 62, 255, 214, 255},//橙色
            {120, 130, 47, 255, 141, 215},//紫色
    };
    int cube_hsv_threshold_1[6][6] = {
            {14, 255, 0, 255, 0, 255},//红色
            {0, 103, 0, 40, 0, 255},//绿色
            {0, 85, 0, 255, 0, 255},//蓝色
            {32, 255, 0, 255, 0, 255},//黄色
            {21, 255, 0, 255, 0, 255},//橙色
            {0, 91, 0, 255, 0, 255},//紫色
    };

    const Scalar_<double> cube_color_scalar[6] = {
            Scalar(0, 0, 255),//红色
            Scalar(0, 255, 0),//绿色
            Scalar(255, 0, 0),//蓝色
            Scalar(0, 255, 255),//黄色
            Scalar(0, 165, 255),//橙色
            Scalar(255, 0, 255),//紫色
    };
//     vector<Point> calibrate_points_pixel = {Point(282, 869), Point(1852, 838), Point(1488, 23), Point(548, 24)};
//     vector<Point>  calibrate_points_real = {Point(0, 0), Point(375, 0), Point(375, 350), Point(0, 350)};
    // vector<Point> calibrate_points_pixel = {Point(505, 859), Point(1345, 842), Point(1229, 281), Point(626, 287)};
    vector<Point> calibrate_points_pixel = {Point(468, 808), Point(1310, 804), Point(1213, 201), Point(588, 201)};
    vector<Point> calibrate_points_real = {Point(0, 0), Point(200, 0), Point(200, 200), Point(0, 200)};

    vector<Point> calibrate_points_world = {Point(0, 0), Point(200, 0), Point(200, 200), Point(0, 200)};
    vector<Point> calibrate_points_base = {Point(502, 72), Point(300, 99), Point(275, -99), Point(475, -125)};


    cube_info cube[6];
    void find_homo_matrix();
    Point2f pixel_to_world_coordinate(const cv::Point2f& pixelPoint);
    Point2f world_to_base_coordinate(const cv::Point2f& worldPoint);
    void cube_info_init();
    void cube_create_window();
    void cube_image_show();
    void cube_image_save();
    void cube_image_draw();
    void cube_info_print();
    void cube_detect();
    void cube_split();
    void cube_locate();
    void cube_info_calc();
    void cube_image_correct();
};

#endif //PROJECT_FINAL_VISION_DETECTOR_H
