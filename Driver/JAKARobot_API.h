#pragma once

#ifndef _JAKAAPI_H_
#define _JAKAAPI_H_

#include <Winsock2.h>

#ifdef DLLEXPORT_EXPORTS
#define DLLEXPORT_API __declspec(dllexport)
#else
#define DLLEXPORT_API __declspec(dllimport)
#endif

enum ERROR_TYPE {ROBOT_NOERR = 0, ROBOT_ERROR = -1};
enum IO_TYPE {IO_CABINET, IO_TOOL, IO_EXTEND};
enum COORD_TYPE {COORD_JOINT = 0, COORD_WORD, COORD_TOOL};	//默认的世界坐标系为基座坐标系

class JAKAZuRobot
{
public:

	/**
	* @brief 机械臂控制类构造函数
	*/
	JAKAZuRobot();

	/*******************************系统指令******************************/
	/**
	* @brief 连接机器人并登陆
	* @param ip IP地址字符串
	* @return NO_ERROR 成功 其他失败
	*/
	int login_in(const char* ip);

	/**
	* @brief 退出连接
	* @return NO_ERROR 成功 其他失败
	*/
	int login_out();

	/**
	* @brief 机器人电源控制
	* @param isOn:1—开启电源；0—关闭电源
	* @return NO_ERROR 成功 其他失败
	*/
	int  power(int isOn = 1);

	/**
	* @brief 使能控制
	* @param enable：1—上使能；0—下使能
	* @return NO_ERROR 成功 其他失败
	*/
	int  enable(int isEnable = 1);

	/**
	* @brief 初始化设置，把所有设置都改为默认值
	* @return NO_ERROR 成功 其他失败
	*/
	int initSetting();

	/**
	* @brief 清除碰撞后的错误
	* @return NO_ERROR 成功 其他失败
	*/
	int clearError();

	/**
	* @brief 获得机器人的使能和电源状态
	* @param(out) is_enable：1—使能状态，0—下使能状态 
	* @param(out) is_powered：1—上电状态，0—下电状态
	* @return NO_ERROR 成功 其他失败
	*/
	int getState(int &is_enable, int &is_powered);

	/*****************************读写指令***************************************/
	/* @brief 读取机器人所有数据到字符串，
	*	注：当同时需要获得多个数据值先调用该函数则会更高效
	* @param(out) all_data 输出的数据字符串，建议字符串字符长度设2000
	* @param port 端口号，默认为1，对应10001，否则为10000（专为读数据）
	* @return NO_ERROR 成功 其他失败
	*/
	int getData(char *all_data, int port = 1);

	/* @brief 读取机器人关节角
	* @param(out) joint_angles 机器人关节角，单位角度
	* @param data_info 如果该指针不为空，那么需要先调用一次getData函数获得原始数据
	* @return NO_ERROR 成功 其他失败
	*/
	int getJoints(double *joint_angles, char *data_info = NULL);

	/* @brief 读取机器人工具的位姿
	* @param(out) tcp_pose 机器人关节角，单位角度
	* @param data_info 如果该指针不为空，那么需要先调用一次getData函数获得原始数据
	* @return NO_ERROR 成功 其他失败
	*/
	int getTcpPose(double *tcp_pose, char *data_info = NULL);

	/* @brief 查询机器人数字输入口的状态
	* @param(out) dio_in 输出机器人对应数字输入口的状态，用位标记，从低到高，
	*					如果第2位为1，那么对应的DI1（下标从0开始）为闭状态
	* @param data_info 如果该指针不为空，那么需要先调用一次getData函数获得原始数据
	* @param IOType IO类型，IO_CABINET(0)—控制柜IO，总数8;IO_TOOL(1)—工具IO，总数2
	* @return NO_ERROR 成功 其他失败
	*/
	int getDin(int &dio_in, int IOType = IO_CABINET, char *data_info = NULL);

	/* @brief 设置数字输出口(DO)的状态
	* @param index DO索引（从0开始）
	* @param value DO设置值（1或0）
	* @param IOType IO类型，IO_CABINET(0)—控制柜IO;IO_TOOL(1)—工具IO
	* @return NO_ERROR 成功 其他失败
	*/
	int setDout(int index, int value = 1, int IOType = IO_CABINET);

	/* @brief 添加自定义的工具坐标系
	* @param tool_offset 工具偏移矩阵，类型同位姿描述，数组长度为6
	* @param tool_name 工具名称
	* @param id_num 工具的ID号
	* @return NO_ERROR 成功 其他失败
	*/
	int addTool(double *tool_offset, char *tool_name, int tool_id = 1);

	/* @brief 设置工具坐标系
	* @param tool_id 工具的ID号，id=0，则表示法兰盘中心；
	*				其它则为自定义的工作坐标系，同addTool函数配合使用
	* @return NO_ERROR 成功 其他失败
	*/
	int setToolID(int tool_id = 0);

	/* @brief 获取当前的工具坐标系ID
	* @param tool_id(输出) 工具的ID号
	* @return NO_ERROR 成功 其他失败
	*/
	int getToolID(int &tool_id);

	/* @brief 添加自定义的用户坐标系（世界坐标系）
	* @param user_offset 世界坐标系的偏移矩阵，类型同位姿描述，数组长度为6
	* @param user_name 工具名称
	* @param user_id 工具的ID号
	* @return NO_ERROR 成功 其他失败
	*/
	int addUserCord(double *user_offset, char *user_name, int user_id = 1);

	/* @brief 选择用户坐标系
	* @param user_id 用户坐标系的ID号，id=0，则表示基座坐标系；
	*				其它则为自定义的工作坐标系，同addUserCord函数配合使用
	* @return NO_ERROR 成功 其他失败
	*/
	int setUserID(int user_id = 0);

	/* @brief 获取当前的用户坐标系ID
	* @param user_id(输出) 用户坐标系的ID号
	* @return NO_ERROR 成功 其他失败
	*/
	int getUserID(int &user_id);

	/* @brief 设置末端负载
	* @param mass 负载的质量，单位kg
	* @param offset 负载的质心[x,y,z]，单位mm
	* @return NO_ERROR 成功 其他失败
	*/
	int setPayload(double mass, double *offset);

	/* @brief 获取末端负载
	* @param mass(out) 负载的质量，单位kg
	* @param offset(out) 负载的质心[x,y,z]，单位mm
	* @return NO_ERROR 成功 其他失败
	*/
	int getPayload(double &mass, double *offset);

	/* @brief 设置机器人速度倍率
	* @param rapid_rate 速度倍率[0,1]
	* @return NO_ERROR 成功 其他失败
	*/
	int setRapidRate(double rapid_rate);

	/***************************运动指令******************************************/
	/* @brief 机器人关节运动
	* @param joint_pos 机器人6个关节运动目标位置,单位：度
	* @param speed 机器人关节运动速度，单位：度/s
	* @move_mode 指定运动模式：0—绝对运动；1—或增量运动
	* @return NO_ERROR 成功 其他失败
	*/
	int  moveJ(const double *joint_pos, double speed, int move_mode = 0);

	/* @brief 机器人末端运动:先算逆运动学，然后再moveJ
	* @param endPos 机器人末端位姿[x,y,z,a,b,c],位姿描述为rpy角，R=rz(c)ry(b)rx(a),角度单位度
	* @param speed 机器人关节运动速度，单位：度/s
	* @return NO_ERROR 成功 其他失败
	*/
	int  moveP(const double *endPos, double speed);

	/* @brief 机器人末端运动直线运动
	* @param endPos 机器人末端位姿[x,y,z,a,b,c],位姿描述为rpy角，R=rz(c)ry(b)rx(a),角度单位度
	* @param speed 机器人关节运动速度，单位：mm/s
	* @move_mode 指定运动模式：0—绝对运动；1—或增量运动
	* @return NO_ERROR 成功 其他失败
	*/
	int  moveL(const double *endPos, double speed, int move_mode = 0);

	/* @brief 停止机器人的运动
	* @return NO_ERROR 成功 其他失败
	*/
	int stopMotion();

	/* @brief 等待运动结束（堵塞指令），运动结束后才执行下一条指令
	/* @param waitTime,最长等待时间（单位:ms)，如果超过这个时间
	* @return >=0 运行时间（ms); < 0 ——超时
	*/
	int waitEndMove(int waitTime = 10000);

	/* @brief 机器人进入连续运动使能
	* @param enable 1——进入；0——退出
	* @return NO_ERROR 成功 其他失败
	*/
	int enableCVMotion(int enable = 1);

	/* @brief 机器人连续曲线运动
	* @param positions 机器人位置数据（n*6），六关节角或笛卡尔位姿描述都可以，取决于坐标系参数
	* @param pts_num 位置点个数
	* @param coord_type 参照坐标系：COORD_JOINT——关节空间，COORD_WORD——世界坐标系（默认基座坐标系）
	* @param move_mode 指定运动模式：0—绝对运动；1—或增量运动
	* @return NO_ERROR 成功 其他失败
	*/
	int cvMove(double *positions, int pts_num, int coord_type = COORD_JOINT, int move_mode = 0);

	/***************************运动学******************************************/
	/* @brief 机器人正运动学
	* @param joint_pos 机器人关节角（数组大小6）
	* @param endPos(out) 机器人末端的位姿
	* @return NO_ERROR 成功 其他失败
	*/
	int fKinematics(double *joint_pos, double *endPos);

	/* @brief 机器人逆运动学
	* @param ref_joints 机器人参考关节角（数组大小6），推荐用户选取机器人当前的关节角作为参考关节角
	* @param endPos 机器人末端的位姿
	* @param joint_pos(out) 机器人关节角
	* @return NO_ERROR 成功 其他失败
	*/
	int iKinematics(double *ref_joints, double *endPos, double *joint_pos);

	//--------------------------------以下为自定义函数-------------------------------------------------
	/* @brief 机器人逆运动学——自行按公式计算
	* @param endPos 机器人末端的位姿4*4矩阵
	* @param joint_pos(out) 机器人关节角（-pi, pi]，地址建议由外部传入，大小至少为8*6
	* @return <0 失败， 其他—逆解个数
	*/
	int iKinematics(double endPos[4][4], double *joint_pos);
public:
	char m_errMsg[128];			//错误代码消息
private:

	SOCKET m_socketClient;
	SOCKET m_socketData;		//用来实时获得数据的socket
};

//测试的时候放在这里，发布后删除！
int findFieldText(char *recvInfo, const char *fieldName, char *fieldText, int txtType = 0);

#endif