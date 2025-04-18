/*****************************************************
 * @file main.hpp
 * @brief This is the main file for the project
 * It is used to test the Vision_Detector class
 * @date 2024-06-21
 *****************************************************/

#ifndef PROJECT_FINAL_MAIN_H
#define PROJECT_FINAL_MAIN_H
#include "stdafx.h"
#include <iostream>
#include <fstream>  
#include <vector>
#include <Eigen/Dense>
#include "JAKARobot_API.h"
#include "Vision_Detector.h"
#include "BSpline.hpp"
#include "user_lib.h"

#define JAKA_ZU3_IP JAKA_ZU3_3_IP
#define JAKA_ZU3_4_IP "192.168.1.100"
#define JAKA_ZU3_3_IP "192.168.1.100"
#define TOOL_PORT 4
#define TOOL_OPEN 0
#define TOOL_CLOSE 1
#define EXIT_KEY 27
using namespace std;
using namespace Eigen;
#define LIFT_HEIGHT 150  //抬升高度
#define CATCH_HEIGHT 150 //抓取高度
#define CUBE_HEIGHT 20  //方块高度
#define RELEASE_HEIGHT 150 //释放高度
#define TOOL_ANGLE_DIFF -7

class Robot{
    public:
    Robot();//构造函数
    ~Robot();//析构函数
    double joints_init[6] = { 5, 79, 129, 61, -90, -122};//初始姿态
    double position_home[6] = {377 , -130, 198, 180, 0, TOOL_ANGLE_DIFF};
    Point3f home_point = {100,100,250};

    vector<Point2f> cube_settle_point = {
        {25,300},
        {100,300},
        {175,300},
        {25,250},
        {100,250},
        {175,250}
    };


    // 连续轨迹运行
    LARGE_INTEGER start_t, end_t, frequency;

    JAKAZuRobot jaka_bot;
    int is_enabled = 0;//是否使能
    int is_powered = 0;//是否上电
    int jaka_bot_ret;//jaka机器人返回值
    int robot_init();//机器人初始化
    int robot_move_to_init_pos();//机器人移动到初始位置
    int robot_move_to_world_point_and_wait(Point3f point,double angle);//机器人移动到世界坐标点并等待移动结束
    int robot_move_home_to_cube(int cube_color,int cube_num);//机器人从家的位置移动到方块的位置
    int robot_move_cube_to_home(int cube_color,int cube_num);//机器人从方块的目标位置移动到家的位置
    int robot_carry_cube(int cube_color,int cube_num);//机器人搬运方块到目标位置
    int robot_carry_cube(Point3f start_point_world , Point3f end_point_world , double start_angle ,double end_angle);
    int robot_move_cube_task();
    int tool_open();//工具打开
    int tool_close();//工具关闭


    // 视觉部分
    Vision_Detector cube_detector;
    void vision_target_detect();// 视觉检测
};









#endif //PROJECT_MAIN_H