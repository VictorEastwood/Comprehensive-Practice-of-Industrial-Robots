/**---------------------------------------*
 * @file Vision_Detector.cpp
 * @brief Vision_Detector class
 * @version 0.1
 * @date 2022-06-24
 * @Created by 東木
 *-----------------------------------------*/
#include "Vision_Detector.h"
/**
 * @brief 构造函数
 */
Vision_Detector :: Vision_Detector() {

}


/**
 * @brief 析构函数
 */
Vision_Detector :: ~Vision_Detector() {

}


/**
 * @name read_Image
 * @brief 读取图片
 * @param void
 */
void Vision_Detector :: read_Image()
{
    #if READ_IMAGE_TYPE == 0
    cv::VideoCapture cap(0); // 打开默认摄像头
    cap.set(CAP_PROP_FRAME_WIDTH,1920);
    cap.set(CAP_PROP_FRAME_HEIGHT,1080);

    if (!cap.isOpened()) {
        std::cerr << "Error: 摄像头无法打开" << std::endl;
        return ;
    }

    while (true) {
        cap >> srcImage; // 从摄像头读取一帧图像到srcImage
        if (srcImage.empty()) {
            std::cerr << "Error: 读取图像失败" << std::endl;
            break; // 如果读取失败，退出循环
        }

        cv::imshow("Captured Image", srcImage); // 显示图像

        // 检查是否按下了ESC键（ASCII值为27），如果是，则退出循环
        if (cv::waitKey(1) == 27) {
            break;
        }
    }

    // 释放摄像头资源并关闭所有OpenCV创建的窗口
    cap.release();
    cv::destroyAllWindows();
    #elif READ_IMAGE_TYPE == 1
        srcImage = imread(IMAGE_PATH);
        if (srcImage.empty()) {
            cout << "图片读取失败" << endl;
            return;
        }
    #endif
}

/**
 * @name find_homo_matrix
 * @brief 寻找单应矩阵,获得平面和相机的对应关系
 */
void Vision_Detector ::find_homo_matrix()
{
    //寻找单应矩阵
    HomoMatrix = findHomography(calibrate_points_pixel, calibrate_points_real);

    //寻找操作空间坐标系到基座坐标系的单应矩阵

    BaseHomoMatrix = findHomography(calibrate_points_world,calibrate_points_base);
    //求逆
//    InverseHomoMatrix = HomoMatrix.inv();


}

/**
 * @name cube_info_init
 * @brief 初始化cube_info
 * @param void
 */
void Vision_Detector :: cube_info_init()
{
    for(int i = 0; i < 6; i++)
    {
        cube[i].color = (cube_color)i;
        cube[i].color_name = cube_color_name[i];
        cube[i].contour_num = 0;
        cube[i].position_pixel.clear();
        cube[i].position_world.clear();
        cube[i].corner_pixel_coordinate.clear();
        cube[i].area = 0;
    }
    find_homo_matrix();
    cube_create_window();
}

/**
 * @name pixel_to_world_coordinate
 * @brief Converts a point from pixel coordinates to world coordinates.
 * @param pixelPoint The point in pixel coordinates.
 * @return The point in world coordinates.
 */
Point2f Vision_Detector::pixel_to_world_coordinate(const cv::Point2f& pixelPoint)
{
    // Convert the pixel coordinates to homogeneous coordinates
    cv::Mat temp = (cv::Mat_<double>(3, 1) << pixelPoint.x, pixelPoint.y, 1);

    // Apply the homography transformation
    temp = HomoMatrix * temp;

    // Convert back to regular 2D coordinates
    temp = temp / temp.at<double>(2, 0);

    // Return the transformed point
    return cv::Point2f(temp.at<double>(0, 0), temp.at<double>(1, 0));
}


Point2f Vision_Detector::world_to_base_coordinate(const cv::Point2f& worldPoint)
{
        // Convert the pixel coordinates to homogeneous coordinates
    cv::Mat temp = (cv::Mat_<double>(3, 1) << worldPoint.x, worldPoint.y, 1);

    // Apply the homography transformation
    temp = BaseHomoMatrix * temp;

    // Convert back to regular 2D coordinates
    temp = temp / temp.at<double>(2, 0);

    // Return the transformed point
    return cv::Point2f(temp.at<double>(0, 0), temp.at<double>(1, 0));
}





/**
* @brief 打印cube_info
 * @param void
 * @return void
 */
void Vision_Detector :: cube_info_print()
{
    cout<<"*******************************************************"<<endl;
    for(int i = 0; i < 6; i++)
    {
        cout << "----------cube[" << cube[i].color_name << "]info------ " <<endl;
        // cout << "cube[" << i << "].color = " << cube[i].color << endl;
        // cout << "cube[" << i << "].color_name = " << cube[i].color_name << endl;
        cout << "cube[" << cube[i].color_name << "].num = " << cube[i].contour_num << endl;
        for(int j = 0; j < cube[i].contour_num; j++)
        {
            cout<<""<<endl;
            // cout << "cube[" << i << "].position_pixel[" << j << "] = " << cube[i].position_pixel[j] << endl;
            cout << "cube[" << cube[i].color_name << "].position_world[" << j << "] = " << cube[i].position_world[j] << endl;
            // cout << "cube[" << i << "].corner_pixel_coordinate[" << j << "] = " << cube[i].corner_pixel_coordinate[j] << endl;
            // cout << "cube[" << i << "].corner_world_coordinate[" << j << "] = " << cube[i].corner_world_coordinate[j] << endl;
            //输出基座坐标
            cout << "cube[" << cube[i].color_name << "].position_base[" << j << "] = " << cube[i].position_base[j] << endl;
            //输出旋转角度
            cout<<"cube["<<cube[i].color_name<<"].angle_world["<<j<<"] = "<<cube[i].angle_world[j]<<endl;
        }
    }
    cout<<"*******************************************************"<<endl;
}

/**
 * @name cube_create_window
 * @brief 创建窗口
 * @param void
 */
void Vision_Detector :: cube_create_window()
{
    //    //红色方块窗口
    #if RED_MASK_TRESHOLD_TUNE ==1
        namedWindow("redMask", WINDOW_AUTOSIZE);
        namedWindow("redMask_1", WINDOW_AUTOSIZE);
        createTrackbar("h_min", "redMask_1", &cube_hsv_threshold_0[0][0], 255);
        createTrackbar("h_max", "redMask_1", &cube_hsv_threshold_0[0][1], 255);
        createTrackbar("s_min", "redMask_1", &cube_hsv_threshold_0[0][2], 255);
        createTrackbar("s_max", "redMask_1", &cube_hsv_threshold_0[0][3], 255);
        createTrackbar("v_min", "redMask_1", &cube_hsv_threshold_0[0][4], 255);
        createTrackbar("v_max", "redMask_1", &cube_hsv_threshold_0[0][5], 255);
        namedWindow("redMask_2", WINDOW_AUTOSIZE);
        createTrackbar("h_min", "redMask_2", &cube_hsv_threshold_1[0][0], 255);
        createTrackbar("h_max", "redMask_2", &cube_hsv_threshold_1[0][1], 255);
        createTrackbar("s_min", "redMask_2", &cube_hsv_threshold_1[0][2], 255);
        createTrackbar("s_max", "redMask_2", &cube_hsv_threshold_1[0][3], 255);
        createTrackbar("v_min", "redMask_2", &cube_hsv_threshold_1[0][4], 255);
        createTrackbar("v_max", "redMask_2", &cube_hsv_threshold_1[0][5], 255);
    #endif
    #if GREEN_MASK_TRESHOLD_TUNE ==1
        namedWindow("greenMask", WINDOW_AUTOSIZE);
        namedWindow("greenMask_1", WINDOW_AUTOSIZE);
        createTrackbar("h_min", "greenMask_1", &cube_hsv_threshold_0[1][0], 255);
        createTrackbar("h_max", "greenMask_1", &cube_hsv_threshold_0[1][1], 255);
        createTrackbar("s_min", "greenMask_1", &cube_hsv_threshold_0[1][2], 255);
        createTrackbar("s_max", "greenMask_1", &cube_hsv_threshold_0[1][3], 255);
        createTrackbar("v_min", "greenMask_1", &cube_hsv_threshold_0[1][4], 255);
        createTrackbar("v_max", "greenMask_1", &cube_hsv_threshold_0[1][5], 255);
        namedWindow("greenMask_2", WINDOW_AUTOSIZE);
        createTrackbar("h_min", "greenMask_2", &cube_hsv_threshold_1[1][0], 255);
        createTrackbar("h_max", "greenMask_2", &cube_hsv_threshold_1[1][1], 255);
        createTrackbar("s_min", "greenMask_2", &cube_hsv_threshold_1[1][2], 255);
        createTrackbar("s_max", "greenMask_2", &cube_hsv_threshold_1[1][3], 255);
        createTrackbar("v_min", "greenMask_2", &cube_hsv_threshold_1[1][4], 255);
        createTrackbar("v_max", "greenMask_2", &cube_hsv_threshold_1[1][5], 255);
    #endif
    #if BLUE_MASK_TRESHOLD_TUNE ==1
        namedWindow("blueMask", WINDOW_AUTOSIZE);
        namedWindow("blueMask_1", WINDOW_AUTOSIZE);
        createTrackbar("h_min", "blueMask_1", &cube_hsv_threshold_0[2][0], 255);
        createTrackbar("h_max", "blueMask_1", &cube_hsv_threshold_0[2][1], 255);
        createTrackbar("s_min", "blueMask_1", &cube_hsv_threshold_0[2][2], 255);
        createTrackbar("s_max", "blueMask_1", &cube_hsv_threshold_0[2][3], 255);
        createTrackbar("v_min", "blueMask_1", &cube_hsv_threshold_0[2][4], 255);
        createTrackbar("v_max", "blueMask_1", &cube_hsv_threshold_0[2][5], 255);
        namedWindow("blueMask_2", WINDOW_AUTOSIZE);
        createTrackbar("h_min", "blueMask_2", &cube_hsv_threshold_1[2][0], 255);
        createTrackbar("h_max", "blueMask_2", &cube_hsv_threshold_1[2][1], 255);
        createTrackbar("s_min", "blueMask_2", &cube_hsv_threshold_1[2][2], 255);
        createTrackbar("s_max", "blueMask_2", &cube_hsv_threshold_1[2][3], 255);
        createTrackbar("v_min", "blueMask_2", &cube_hsv_threshold_1[2][4], 255);
        createTrackbar("v_max", "blueMask_2", &cube_hsv_threshold_1[2][5], 255);
    #endif
    #if YELLOW_MASK_TRESHOLD_TUNE ==1
        namedWindow("yellowMask", WINDOW_AUTOSIZE);
        namedWindow("yellowMask_1", WINDOW_AUTOSIZE);
        createTrackbar("h_min", "yellowMask_1", &cube_hsv_threshold_0[3][0], 255);
        createTrackbar("h_max", "yellowMask_1", &cube_hsv_threshold_0[3][1], 255);
        createTrackbar("s_min", "yellowMask_1", &cube_hsv_threshold_0[3][2], 255);
        createTrackbar("s_max", "yellowMask_1", &cube_hsv_threshold_0[3][3], 255);
        createTrackbar("v_min", "yellowMask_1", &cube_hsv_threshold_0[3][4], 255);
        createTrackbar("v_max", "yellowMask_1", &cube_hsv_threshold_0[3][5], 255);
        namedWindow("yellowMask_2", WINDOW_AUTOSIZE);
        createTrackbar("h_min", "yellowMask_2", &cube_hsv_threshold_1[3][0], 255);
        createTrackbar("h_max", "yellowMask_2", &cube_hsv_threshold_1[3][1], 255);
        createTrackbar("s_min", "yellowMask_2", &cube_hsv_threshold_1[3][2], 255);
        createTrackbar("s_max", "yellowMask_2", &cube_hsv_threshold_1[3][3], 255);
        createTrackbar("v_min", "yellowMask_2", &cube_hsv_threshold_1[3][4], 255);
        createTrackbar("v_max", "yellowMask_2", &cube_hsv_threshold_1[3][5], 255);
    #endif
    #if ORANGE_MASK_TRESHOLD_TUNE ==1
        namedWindow("orangeMask", WINDOW_AUTOSIZE);
        namedWindow("orangeMask_1", WINDOW_AUTOSIZE);
        createTrackbar("h_min", "orangeMask_1", &cube_hsv_threshold_0[4][0], 255);
        createTrackbar("h_max", "orangeMask_1", &cube_hsv_threshold_0[4][1], 255);
        createTrackbar("s_min", "orangeMask_1", &cube_hsv_threshold_0[4][2], 255);
        createTrackbar("s_max", "orangeMask_1", &cube_hsv_threshold_0[4][3], 255);
        createTrackbar("v_min", "orangeMask_1", &cube_hsv_threshold_0[4][4], 255);
        createTrackbar("v_max", "orangeMask_1", &cube_hsv_threshold_0[4][5], 255);
        namedWindow("orangeMask_2", WINDOW_AUTOSIZE);
        createTrackbar("h_min", "orangeMask_2", &cube_hsv_threshold_1[4][0], 255);
        createTrackbar("h_max", "orangeMask_2", &cube_hsv_threshold_1[4][1], 255);
        createTrackbar("s_min", "orangeMask_2", &cube_hsv_threshold_1[4][2], 255);
        createTrackbar("s_max", "orangeMask_2", &cube_hsv_threshold_1[4][3], 255);
        createTrackbar("v_min", "orangeMask_2", &cube_hsv_threshold_1[4][4], 255);
        createTrackbar("v_max", "orangeMask_2", &cube_hsv_threshold_1[4][5], 255);
    #endif
    #if PURPLE_MASK_TRESHOLD_TUNE ==1
        namedWindow("purpleMask", WINDOW_AUTOSIZE);
        namedWindow("purpleMask_1", WINDOW_AUTOSIZE);
        createTrackbar("h_min", "purpleMask_1", &cube_hsv_threshold_0[5][0], 255);
        createTrackbar("h_max", "purpleMask_1", &cube_hsv_threshold_0[5][1], 255);
        createTrackbar("s_min", "purpleMask_1", &cube_hsv_threshold_0[5][2], 255);
        createTrackbar("s_max", "purpleMask_1", &cube_hsv_threshold_0[5][3], 255);
        createTrackbar("v_min", "purpleMask_1", &cube_hsv_threshold_0[5][4], 255);
        createTrackbar("v_max", "purpleMask_1", &cube_hsv_threshold_0[5][5], 255);
        namedWindow("purpleMask_2", WINDOW_AUTOSIZE);
        createTrackbar("h_min", "purpleMask_2", &cube_hsv_threshold_1[5][0], 255);
        createTrackbar("h_max", "purpleMask_2", &cube_hsv_threshold_1[5][1], 255);
        createTrackbar("s_min", "purpleMask_2", &cube_hsv_threshold_1[5][2], 255);
        createTrackbar("s_max", "purpleMask_2", &cube_hsv_threshold_1[5][3], 255);
        createTrackbar("v_min", "purpleMask_2", &cube_hsv_threshold_1[5][4], 255);
        createTrackbar("v_max", "purpleMask_2", &cube_hsv_threshold_1[5][5], 255);
    #endif

}




/**
 * @name cube_detect
 * @brief 方块检测任务
 * @param void
*/
void Vision_Detector ::cube_detect()
{
    read_Image();
    cube_split();//分割方块顶面

#if HSV_TUNE == 0
    cube_locate();//定位方块
    cube_image_draw();//绘制图片
    cube_info_calc();//计算cube_info
    cube_image_correct();//图像矫正
#endif
    cube_image_show();//显示图片
}

/**
 * @name cube_split
 * @brief 分割方块顶面使之成为掩膜
 * @param void
 */
void Vision_Detector :: cube_split()
{
    Mat srcImage_copy = srcImage.clone();
    Mat hsv_image;
    Mat mask_image;
    //设置 mask_image 为全黑
    mask_image = Mat::zeros(srcImage.size(), CV_8UC1);
    cvtColor(srcImage_copy, hsv_image, COLOR_BGR2HSV);
    
    for(uint8_t i = 0;i<6;i++)
    {
        inRange(hsv_image, Scalar(cube_hsv_threshold_0[i][0], cube_hsv_threshold_0[i][2], cube_hsv_threshold_0[i][4]),
                Scalar(cube_hsv_threshold_0[i][1], cube_hsv_threshold_0[i][3], cube_hsv_threshold_0[i][5]), cube[i].mask_1);
        inRange(hsv_image, Scalar(cube_hsv_threshold_1[i][0], cube_hsv_threshold_1[i][2], cube_hsv_threshold_1[i][4]),
                Scalar(cube_hsv_threshold_1[i][1], cube_hsv_threshold_1[i][3], cube_hsv_threshold_1[i][5]), cube[i].mask_2);
        cube[i].mask = cube[i].mask_1 - cube[i].mask_2;

        //形态学处理
        Mat morphed_mask = cube[i].mask.clone();
        Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
        morphologyEx(morphed_mask, morphed_mask, MORPH_CLOSE, element);
//        morphologyEx(morphed_mask, morphed_mask, MORPH_OPEN, element);
        cube[i].morphological_mask = morphed_mask;

        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(morphed_mask, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);
        for (int i = 0; i < contours.size(); i++)
        {
            double area = contourArea(contours[i]);
            if (area < 1000)
            {
                if (hierarchy[i][3] < 0 && area < 1000)
                {
                    drawContours(morphed_mask, contours, i, Scalar(0), FILLED);
                }
                if (hierarchy[i][3] >= 0 && area < 1000)
                {
                    drawContours(morphed_mask, contours, i, Scalar(255), FILLED);
                }
            }
        }
        cube[i].filled_mask = morphed_mask;
        cube[i].last_operated_mask = morphed_mask;
        //将掩膜覆盖到mask_image上
        mask_image = mask_image + morphed_mask;
    }
    maskedImage = mask_image;

}
/**
 * @name cube_locate
 * @brief 定位方块,寻找角点和中心点的像素坐标
 * @param void
 */
void Vision_Detector :: cube_locate()
{
    for(int i = 0;i<6;i++)
    {
        // 寻找边框
        findContours(cube[i].last_operated_mask, cube[i].contours, cube[i].hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);
        cube[i].contour_num = cube[i].contours.size();
        if(cube[i].contour_num == 0)
        {
            continue;
        }
        else
        {
            //寻找凸包
            vector<vector<Point>> hulls(cube[i].contour_num);
            for (int j = 0; j < cube[i].contour_num; j++)
            {
                convexHull(cube[i].contours[j], hulls[j], true);

            }
            cube[i].hull = hulls;
            //寻找最小外接四边形
            vector<vector<Point>> boxes(cube[i].contour_num);

            for (int j = 0; j < cube[i].contour_num; j++)
            {
                double epsilon = 0.1 * arcLength(hulls[j], true);
                approxPolyDP(hulls[j], boxes[j], epsilon, true);

            }
            cube[i].box = boxes;
            //创建点集
            vector<vector<Point>> points(cube[i].contour_num);
            for (int j = 0; j < cube[i].contour_num; j++)
            {
                vector<Point> calculatedPoints = boxes[j];
                vector<Point> adjustedPoints;
                // 调整四个角点的顺序
                adjustedPoints = adjustPoints(calculatedPoints);
                //记录四个角点的像素位置
                points[j] = adjustedPoints;
            }
            cube[i].corner_pixel_coordinate = points;
            //绘制点
            Mat pointedImage = srcImage.clone();
            for(int j = 0;j<cube[i].contour_num;j++)
            {
                Point center  = (cube[i].corner_pixel_coordinate[j][0] + cube[i].corner_pixel_coordinate[j][1] + cube[i].corner_pixel_coordinate[j][2] + cube[i].corner_pixel_coordinate[j][3]) / 4;
                cube[i].position_pixel.push_back(center);
            }
        }

    }
}

/**
 * @name cube_info_calc
 * @brief 计算cube_info
 * @param void
 */
void Vision_Detector :: cube_info_calc()
{
    for(int i = 0; i < 6; i++)
    {
        for(int j = 0;j<cube[i].contour_num;j++)
        {   //计算中心点的世界坐标
            Point2f center = pixel_to_world_coordinate(cube[i].position_pixel[j]);
            cube[i].position_world.push_back(center);
            Point2f center_base = world_to_base_coordinate(center);
            cube[i].position_base.push_back(center_base);
            //计算四个角点的世界坐标
            vector<Point2f> corner_world_coordinate;
            for(int k = 0;k<4;k++)
            {
                Point2f corner = pixel_to_world_coordinate(cube[i].corner_pixel_coordinate[j][k]);
                corner_world_coordinate.push_back(corner);
            }
            cube[i].corner_world_coordinate.push_back(corner_world_coordinate);
        }
    }
}
//图像矫正,将图像矫正到正视图
void Vision_Detector::cube_image_correct()
{
    int scale = 4;
    // 创建一个500x500的黑色画布
    Mat canvas = Mat::ones(500*scale, 500*scale, CV_8UC3);
    // 遍历每个色块
    for(int i = 0; i < 6; i++)
    {
        Mat sub_canvas = Mat::ones(500*scale, 500*scale, CV_8UC3);
        for(int j = 0; j < cube[i].contour_num; j++)
        {
            // 绘制四边形
            for(int k = 0; k < 4; k++)
            {
//                line(canvas, cube[i].corner_world_coordinate[j][k]*scale, cube[i].corner_world_coordinate[j][(k + 1) % 4]*scale, cube_color_scalar[i], 2);
            }
            //寻找最小外接矩形
            RotatedRect rect = minAreaRect(cube[i].corner_world_coordinate[j]);
            //绘制最小外接矩形
            Point2f vertices[4];
            rect.points(vertices);
            for (int k = 0; k < 4; k++)
            {
                line(canvas, vertices[k]*scale, vertices[(k + 1) % 4]*scale, cube_color_scalar[i], 2);
            }
            //在图上标出角度
            putText(canvas, to_string(static_cast<int>(round(rect.angle))), rect.center*scale, FONT_HERSHEY_SIMPLEX, 1, cube_color_scalar[i], 2);
            //存储角度
            // cube[i].angle_world.push_back(round(rect.angle));
            cube[i].angle_world.push_back(angle_format_90(round(rect.angle)));
            // vector<float> formatted_angle;
            // formatted_angle.push_back(angle_format_90(round(rect.angle)));
            // cout<<"cube["<<i<<"]["<<j<<"]'s angle = "<<rect.angle<<endl;
            //输出角度
            // cout << "cube[" << i << "][" << j << "]'s angle = " << rect.angle << endl;
            // cout << "cube[" << i << "][" << j << "]'s formatted_angle = " << formatted_angle[0] << endl;
        }


    }
    //将画布上下翻转
    flip(canvas, canvas, 0);
    correctedImage = canvas;
}
/**
 * @name cube_image_draw
 * @brief 绘制图片
 * @param void
 */
void Vision_Detector :: cube_image_draw()
{

    //绘制最小外接四边形
    Mat boxedImage = srcImage.clone();
    for(int i = 0;i<6;i++)
    {
        for(int j = 0;j<cube[i].contour_num;j++)
        {
            drawContours(boxedImage, cube[i].box, j, cube_color_scalar[i], 4);
        }
    }
    boxImage = boxedImage;
    //绘制凸包
    Mat hulledImage = srcImage.clone();
    for(int i = 0;i<6;i++)
    {
        for(int j = 0;j<cube[i].contour_num;j++)
        {
            drawContours(hulledImage, cube[i].hull, j, cube_color_scalar[i], 4);
        }
    }
    hullImage = hulledImage;
    //绘制点
    Mat pointedImage = srcImage.clone();
    for(int i = 0;i<6;i++)
    {
        for(int j = 0;j<cube[i].contour_num;j++)
        {
            for(int k = 0;k<4;k++)
            {
                circle(pointedImage, cube[i].corner_pixel_coordinate[j][k], 5, cube_color_scalar[i], -1);
                putText(pointedImage, to_string(k), cube[i].corner_pixel_coordinate[j][k], FONT_HERSHEY_SIMPLEX, 1, cube_color_scalar[i], 2);
            }
            //绘制中心点
            Point center = (cube[i].corner_pixel_coordinate[j][0] + cube[i].corner_pixel_coordinate[j][1] + cube[i].corner_pixel_coordinate[j][2] + cube[i].corner_pixel_coordinate[j][3]) / 4;
            circle(pointedImage, center, 5, cube_color_scalar[i], -1);
            putText(pointedImage, "center", center, FONT_HERSHEY_SIMPLEX, 1, cube_color_scalar[i], 2);
        }
    }
    pointImage = pointedImage;
    Mat coordinatedImage = srcImage.clone();
    for(int i = 0;i<6;i++)
    {
        for(int j = 0;j<cube[i].contour_num;j++)
        {

            //绘制中心点
            Point center = (cube[i].corner_pixel_coordinate[j][0] + cube[i].corner_pixel_coordinate[j][1] + cube[i].corner_pixel_coordinate[j][2] + cube[i].corner_pixel_coordinate[j][3]) / 4;
            circle(coordinatedImage, center, 5, cube_color_scalar[i], -1);
//            putText(coordinatedImage, "center", center, FONT_HERSHEY_SIMPLEX, 1, cube_color_scalar[i], 2);
            //绘制中心点的世界坐标
            Point2f center_world_coordinate = pixel_to_world_coordinate(center);
            putText(coordinatedImage, to_string(static_cast<int>(std::round(center_world_coordinate.x))) + "," + to_string(static_cast<int>(std::round(center_world_coordinate.y))), center, FONT_HERSHEY_SIMPLEX, 1, cube_color_scalar[i], 2);
        }
    }
    coordinateImage = coordinatedImage;

}

/**
 * @name cube_image_show
 * @brief 显示图片
 * @param void
 */
void Vision_Detector :: cube_image_show()
{
    //红色方块
    #if RED_MASK_TRESHOLD_TUNE ==1
        imshow("srcImage", srcImage);
        imshow("redMask", cube[0].mask);
        imshow("redMask_1", cube[0].mask_1);
        imshow("redMask_2", cube[0].mask_2);
        imshow("red_morphological_mask", cube[0].morphological_mask);
        imshow("red_filled_mask", cube[0].filled_mask);
    #endif
    #if GREEN_MASK_TRESHOLD_TUNE ==1
        imshow("srcImage", srcImage);
        imshow("greenMask", cube[1].mask);
        imshow("greenMask_1", cube[1].mask_1);
        imshow("greenMask_2", cube[1].mask_2);
        imshow("green_morphological_mask", cube[1].morphological_mask);
        imshow("green_filled_mask", cube[1].filled_mask);
    #endif
    #if BLUE_MASK_TRESHOLD_TUNE ==1
        imshow("srcImage", srcImage);
        imshow("blueMask", cube[2].mask);
        imshow("blueMask_1", cube[2].mask_1);
        imshow("blueMask_2", cube[2].mask_2);
        imshow("blue_morphological_mask", cube[2].morphological_mask);
        imshow("blue_filled_mask", cube[2].filled_mask);
    #endif
    #if YELLOW_MASK_TRESHOLD_TUNE ==1
        imshow("srcImage", srcImage);
        imshow("yellowMask", cube[3].mask);
        imshow("yellowMask_1", cube[3].mask_1);
        imshow("yellowMask_2", cube[3].mask_2);
        imshow("yellow_morphological_mask", cube[3].morphological_mask);
        imshow("yellow_filled_mask", cube[3].filled_mask);
    #endif
    #if ORANGE_MASK_TRESHOLD_TUNE ==1
        imshow("srcImage", srcImage);
        imshow("orangeMask", cube[4].mask);
        imshow("orangeMask_1", cube[4].mask_1);
        imshow("orangeMask_2", cube[4].mask_2);
        imshow("orange_morphological_mask", cube[4].morphological_mask);
        imshow("orange_filled_mask", cube[4].filled_mask);
    #endif
    #if PURPLE_MASK_TRESHOLD_TUNE ==1
        imshow("srcImage", srcImage);
        imshow("purpleMask", cube[5].mask);
        imshow("purpleMask_1", cube[5].mask_1);
        imshow("purpleMask_2", cube[5].mask_2);
        imshow("purple_morphological_mask", cube[5].morphological_mask);
        imshow("purple_filled_mask", cube[5].filled_mask);
    #endif

    #if HSV_TUNE == 0 && IMAGE_SHOW == 1
        imshow("srcImage", srcImage);

        imshow("hullImage", hullImage);

        imshow("boxImage", boxImage);

        imshow("pointImage", pointImage);

        imshow("coordinateImage", coordinateImage);

        imshow("correctedImage", correctedImage);

        imshow("maskedImage", maskedImage);

    #endif
}
/**
 * @name cube_image_save
 * @brief 保存图片
 * @param void
 */
void Vision_Detector :: cube_image_save()
{
    imwrite("../saved_images/srcImage.jpg", srcImage);
    imwrite("../saved_images/hullImage.jpg", hullImage);
    imwrite("../saved_images/boxImage.jpg", boxImage);
    imwrite("../saved_images/pointImage.jpg", pointImage);
    imwrite("../saved_images/coordinateImage.jpg", coordinateImage);
    imwrite("../saved_images/correctedImage.jpg", correctedImage);
    imwrite("../saved_images/maskedImage.jpg", maskedImage);

}


// 调整检测到的四个角点的顺序，使其按照逆时针顺序排列
vector<Point> adjustPoints(vector<Point> points) {
    // 采用选择排序法对四个点的y坐标进行排序
    for (int a = 0; a < 4; a++) {
        for (int b = a + 1; b < 4; b++) {
            if (points[a].y > points[b].y) {
                swap(points[a], points[b]);
            }
        }
    }
    // 如果第一个点的x坐标大于第二个点的x坐标，交换两个点的坐标
    if (points[0].x > points[1].x) swap(points[0], points[1]);
    // 如果第三个点的x坐标大于第四个点的x坐标，交换两个点的坐标
    if (points[2].x > points[3].x) swap(points[2], points[3]);

    // 确保点的顺序是逆时针的
    vector<Point> adjusted_points(4);
    adjusted_points[0] = points[3]; // 左上角
    adjusted_points[1] = points[2]; // 左下角
    adjusted_points[2] = points[0]; // 右下角
    adjusted_points[3] = points[1]; // 右上角

    return adjusted_points;
}
/**
 * @name angle_format_90
 * @brief 角度格式化为-45到45
 * @param angle 输入角度（0到90）
 * @return ret_angle 输出角度（-45到45）
 */
float angle_format_90(float angle)
{
    float ret_angle = angle;
    if(ret_angle > 45)
    {
        ret_angle = angle - 90;
    }
    else if(ret_angle < -45)
    {
        ret_angle = angle + 90;
    }
    else
    {
        ret_angle = angle;
    }
    return ret_angle;
}