// JAKAAPI.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <fstream>  
#include "JAKARobot_API.h"
#include "BSpline.hpp"
using namespace std;


#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#pragma warning(disable : 4996)
#endif

int main()
{
#if 1
	//读取关节角
	//double pts[3000];
	//char pl[128];
	//double *pt = pts;
	//fstream infile("joints_inc.txt", ios::in);
	//while (infile.getline(pl, 128))
	//{
	//	sscanf(pl, "%lf,%lf,%lf,%lf,%lf,%lf", pt, pt + 1, pt + 2, pt + 3, pt + 4, pt + 5);
	//	pt += 6;
	//}
	//infile.close();

	JAKAZuRobot jakaRob;
	int is_enable = 0, is_powered = 0;

	double T[4][4] = { { -0.572358962775918, -0.658119909692108, -0.0334441901261411,	0.861885052682280 },
		{ -0.858592201243573, 0.518658968830468,	0.100462213584561732,	0.294352405492099 },
		{ -0.1175420987028802,	0.0284781015464451, -0.909440479721233,	0.291656649169983 },
		{0.0, 0.0, 0.0, 1.0} };
	double qs[48];
	int num = jakaRob.iKinematics(T, qs);

	int ret = jakaRob.login_in("192.168.1.101");
	if (ret) printf("%s\n", jakaRob.m_errMsg);
	jakaRob.getState(is_enable, is_powered);
	if (!is_powered)
	{
		ret = jakaRob.power(1);
		Sleep(6);
	}
	if (!is_enable)
	{
		ret = jakaRob.enable(1);
		Sleep(6);
	}

	jakaRob.initSetting();
	Sleep(10);
	double jointpos1[6], jointpos2[6];
	jakaRob.getJoints(jointpos1);

	double jointsHome[6] = { 109.30, 88.61, -80.53, 196.11, 117.77, 107.22};
	double incJoint[6] = { 0.0, 0.0,-40.0,0.0,0.0,0.0 };
	ret = jakaRob.moveJ(jointsHome, 10.0, 0);
	//ret = jakaRob.moveL(incJoint, 10.0, 1);
	ret = jakaRob.waitEndMove();

	//定义t
	float t = 0.5;




	//6. 机械臂启动抓取
		Point3f storePoint;//定义物块存放点
		for (int i = 0; i < output.Point.size(); i++)
		{
			Coord3D controlPoints[] =
			{
				{ 0,0,0},
				{ 0,0,0},
				{ 0,0,0},
				{ 0,0,0},
				{ 0,0,0},
			};  //控制点
			controlPoints[0]={ newIniPose[0],newIniPose[1],newIniPose[2] };
			controlPoints[1] = { newIniPose[0],newIniPose[1],300 };
			controlPoints[2] = { output.Point[i].x, output.Point[i].y,300 };
			controlPoints[3] = { output.Point[i].x,output.Point[i].y,180 };
			controlPoints[4] = { output.Point[i].x,output.Point[i].y,180 };

			float knots[] = { 0,0,0,0,0.5,1,1,1,1 };//按这样给定的节点保证生成的B样条曲线会经过初末控制点
	//构造B样条函数
			BSpline spline(3, 4, knots, controlPoints);//T1=k+n+2
			for (int j = 0; j < 1001; j++)
			{
				Coord3D point = spline.PtsInCur(0.001 * j);//获得坐标
				//cout << point.x << "\t" << point.y << "\t" << point.z << endl;
				pts[j * 6 + 0] = point.x;
				pts[j * 6 + 1] = point.y;
				pts[j * 6 + 2] = point.z;
				pts[j * 6 + 3] = 180;
				pts[j * 6 + 4] = 0;
				pts[j * 6 + 5] = 45+output.Angle[i]*0.001*j;
			}
			//连续轨迹运行
			ret = jakaRob.setDout(Index, 0, IO_CABINET);
			Sleep(100);
			ret = jakaRob.enableCVMotion(1);
			ret = jakaRob.cvMove(pts, 1001, COORD_WORD, 0);
			ret = jakaRob.enableCVMotion(0);
			Sleep(1000);
			ret = jakaRob.setDout(Index, 1, IO_CABINET);
			Sleep(1000);
			cout <<"第"+to_string(i)+"个\n" << endl;





	// 连续轨迹运行
	LARGE_INTEGER start_t, end_t, frequency;
	QueryPerformanceFrequency(&frequency);	//获取计时器的时钟频率
	QueryPerformanceCounter(&start_t);		//获得计数器初始值
	ret = jakaRob.enableCVMotion(1);
	ret = jakaRob.cvMove(pts, 500, COORD_JOINT, 1);
	ret = jakaRob.enableCVMotion(0);
	QueryPerformanceCounter(&end_t);		//获得计数器初始值
	double tt = (end_t.QuadPart - start_t.QuadPart) / frequency.QuadPart;

	jakaRob.getJoints(jointpos2);

	jakaRob.login_out();
#else

	JAKAZuRobot jakaRob;
	int is_enable = 0, is_powered = 0;

	int ret = jakaRob.login_in("192.168.1.101");
	if (ret) printf("%s\n", jakaRob.m_errMsg);
	jakaRob.enable(0);
	Sleep(1000);
	jakaRob.power(0);
#endif

    return 0;
}

