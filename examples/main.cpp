// JAKAAPI.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <fstream>  
#include "JAKARobot_API.h"
using namespace std;


#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#pragma warning(disable : 4996)
#endif

int main()
{
	//1. 读取彩色图片
	//2. 处理图片，得到方块信息
	//findBlock(imBlock, blockProps);

	//3. 导入单应矩阵（离线计算）
	//4. 利用单应矩阵把方块信息从图像坐标系转换到机器人坐标系

	//5. 初始化机械臂并启动
	//6. 拾取每个目标根据颜色到特定位置
	//7. 注意机械爪的状态：开——闭——开

	//8. 所有目标都抓完后关闭机器人、退出程序

    return 0;
}

