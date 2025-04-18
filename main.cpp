/*****************************************************
 * @file main.cpp
 * @brief This is the main file for the project
 * It is used to test the Vision_Detector class
 * @date 2024-06-21
 *****************************************************/

#include "main.hpp"


#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#pragma warning(disable : 4996)
#endif


Vision_Detector cube_detector;
Robot robot;
int main() 
{
    cout << "Hello, World!" << endl;

    robot.robot_init();
    robot.robot_move_to_init_pos();
    robot.vision_target_detect();
    cout<<"If you accept the detection result?[Y/n]:";
    char choice;
    cin>>choice;

    if (choice == 'Y' || choice == 'y')
    {
        Point3f point = {robot.home_point.x,robot.home_point.y,robot.home_point.z-60};
        robot.robot_move_to_world_point_and_wait(point,0);
        robot.tool_open();
        Point3f end_point3f = {robot.home_point.x,robot.home_point.y,robot.home_point.z-60};
        Point3f start_point3f = end_point3f;

        for(int i = 0; i < 6; i++)
        {
            for(int j = 0; j < robot.cube_detector.cube[i].contour_num; j++)
            {
                start_point3f = end_point3f;
                Point2f  end_point2f = robot.cube_detector.cube[i].position_world[j];
                end_point3f = {end_point2f.x,end_point2f.y,CATCH_HEIGHT};
                robot.robot_carry_cube(start_point3f,end_point3f,0,(double)robot.cube_detector.cube[i].angle_world[j]);
                Sleep(2000);
                robot.tool_close();
                Sleep(2000);
                start_point3f = end_point3f;
                end_point3f = {robot.cube_settle_point[i].x,robot.cube_settle_point[i].y,RELEASE_HEIGHT+(float)CUBE_HEIGHT*j};
                robot.robot_carry_cube(start_point3f,end_point3f,(double)robot.cube_detector.cube[i].angle_world[j],0);
                Sleep(2000);
                robot.tool_open();
            }
        }
        point = {robot.home_point.x,robot.home_point.y,robot.home_point.z};
        robot.robot_move_to_world_point_and_wait(point,0);


        // for(int i = 0; i < 6; i++)
        // {
        //     for(int j = 0; j < robot.cube_detector.cube[i].contour_num; j++)
        //     {
        //         robot.robot_move_home_to_cube(i,j);
        //         Sleep(2000);
        //         robot.tool_close();
        //         robot.robot_carry_cube(i,j);
        //         Sleep(2000);
        //         robot.tool_open();
        //         robot.robot_move_cube_to_home(i,j);
        //     }
        // }
    }else
    {
        cout<<"There is something wrong with the detection result!"<<endl;
    }
    cout<<"Thank you for using!"<<endl;
    return 0;
}


Robot::Robot() 
{
    cout<<"Robot is created!"<<endl;
}

Robot::~Robot() 
{
    cout<<"Robot is destroyed!"<<endl;
}


void Robot::vision_target_detect() 
{
    cube_detector.cube_info_init();//初始化cube_info
    cube_detector.cube_detect();//检测颜色方块
    cube_detector.cube_image_save();//保存检测到的颜色方块图片
    cube_detector.cube_info_print();//打印检测到的颜色方块信息
}

int Robot::robot_init() 
{
    cout<<"ROBOT INITIALIZATION..."<<endl;
    //初始化机器人
    jaka_bot_ret = jaka_bot.login_in(JAKA_ZU3_IP);//登录机器人
    if (jaka_bot_ret)
    {
        printf("%s\n", jaka_bot.m_errMsg);//打印错误信息 
        cout<<"Login in failed!"<<endl;
        //退出程序
        return 0;
    }else
    {
        cout<<"Login in successfully!"<<endl;
        jaka_bot.getState(is_enabled, is_powered);//获取机器人状态
        if (!is_powered)//如果机器人没有上电
        {
            jaka_bot_ret = jaka_bot.power(1);
            cout<<"Waiting for power on..."<<endl;
            Sleep(6);
        }
        if (!is_enabled)
        {
            jaka_bot_ret = jaka_bot.enable(1);
            cout<<"Waiting for enable..."<<endl;
            Sleep(6);
        }
        cout<<"Waiting for robot initialization!"<<endl;
        jaka_bot.initSetting();
        Sleep(1000);
//        QueryPerformanceFrequency(&frequency);	//获取计时器的时钟频率
       jaka_bot.setRapidRate(1);
//        cout<<"Robot is initialized!"<<endl;
        cout<<"robot initialized"<<endl;
        return 1;
    }
}

int Robot::tool_open()
{
    int ret;
    ret = jaka_bot.setDout(TOOL_PORT,TOOL_OPEN);
    return ret;
}

int Robot::tool_close()
{
    int ret;
    ret = jaka_bot.setDout(TOOL_PORT,TOOL_CLOSE);
    return ret;
}

int Robot::robot_move_to_init_pos()
{
    cout<<"ROBOT MOVING TO INIT POSE..."<<endl;
    jaka_bot.moveJ(joints_init, 10.0, 0);
    for(int i = 0; i < 6; i++)
    {
        jaka_bot.waitEndMove();
        Sleep(1000);
        cout<<"wait:"<<i<<endl;
    }
    cout<<" "<<endl;
    return 1;
}

int Robot::robot_move_to_world_point_and_wait(Point3f point,double angle)
{
    cout<<"ROBOT MOVING TO WORLD POINT:"<<point<<"..."<<endl;
    Point2f point2f = {point.x,point.y};
    point2f = cube_detector.world_to_base_coordinate((Point)point2f);
    double position_test[6] = {point2f.x,point2f.y,point.z,180,0,angle+TOOL_ANGLE_DIFF};
    jaka_bot.moveL(position_test, 30.0, 0);
    for(int i = 0; i < 2; i++)
    {
        jaka_bot.waitEndMove();
        Sleep(500);
        cout<<"wait:"<<i<<endl;
    }
    cout<<"Robot is in world point:"<<point<<endl;
    return 1;
}



int Robot::robot_move_home_to_cube(int cube_color,int cube_num)
{
    

    Point2f start_point = {home_point.x,home_point.y};
    start_point = cube_detector.world_to_base_coordinate((Point)start_point);
    Point2f end_point = cube_detector.cube[cube_color].position_base[cube_num];
    Point2f mid_point = {(start_point.x+end_point.x)/2,(start_point.y+end_point.y)/2};
    float angle = cube_detector.cube[cube_color].angle_world[cube_num];

    int step_num = 1500;
    float step_time = 1.0f/(float)step_num;  

    Coord3D controlPoints[] =
    {
        { start_point.x,start_point.y,home_point.z},
        { mid_point.x,mid_point.y,home_point.z},
        // {(float)position_home[0],(float)position_home[1],(float)position_home[2]},

        {end_point.x,end_point.y,CATCH_HEIGHT+LIFT_HEIGHT},
        {end_point.x,end_point.y,CATCH_HEIGHT},
        {end_point.x,end_point.y,CATCH_HEIGHT},

    };  //控制点
    float knots[] = { 0,0,0,0,0.5,1,1,1,1 };//按这样给定的节点保证生成的B样条曲线会经过初末控制点
    BSpline spline(3, 4, knots, controlPoints);
    double pts[6*step_num];
    for(int j = 0; j < step_num; j++)
    {
        Coord3D point = spline.PtsInCur((float)j *step_time);
        pts[j * 6 + 0] = (double)point.x;
        pts[j * 6 + 1] = (double)point.y;
        pts[j * 6 + 2] = (double)point.z;
        pts[j * 6 + 3] = 180;
        pts[j * 6 + 4] = 0;
        pts[j * 6 + 5] = TOOL_ANGLE_DIFF+(double)angle*j *step_time;
    }
    jaka_bot_ret = jaka_bot.enableCVMotion(1);
    jaka_bot_ret = jaka_bot.cvMove(pts,step_num,COORD_WORD,0);
    jaka_bot_ret = jaka_bot.enableCVMotion(0);
    return 1;
}
int Robot::robot_move_cube_to_home(int cube_color,int cube_num)
{
    cout<<"ROBOT MOVING TO HOME..."<<endl;
    Point2f start_point = cube_settle_point[cube_color];
    start_point = cube_detector.world_to_base_coordinate((Point)start_point);
    Point2f end_point = {home_point.x,home_point.y};
    end_point = cube_detector.world_to_base_coordinate((Point)end_point);
    Point2f mid_point = {(start_point.x+end_point.x)/2,(start_point.y+end_point.y)/2};
    double angle = (double)cube_detector.cube[cube_color].angle_world[cube_num];
    float low_height = CATCH_HEIGHT+(float)CUBE_HEIGHT*cube_num;
    float high_height = home_point.z;
    float mid_height = (low_height+high_height)/2;
    Coord3D controlPoints[] =
    {
        {start_point.x,start_point.y,low_height},
        {start_point.x,start_point.y,mid_height},
        {start_point.x,start_point.y,high_height},
        {mid_point.x,mid_point.y,high_height},
        {end_point.x,end_point.y,high_height},

    };  //控制点
    int step_num = 1500;
    float step_time = 1.0f/(float)step_num;
    float knots[] = { 0,0,0,0,0.5,1,1,1,1 };//按这样给定的节点保证生成的B样条曲线会经过初末控制点
    BSpline spline(3, 4, knots, controlPoints);
    double pts[6*step_num];
    for(int j = 0; j < step_num; j++)
    {
        Coord3D point = spline.PtsInCur((float)j *step_time);
        pts[j * 6 + 0] = (double)point.x;
        pts[j * 6 + 1] = (double)point.y;
        pts[j * 6 + 2] = (double)point.z;
        pts[j * 6 + 3] = 180;
        pts[j * 6 + 4] = 0;
        pts[j * 6 + 5] = TOOL_ANGLE_DIFF;
    }
    jaka_bot_ret = jaka_bot.enableCVMotion(1);
    jaka_bot_ret = jaka_bot.cvMove(pts,step_num,COORD_WORD,0);
    jaka_bot_ret = jaka_bot.enableCVMotion(0);
    cout<<"FINISH MOVING TO HOME"<<endl;
    return 1;
}


int Robot::robot_carry_cube(int cube_color,int cube_num)
{
    cout<<"START CARRING "<<cube_detector.cube[cube_color].color_name<<"["<<cube_num<<"]"<<endl;
    Point2f start_point = cube_detector.cube[cube_color].position_base[cube_num];
    Point2f end_point = cube_settle_point[cube_color];
    end_point = cube_detector.world_to_base_coordinate((Point)end_point);
    double angle = (double)cube_detector.cube[cube_color].angle_world[cube_num];
    int step_num = 3000;
    float step_time = 1.0f/(float)step_num;  

    Coord3D controlPoints[] =
    {
        { start_point.x,start_point.y,CATCH_HEIGHT},
        { start_point.x,start_point.y,CATCH_HEIGHT+LIFT_HEIGHT},
        // {(float)position_home[0],(float)position_home[1],(float)position_home[2]},

        {end_point.x,end_point.y,CATCH_HEIGHT+LIFT_HEIGHT+(float)CUBE_HEIGHT*cube_num},
        {end_point.x,end_point.y,CATCH_HEIGHT+(float)CUBE_HEIGHT*cube_num},
        {end_point.x,end_point.y,CATCH_HEIGHT+(float)CUBE_HEIGHT*cube_num},

    };  //控制点
    float knots[] = { 0,0,0,0,0.5,1,1,1,1 };//按这样给定的节点保证生成的B样条曲线会经过初末控制点
    BSpline spline(3, 4, knots, controlPoints);
    double pts[6*step_num];
    for(int j = 0; j < step_num; j++)
    {
        Coord3D point = spline.PtsInCur((float)j *step_time);
        pts[j * 6 + 0] = (double)point.x;
        pts[j * 6 + 1] = (double)point.y;
        pts[j * 6 + 2] = (double)point.z;
        pts[j * 6 + 3] = 180;
        pts[j * 6 + 4] = 0;
        pts[j * 6 + 5] = TOOL_ANGLE_DIFF+angle-angle*j *step_time;
    }
    jaka_bot_ret = jaka_bot.enableCVMotion(1);
    jaka_bot_ret = jaka_bot.cvMove(pts,step_num,COORD_WORD,0);
    jaka_bot_ret = jaka_bot.enableCVMotion(0);
    cout<<"FINISH CARRING "<<cube_detector.cube[cube_color].color_name<<"["<<cube_num<<"]"<<endl;
    return 1;
}
int Robot::robot_carry_cube(Point3f start_point_world , Point3f end_point_world , double start_angle ,double end_angle)
{
    Point2f start_point = {start_point_world.x,start_point_world.y};
    start_point = cube_detector.world_to_base_coordinate(start_point);
    Point2f end_point = {end_point_world.x,end_point_world.y};
    end_point = cube_detector.world_to_base_coordinate(end_point);
    int height_diff = fabs(start_point_world.z-end_point_world.z);
    int distance = sqrt(pow(start_point.x-end_point.x,2)+pow(start_point.y-end_point.y,2));
    int traj_length = height_diff+distance+LIFT_HEIGHT*2;
    int step_num_calc = traj_length*8;
    cout<<"step_num_calc:"<<step_num_calc<<endl;
    int step_num = 2000;
    float step_time = 1.0f/(float)step_num;  
    Coord3D controlPoints[] =
    {
        {start_point.x,start_point.y,start_point_world.z},
        {start_point.x,start_point.y,start_point_world.z+LIFT_HEIGHT},
        {end_point.x,end_point.y,end_point_world.z+LIFT_HEIGHT},
        {end_point.x,end_point.y,end_point_world.z},
        {end_point.x,end_point.y,end_point_world.z},
    };
    float knots[] = { 0,0,0,0,0.5,1,1,1,1 };//按这样给定的节点保证生成的B样条曲线会经过初末控制点
    BSpline spline(3, 4, knots, controlPoints);
    double pts[6*step_num];
    for(int j = 0; j < step_num; j++)
    {
        Coord3D point = spline.PtsInCur((float)j *step_time);
        pts[j * 6 + 0] = (double)point.x;
        pts[j * 6 + 1] = (double)point.y;
        pts[j * 6 + 2] = (double)point.z;
        pts[j * 6 + 3] = 180;
        pts[j * 6 + 4] = 0;
        pts[j * 6 + 5] = TOOL_ANGLE_DIFF+start_angle+(end_angle-start_angle)*j *step_time;
    }
    jaka_bot_ret = jaka_bot.enableCVMotion(1);
    jaka_bot_ret = jaka_bot.cvMove(pts,step_num,COORD_WORD,0);
    jaka_bot_ret = jaka_bot.enableCVMotion(0);
    return 1;
}
int Robot::Robot::robot_move_cube_task()
{


    return 1;
}