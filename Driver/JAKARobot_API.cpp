#include "stdafx.h"
#include <iostream>
#include "JAKARobot_API.h"
#include <windows.h>
#include <stdlib.h>
#include <string>
#include <math.h>

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#pragma warning(disable : 4996)
#endif
#pragma comment(lib, "ws2_32.lib")

#define  BUFF_SIZE	512
#define  MAX_BUFF_SIZE	2048			//获取的机器人所有数据所需要的内存大小，实际值在1500左右

#ifndef M_PI
#define  M_PI 3.141516925
#endif

//定义参数的数据类型
enum TEXTYPE { STRING, NUMBER, ARRAY };

/* @brief 查找返回信息中对应字段的内容
* @param recvInfo 返回信息
* @param fieldName 要查找的字段名称好的
* @param fieldText 输出的字段内容
* @param txtType 字段数据类型，0—字符串，1—数，2—数组
* @return 1— 成功；0—失败
*/
//JSON 值可以是：数字（整数或浮点数）字符串（在双引号中）逻辑值（true 或 false）数组（在方括号中）对象（在花括号中） null
int findFieldText(char *recvInfo, const char *fieldName, char *fieldText, int txtType)
{
	int num = 0;
	char *ptr = strstr(recvInfo, fieldName);
	if (ptr != NULL)
	{
		switch (txtType)
		{
		case STRING:
			num = sscanf(ptr, "%*[^:]%*[^\"]\"%[^\"]", fieldText);
			break;
		case NUMBER:
			num = sscanf(ptr, "%*[^:]:%[^,]", fieldText);
			if (strchr(fieldText, '\"'))		//部分数字型的字段内容包含“”
			{
				int n = strlen(fieldText);
				memcpy(fieldText, fieldText + 1, n - 2);
				fieldText[n - 2] = '\0';
			}
			break;
		case  ARRAY:
			num = sscanf(ptr, "%*[^:]%*[^[][%[^]]", fieldText);
			break;
		default:
			break;
		}
	}

	return num;
}

JAKAZuRobot::JAKAZuRobot(){}

int JAKAZuRobot::login_in(const char* ip)
{
	WORD wVersinRequested;
	WSADATA wsaData;
	int err;
	wVersinRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersinRequested, &wsaData);
	if (err != 0) return ROBOT_ERROR; 
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();		
		return ROBOT_ERROR;
	}
	SOCKADDR_IN addrSrv;//设定服务器端的IP和端口
	addrSrv.sin_addr.s_addr = inet_addr(ip);//本地回路地址	
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(10001);
	m_socketClient = socket(AF_INET, SOCK_STREAM, 0);//将其第三个参数设为0，让其自动选择协议。
	if (m_socketClient == INVALID_SOCKET)
		sprintf(m_errMsg, "socket() called failed!, error code is %d", WSAGetLastError());
	int u = connect(m_socketClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));//与服务器建立连接。	Sleep(2000);
	if (u == SOCKET_ERROR)
	{
		sprintf(m_errMsg, "connect() called failed!, error code is %d", WSAGetLastError());
		closesocket(m_socketClient);
		return ROBOT_ERROR;
	}

	//10000端口的设置
	addrSrv.sin_port = htons(10000);
	m_socketData = socket(AF_INET, SOCK_STREAM, 0);//将其第三个参数设为0，让其自动选择协议。

	 //设置缓冲区大小，不需要存储很多数据，实际大小是设置的2倍
	int rcvcbuf_len = MAX_BUFF_SIZE;
	int len = sizeof(rcvcbuf_len);
	setsockopt(m_socketData, SOL_SOCKET, SO_RCVBUF, (char *)&rcvcbuf_len, len); 

	u = connect(m_socketData, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));//与服务器建立连接。	Sleep(2000);
	if (u == SOCKET_ERROR)
	{
		sprintf(m_errMsg, "connect() called failed!, error code is %d", WSAGetLastError());
		closesocket(m_socketData);
		return ROBOT_ERROR;
	}

	return NO_ERROR;
}

int JAKAZuRobot::login_out()
{
	char sendCmd[BUFF_SIZE], recvBuff[BUFF_SIZE];
	strcpy(sendCmd, "{\"cmdName\":\"quit\"}");

	int rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
	rtn = recv(m_socketClient, recvBuff, 100, 0);
	if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;

	int isOK = findFieldText(recvBuff, "errorCode", m_errMsg, NUMBER);
	rtn = atoi(m_errMsg);
	isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);

	//关闭套接字
	closesocket(m_socketClient);
	closesocket(m_socketData);
	//终止套接字库的使用
	WSACleanup();

	return  rtn;
}

int JAKAZuRobot::power(int isOn)
{
	char sendCmd[BUFF_SIZE], recvBuff[BUFF_SIZE];
	if (isOn > 0)
		strcpy(sendCmd, "{\"cmdName\":\"power_on\"}");
	else
		strcpy(sendCmd, "{\"cmdName\":\"power_off\"}");

	int rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
	//Sleep(6);
	rtn = recv(m_socketClient, recvBuff, BUFF_SIZE, 0);
	if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;
	//Sleep(6);

	int isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
	char errs[8];
	isOK = findFieldText(recvBuff, "errorCode", errs, NUMBER);
	return   atoi(errs);
}

int JAKAZuRobot::enable(int isEnable)
{
	char sendCmd[BUFF_SIZE], recvBuff[BUFF_SIZE];
	if (isEnable > 0)
		strcpy(sendCmd, "{\"cmdName\":\"enable_robot\"}");
	else
		strcpy(sendCmd, "{\"cmdName\":\"disable_robot\"}");

	int rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
	rtn = recv(m_socketClient, recvBuff, BUFF_SIZE, 0);
	if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;

	int isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
	char errs[8];
	isOK = findFieldText(recvBuff, "errorCode", errs, NUMBER);
	return   atoi(errs);
}

int JAKAZuRobot::getState(int &is_enable, int &is_powered)
{
	char sendCmd[] = "{\"cmdName\":\"get_robot_state\"}";
	char recvBuff[BUFF_SIZE];

	int rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
	rtn = recv(m_socketClient, recvBuff, BUFF_SIZE, 0);
	if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;

	char errs[8];
	int isOK = findFieldText(recvBuff, "errorCode", errs, NUMBER);
	rtn = atoi(errs);
	if (rtn) 
	{
		isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
		return rtn;
	}

	char fieldText[BUFF_SIZE];
	isOK = findFieldText(recvBuff, "enable", fieldText);
	if (strstr(fieldText, "robot_enabled"))		is_enable = 1;
	else is_enable = 0;

	isOK = findFieldText(recvBuff, "power", fieldText);
	if (strstr(fieldText, "powered_on"))		is_powered = 1;
	else is_powered = 0;

	return  rtn;
}

int JAKAZuRobot::initSetting()
{
	int rtn;

	rtn = clearError();
	if (rtn) return rtn;

	rtn = setToolID(0);
	if (rtn) return rtn;

	rtn = setUserID(0);
	if (rtn) return rtn;

	double offset[3] = { 0.0f };
	rtn = setPayload(0.0, offset);
	if (rtn) return rtn;

	rtn = setRapidRate(1.0);
	if (rtn) return rtn;

	return NO_ERROR;
}

int JAKAZuRobot::clearError()
{
	int rtn;

	//清除错误
	char sendCmd[BUFF_SIZE] = "{\"cmdName\":\"clear_error\"}";
	char recvBuff[BUFF_SIZE];
	rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
	rtn = recv(m_socketClient, recvBuff, MAX_BUFF_SIZE, 0);

	if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;
	else return ROBOT_NOERR;
}

int JAKAZuRobot::getData(char *all_data, int port)
{
	char sendCmd[] = "{\"cmdName\":\"get_data\"}";

	int rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
	rtn = recv(m_socketClient, all_data, MAX_BUFF_SIZE, 0);
	if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;

	int isOK = findFieldText(all_data, "errorMsg", m_errMsg);
	char errs[8];
	isOK = findFieldText(all_data, "errorCode", errs, NUMBER);
	return   atoi(errs);
}

int JAKAZuRobot::getJoints(double *joint_angles, char *data_info /* = NULL */)
{
	char jAngles[BUFF_SIZE];

	int rtn = 0;
	if (data_info == NULL)
	{
		char recvBuff[BUFF_SIZE];
		char sendCmd[] = "{\"cmdName\":\"get_joint_pos\"}";
		rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
		rtn = recv(m_socketClient, recvBuff, BUFF_SIZE, 0);

		int isOK = findFieldText(recvBuff, "errorCode", m_errMsg, NUMBER);
		rtn = atoi(m_errMsg);
		if (rtn != 0)
		{
			isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
			return rtn;
		}

		isOK = findFieldText(recvBuff, "joint_pos", jAngles, ARRAY);
		isOK = sscanf(jAngles, "%lf,%lf,%lf,%lf,%lf,%lf",
			joint_angles, joint_angles+1, joint_angles+2, joint_angles+3, joint_angles+4, joint_angles+5);
	}
	else
	{
		int isOK = findFieldText(data_info, "joint_actual_position", jAngles, ARRAY);
		isOK = sscanf(jAngles, "%lf,%lf,%lf,%lf,%lf,%lf",
			joint_angles, joint_angles + 1, joint_angles + 2, joint_angles + 3, joint_angles + 4, joint_angles + 5);
	}

	return  rtn;
}

int JAKAZuRobot::getTcpPose(double *tcp_pose, char *data_info /* = NULL */)
{
	char poses[BUFF_SIZE];

	int rtn = 0;
	if (data_info == NULL)
	{
		char recvBuff[BUFF_SIZE];
		char sendCmd[] = "{\"cmdName\":\"get_tcp_pos\"}";
		rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
		rtn = recv(m_socketClient, recvBuff, BUFF_SIZE, 0);

		int isOK = findFieldText(recvBuff, "errorCode", m_errMsg, NUMBER);
		rtn = atoi(m_errMsg);
		if (rtn != 0)
		{
			isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
			return rtn;
		}

		isOK = findFieldText(recvBuff, "tcp_pos", poses, ARRAY);
		isOK = sscanf(poses, "%lf,%lf,%lf,%lf,%lf,%lf",
			tcp_pose, tcp_pose + 1, tcp_pose + 2, tcp_pose + 3, tcp_pose + 4, tcp_pose + 5);
	}
	else
	{
		int isOK = findFieldText(data_info, "\"actual_position\"", poses, ARRAY);
		isOK = sscanf(poses, "%lf,%lf,%lf,%lf,%lf,%lf",
			tcp_pose, tcp_pose + 1, tcp_pose + 2, tcp_pose + 3, tcp_pose + 4, tcp_pose + 5);
	}

	return  rtn;
}

int JAKAZuRobot::getDin(int &dio_in, int IOType /* = IO_CABINET */, char *data_info /* = NULL */)
{
	int rtn = 0, isOK;
	char strDins[BUFF_SIZE];
	dio_in = 0;

	if (IOType == IO_CABINET)
	{
		if (data_info == NULL)
		{
			char recvBuff[BUFF_SIZE];
			char sendCmd[] = "{\"cmdName\":\"get_digital_input_status\"}";
			int rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
			rtn = recv(m_socketClient, recvBuff, BUFF_SIZE, 0);

			isOK = findFieldText(recvBuff, "errorCode", m_errMsg, NUMBER);
			rtn = atoi(m_errMsg);
			if (rtn != 0)
			{
				isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
				return rtn;
			}

			isOK = findFieldText(recvBuff, "din_status", strDins, ARRAY);
		}
		else
		{
			isOK = findFieldText(data_info, "\"din\"", strDins, ARRAY);	//din字段有重复
		}

		int ins[8] = { 0 };
		isOK = sscanf(strDins, "%d,%d,%d,%d,%d,%d,%d,%d,%*s",
			ins, ins + 1, ins + 2, ins + 3, ins + 4, ins + 5, ins + 6, ins + 7);	//只获取前8个值
		for (int i = 0; i < 8; i++)
		{
			dio_in |= (ins[i] << i);
		}
	}
	else
	{
		if (data_info == NULL)
		{
			char recvBuff[MAX_BUFF_SIZE];
			char sendCmd[] = "{\"cmdName\":\"get_data\"}";		//没有单独获得TOOL_IO的指令

			rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
			rtn = recv(m_socketClient, recvBuff, MAX_BUFF_SIZE, 0);

			isOK = findFieldText(recvBuff, "errorCode", m_errMsg, NUMBER);
			rtn = atoi(m_errMsg);
			if (rtn != 0)
			{
				isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
				return rtn;
			}

			isOK = findFieldText(recvBuff, "tio_din", strDins, ARRAY);
		}
		else
		{
			isOK = findFieldText(data_info, "tio_din", strDins, ARRAY);
		}

		int ins[2] = { 0 };
		isOK = sscanf(strDins, "%d,%d,%*s", ins, ins + 1);	//只获取前8个值
		dio_in = ins[0] + (ins[1] << 1);
	}

	return  rtn;
}

int JAKAZuRobot::setDout(int index, int value /* = 1 */, int IOType /* = IO_CABINET */)
{
	char sendCmd[BUFF_SIZE];
	sprintf(sendCmd, "{\"cmdName\":\"set_digital_output\",\"type\":%d,\"index\":%d,\"value\":%d}", 
		IOType, index, value);

	char recvBuff[BUFF_SIZE];
	int rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
	rtn = recv(m_socketClient, recvBuff, MAX_BUFF_SIZE, 0);

	int isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
	char errs[8];
	isOK = findFieldText(recvBuff, "errorCode", errs, NUMBER);
	return atoi(errs);
}

int JAKAZuRobot::addTool(double *tool_offset, char *tool_name, int tool_id /* = 1 */)
{
	char sendCmd[BUFF_SIZE]= "{\"cmdName\":\"set_tool_offsets\",\"tooloffset\":";
	char strParams[BUFF_SIZE];

	sprintf(strParams, "[%f,%f,%f,%f,%f,%f],\"id\":%d,\"name\":\"%s\"}",
		tool_offset[0], tool_offset[1], tool_offset[2], tool_offset[3], tool_offset[4], tool_offset[5], tool_id, tool_name);
	strcat(sendCmd, strParams);

	int rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
	if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;

	char recvBuff[BUFF_SIZE];
	rtn = recv(m_socketClient, recvBuff, BUFF_SIZE, 0);
	if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;

	int isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
	char errs[8];
	isOK = findFieldText(recvBuff, "errorCode", errs, NUMBER);
	return atoi(errs);
}

int JAKAZuRobot::setToolID(int tool_id /* = 0 */)
{
	char sendCmd[BUFF_SIZE];
	sprintf(sendCmd, "{\"cmdName\":\"set_tool_id\",\"tool_id\":%d}", tool_id);

	char recvBuff[BUFF_SIZE];
	int rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
	rtn = recv(m_socketClient, recvBuff, MAX_BUFF_SIZE, 0);

	int isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
	char errs[8];
	isOK = findFieldText(recvBuff, "errorCode", errs, NUMBER);
	return atoi(errs);
}

int JAKAZuRobot::getToolID(int &tool_id)
{
	char recvBuff[MAX_BUFF_SIZE];
	char sendCmd[] = "{\"cmdName\":\"get_data\"}";		//没有单独获得get_tool_id的指令

	int rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
	rtn = recv(m_socketClient, recvBuff, MAX_BUFF_SIZE, 0);

	int isOK = findFieldText(recvBuff, "errorCode", m_errMsg, NUMBER);
	rtn = atoi(m_errMsg);
	if (rtn != 0)
	{
		isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
	}
	else
	{
		char ids[8];
		isOK = findFieldText(recvBuff, "current_tool_id", ids, NUMBER);
		tool_id = atoi(ids);
	}

	return rtn;
}

int JAKAZuRobot::addUserCord(double *user_offset, char *user_name, int user_id /* = 1 */)
{
	char sendCmd[BUFF_SIZE] = "{\"cmdName\":\"set_user_offsets\",\"userffset\":";
	char strParams[BUFF_SIZE];

	sprintf(strParams, "[%f,%f,%f,%f,%f,%f],\"id\":%d,\"name\":\"%s\"}",
		user_offset[0], user_offset[1], user_offset[2], user_offset[3], user_offset[4], user_offset[5], user_id, user_name);
	strcat(sendCmd, strParams);

	int rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
	if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;

	char recvBuff[BUFF_SIZE];
	rtn = recv(m_socketClient, recvBuff, BUFF_SIZE, 0);
	if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;

	int isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
	char errs[8];
	isOK = findFieldText(recvBuff, "errorCode", errs, NUMBER);
	return atoi(errs);
}

int JAKAZuRobot::setUserID(int user_id /* = 0 */)
{
	char sendCmd[BUFF_SIZE];
	sprintf(sendCmd, "{\"cmdName\":\"set_user_id\",\"user_frame_id\":%d}", user_id);

	char recvBuff[BUFF_SIZE];
	int rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
	rtn = recv(m_socketClient, recvBuff, MAX_BUFF_SIZE, 0);

	int isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
	char errs[8];
	isOK = findFieldText(recvBuff, "errorCode", errs, NUMBER);
	return atoi(errs);
}

int JAKAZuRobot::getUserID(int &user_id)
{
	char recvBuff[MAX_BUFF_SIZE];
	char sendCmd[] = "{\"cmdName\":\"get_data\"}";		//没有单独获得get_tool_id的指令

	int rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
	rtn = recv(m_socketClient, recvBuff, MAX_BUFF_SIZE, 0);

	int isOK = findFieldText(recvBuff, "errorCode", m_errMsg, NUMBER);
	rtn = atoi(m_errMsg);
	if (rtn != 0)
	{
		isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
	}
	else
	{
		char ids[8];
		isOK = findFieldText(recvBuff, "current_user_id", ids, NUMBER);
		user_id = atoi(ids);
	}

	return rtn;
}

int JAKAZuRobot::setPayload(double mass, double *offset)
{
	char sendCmd[BUFF_SIZE];
	sprintf(sendCmd, "{\"cmdName\":\"set_payload\",\"mass\":%f,\"centroid\":[%f,%f,%f]}", 
		mass, offset[0], offset[1], offset[2]);

	char recvBuff[BUFF_SIZE];
	int rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
	rtn = recv(m_socketClient, recvBuff, MAX_BUFF_SIZE, 0);

	int isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
	char errs[8];
	isOK = findFieldText(recvBuff, "errorCode", errs, NUMBER);
	return atoi(errs);
}

int JAKAZuRobot::getPayload(double &mass, double *offset)
{
	char sendCmd[BUFF_SIZE] = "{\"cmdName\":\"get_payload\"}";

	char recvBuff[BUFF_SIZE];
	int rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
	rtn = recv(m_socketClient, recvBuff, MAX_BUFF_SIZE, 0);

	int isOK = findFieldText(recvBuff, "errorCode", m_errMsg, NUMBER);
	rtn = atoi(m_errMsg);
	if (rtn != 0)
	{
		isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
	}
	else
	{
		char txts[64];
		isOK = findFieldText(recvBuff, "mass", txts, NUMBER);
		mass = atof(txts);

		isOK = findFieldText(recvBuff, "centroid", txts, ARRAY);
		isOK = sscanf(txts, "%lf, %lf, %lf", offset, offset + 1, offset + 2);
	}

	return rtn;
}

int JAKAZuRobot::setRapidRate(double rapid_rate)
{
	char sendCmd[BUFF_SIZE];
	sprintf(sendCmd, "{\"cmdName\":\"rapid_rate\",\"rate_value\":%lf}", rapid_rate);

	char recvBuff[BUFF_SIZE];
	int rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
	rtn = recv(m_socketClient, recvBuff, MAX_BUFF_SIZE, 0);
	if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;

	int isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
	char errs[8];
	isOK = findFieldText(recvBuff, "errorCode", errs, NUMBER);
	return atoi(errs);
}

int JAKAZuRobot::moveJ(const double *joint_pos, double speed, int move_mode)
{
	char sendCmd[BUFF_SIZE] = "{\"cmdName\":\"joint_move\",\"jointPosition\":";
	char strParams[BUFF_SIZE];

	sprintf(strParams, "[%f,%f,%f,%f,%f,%f],\"speed\":%f,\"relFlag\":%d}", 
		joint_pos[0], joint_pos[1], joint_pos[2], joint_pos[3], joint_pos[4], joint_pos[5],speed, move_mode);
	strcat(sendCmd, strParams);
	
	int rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
	if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;

	char recvBuff[BUFF_SIZE];
	rtn = recv(m_socketClient, recvBuff, BUFF_SIZE, 0);
	if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;

	int isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
	char errs[8];
	isOK = findFieldText(recvBuff, "errorCode", errs, NUMBER);
	return atoi(errs);
}

int JAKAZuRobot::moveP(const double *endPos, double speed)
{
	char sendCmd[BUFF_SIZE] = "{\"cmdName\":\"end_move\",\"endPosition\":";
	char strParams[BUFF_SIZE], recvBuff[BUFF_SIZE];

	sprintf(strParams, "[%f,%f,%f,%f,%f,%f],\"speed\":%f}",
		endPos[0], endPos[1], endPos[2], endPos[3], endPos[4], endPos[5], speed);
	strcat(sendCmd, strParams);

	int rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);		//发送命令字符串
	if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;

	rtn = recv(m_socketClient, recvBuff, BUFF_SIZE, 0);
	if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;

	int isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
	char errs[8];
	isOK = findFieldText(recvBuff, "errorCode", errs, NUMBER);
	return atoi(errs);
}

int JAKAZuRobot::moveL(const double *endPos, double speed, int move_mode)
{
	char sendCmd[BUFF_SIZE] = "{\"cmdName\":\"moveL\",\"cartPosition\":";
	char strParams[BUFF_SIZE], recvBuff[BUFF_SIZE];

	sprintf(strParams, "[%f,%f,%f,%f,%f,%f],\"speed\":%f,\"relFlag\":%d}",
		endPos[0], endPos[1], endPos[2], endPos[3], endPos[4], endPos[5], speed, move_mode);
	strcat(sendCmd, strParams);

	int rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);		//发送命令字符串
	if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;

	rtn = recv(m_socketClient, recvBuff, BUFF_SIZE, 0);
	if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;

	int isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
	char errs[8];
	isOK = findFieldText(recvBuff, "errorCode", errs, NUMBER);
	return atoi(errs);
}

int JAKAZuRobot::stopMotion()
{
	char sendCmd[BUFF_SIZE] = "{\"cmdName\":\"stop_program\"}";

	char recvBuff[BUFF_SIZE];
	int rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
	rtn = recv(m_socketClient, recvBuff, MAX_BUFF_SIZE, 0);
	if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;

	int isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
	char errs[8];
	isOK = findFieldText(recvBuff, "errorCode", errs, NUMBER);
	return atoi(errs);
}

int JAKAZuRobot::waitEndMove(int waitTime)
{
	LARGE_INTEGER start_t, end_t, frequency;
	QueryPerformanceFrequency(&frequency);	//获取计时器的时钟频率
	QueryPerformanceCounter(&start_t);		//获得计数器初始值

	char recvBuff[MAX_BUFF_SIZE];
	int rtn = 0;
	double tt = 0, tt_thre = 100.0;

	/*有可能存储运动前的状态，因此先保证能够获得运动中状态信息,
	*如果超过tt_thre时间后还是运动完成状态，那么就直接返回*/
	do	
	{
		rtn = recv(m_socketData, recvBuff, MAX_BUFF_SIZE, 0);
		if (rtn < 0) return ROBOT_ERROR;

		char *ptr_inpos = strstr(recvBuff, "\"inpos\"");
		char *ptr_ain = NULL;
		if (ptr_inpos) 	ptr_ain = strstr(recvBuff, "\"ain\"");
		if (ptr_ain)		//只有当“inpos”和“ain”这两个字段前后出现时才能保证inpos内容的完整性
		{
			char fieldText[8];
			int num = sscanf(ptr_inpos, "%*[^:]:%[^,]", fieldText);
			if (strstr(fieldText, "true")) rtn = 1;
			else rtn = 0;
		}

		QueryPerformanceCounter(&end_t);
		tt = (end_t.QuadPart - start_t.QuadPart)*1000.0 / frequency.QuadPart;
	} while (rtn == 1 && tt < tt_thre);	

	if (tt >= tt_thre) return ROBOT_NOERR;

	do 
	{
		rtn = recv(m_socketData, recvBuff, MAX_BUFF_SIZE, 0);
		if (rtn < 0) return ROBOT_ERROR;

		char *ptr_inpos = strstr(recvBuff, "\"inpos\"");
		char *ptr_ain = NULL;
		if (ptr_inpos) 	ptr_ain = strstr(ptr_inpos, "\"ain\"");
		if (ptr_ain)
		{
			char fieldText[8];
			int num = sscanf(ptr_inpos, "%*[^:]:%[^,]", fieldText);
			if (strstr(fieldText, "true")) rtn = 1;
			else rtn = 0;
		}

		QueryPerformanceCounter(&end_t);
		tt = (end_t.QuadPart - start_t.QuadPart)*1000.0 / frequency.QuadPart;
	} while (rtn == 0 && tt < waitTime);

	if (tt >= waitTime) return ROBOT_ERROR;
	else return (int)tt;
}

int JAKAZuRobot::enableCVMotion(int enable /* = 1 */)
{
	char sendCmd[64];

	sprintf(sendCmd, "{\"cmdName\":\"servo_move\",\"relFlag\":%d}", enable);

	char recvBuff[BUFF_SIZE];
	int rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
	rtn = recv(m_socketClient, recvBuff, MAX_BUFF_SIZE, 0);
	if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;

	int isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
	char errs[8];
	isOK = findFieldText(recvBuff, "errorCode", errs, NUMBER);
	return atoi(errs);
}

int JAKAZuRobot::cvMove(double *positions, int pts_num, int coord_type /* = COORD_JOINT */, int move_mode /* = 0 */)
{
	char sendCmd[BUFF_SIZE], cmdName[64], recvBuff[BUFF_SIZE];

	if (coord_type == COORD_JOINT)
	{
		strcpy(cmdName, "{\"cmdName\":\"servo_j\",\"jointPosition\":");
	}
	else
	{
		strcpy(cmdName, "{\"cmdName\":\"servo_p\",\"catPosition\":");
	}

	LARGE_INTEGER start_t, end_t, frequency;
	QueryPerformanceFrequency(&frequency);	//获取计时器的时钟频率
	double *ptr = positions;
	for (int i = 0; i < pts_num; i++)
	{
		QueryPerformanceCounter(&start_t);		//获得计数器初始值
		sprintf(sendCmd, "%s[%lf,%lf,%lf,%lf,%lf,%lf],\"relFlag\":%d}",
			cmdName,ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], move_mode);
		ptr += 6;

		int rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);		//发送命令字符串
		if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;

		rtn = recv(m_socketClient, recvBuff, BUFF_SIZE, 0);
		if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;

		char errs[8];
		int isOK = findFieldText(recvBuff, "errorCode", errs, NUMBER);
		rtn = atoi(errs);
		if (rtn != 0)
		{
			isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
			//return rtn;													//绝对运动时返回的字符串不合规(应该是内部有问题)导致直接跳出
		}
		//printf("pt_num = %d\n", i);
		do
		{
			QueryPerformanceCounter(&end_t); //获得计数器终止值
		} while ((end_t.QuadPart - start_t.QuadPart) * 1000.0 <= 8.0 * frequency.QuadPart);
	}

	return ROBOT_NOERR;
}

int JAKAZuRobot::fKinematics(double *joint_pos, double *endPos)
{
	char sendCmd[BUFF_SIZE] = "{\"cmdName\":\"kine_forward\",\"jointPosition\":";
	char strParams[BUFF_SIZE];

	sprintf(strParams, "[%f,%f,%f,%f,%f,%f]}",
		joint_pos[0], joint_pos[1], joint_pos[2], joint_pos[3], joint_pos[4], joint_pos[5]);
	strcat(sendCmd, strParams);

	int rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
	if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;

	char recvBuff[BUFF_SIZE];
	rtn = recv(m_socketClient, recvBuff, BUFF_SIZE, 0);
	if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;

	char errs[8];
	int isOK = findFieldText(recvBuff, "errorCode", errs, NUMBER);
	rtn = atoi(errs);
	if (rtn != 0)
	{
		isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
		return rtn;
	}

	char *posbuf = sendCmd;
	isOK = findFieldText(recvBuff, "cartPosition", posbuf, ARRAY);
	isOK = sscanf(posbuf, "%lf,%lf,%lf,%lf,%lf,%lf",
		endPos, endPos + 1, endPos + 2, endPos + 3, endPos + 4, endPos + 5);

	return rtn;
}

int JAKAZuRobot::iKinematics(double *ref_joints, double *endPos, double *joint_pos)
{
	char sendCmd[BUFF_SIZE] = "{\"cmdName\":\"kine_inverse\",\"jointPosition\":";
	char strParams[BUFF_SIZE];

	sprintf(strParams, "[%f,%f,%f,%f,%f,%f],\"cartPosition\":[%f,%f,%f,%f,%f,%f]}",
		ref_joints[0], ref_joints[1], ref_joints[2], ref_joints[3], ref_joints[4], ref_joints[5],
		endPos[0], endPos[1], endPos[2], endPos[3], endPos[4], endPos[5]);
	strcat(sendCmd, strParams);

	int rtn = send(m_socketClient, sendCmd, strlen(sendCmd), 0);
	if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;

	char recvBuff[BUFF_SIZE];
	rtn = recv(m_socketClient, recvBuff, BUFF_SIZE, 0);
	if (rtn == SOCKET_ERROR)	return ROBOT_ERROR;

	char errs[8];
	int isOK = findFieldText(recvBuff, "errorCode", errs, NUMBER);
	rtn = atoi(errs);
	if (rtn != 0)
	{
		isOK = findFieldText(recvBuff, "errorMsg", m_errMsg);
		return rtn;
	}

	char *posbuf = sendCmd;
	isOK = findFieldText(recvBuff, "jointPosition", posbuf, ARRAY);
	isOK = sscanf(posbuf, "%lf,%lf,%lf,%lf,%lf,%lf",
		joint_pos, joint_pos + 1, joint_pos + 2, joint_pos + 3, joint_pos + 4, joint_pos + 5);

	return rtn;
}

//求解:a*sin(theta)+b*cos(theta) = c;返回2个q解
void slvTrigonometry(double a, double b, double c, double *q)
{
	q[0] = atan2(b, -a) - atan2(c, sqrt(a*a + b*b - c*c));
	q[1] = atan2(b, -a) - atan2(c, -sqrt(a*a + b*b - c*c));
}


int JAKAZuRobot::iKinematics(double T[4][4], double *joint_pos)
{
	double a2 = 0.246, a3 = 0.190;
	double d1 = 0.15055, d2 = 0.115, d3 = 0.1163;
	double d4 = 0.1175, d5 = 0.1175, d6 = 0.105;

	double q1[8], q2[8], q3[8], q4[8], q5[8], q6[8];
	char flag[8] = { 0 };		//标记每个解是否合理

	slvTrigonometry(T[0][3] - T[0][2]*d6, -T[1][3] + d6*T[1][2], -d2 + d3 - d4, q1);
	q5[0] = acos(T[0][2] *sin(q1[0]) - T[1][2]*cos(q1[0]));
	q5[1] = acos(T[0][2] * sin(q1[1]) - T[1][2] * cos(q1[1]));
	q1[2] = q1[0]; q1[3] = q1[1];
	q5[2] = -q5[0]; q5[3] = -q5[1];

	int nslv = 0;				//有效解个数
	for (int i = 0; i < 4; i ++)
	{
		double aa = (T[0][1]*sin(q1[i]) - T[1][1]*cos(q1[i])) / -sin(q5[i]);
		double bb = (T[0][0]*sin(q1[i]) - T[1][0]*cos(q1[i])) / sin(q5[i]);
		q6[i] = atan2(aa, bb);

		double q234 = atan2(-T[2][2] / sin(q5[i]), -(cos(q1[i])*T[0][2] + sin(q1[i])*T[1][2]) / sin(q5[i]));

		aa = T[0][3]*cos(q1[i]) + T[1][3]*sin(q1[i]) - d5*sin(q234) + d6*sin(q5[i])*cos(q234);
		bb = T[2][3] - d1 + d5*cos(q234) + d6*sin(q5[i])*sin(q234);
		double tt = (aa*aa + bb*bb - a2*a2 - a3*a3) / 2 / a2 / a3;

		if (fabs(tt) <= 1.0)
		{
			q3[i] = acos(tt);
			double ab = a2*a2 + a3*a3 + 2 * cos(q3[i])*a2*a3;
			double s2 = (-a3*sin(q3[i])*aa + (a2 + a3*cos(q3[i]))*bb) / ab;
			double c2 = ((a2 + a3*cos(q3[i]))*aa + a3*sin(q3[i])*bb) / ab;
			q2[i] = atan2(s2, c2);
			q4[i] = q234 - q2[i] - q3[i];

			flag[i] = flag[4 + i] = 1;		//标记有效解
			nslv += 2;

			q3[4+i] = -q3[i];
			//ab = a2*a2 + a3*a3 + 2 * cos(q3[i])*a2*a3;
			s2 = (a3*sin(q3[i])*aa + (a2 + a3*cos(q3[i]))*bb) / ab;
			c2 = ((a2 + a3*cos(q3[i]))*aa - a3*sin(q3[i])*bb) / ab;
			q2[4+i] = atan2(s2, c2);
			q4[4+i] = q234 - q2[4+i] - q3[4+i];
			q1[4 + i] = q1[i];	q5[4 + i] = q5[i]; q6[4 + i] = q6[i];
		}
	}

	if (joint_pos == NULL)
	{
		joint_pos = new double[nslv];
	}
	double *ppos = joint_pos;
	for (int i = 0; i < 8; i ++)
	{
		if (flag[i])
		{
			ppos[0] = q1[i]; ppos[1] = q2[i]; ppos[2] = q3[i];
			ppos[3] = q4[i];ppos[4] = q5[i]; ppos[5] = q6[i];
			ppos += 6;
		}
	}

	//控制角度范围为(-pi, pi]
	for (int i = 0; i < nslv*6; i++)
	{
		if (joint_pos[i] > M_PI) joint_pos[i] -= 2*M_PI;
		if (joint_pos[i] <= -M_PI) joint_pos[i] += 2*M_PI;
	}

	return nslv;
}