#include "user_lib.h"


void exportToCSV(const vector<int>& s, const string& filename)
{
    ofstream file(filename);

    if (!file.is_open())
    {
        cerr << "Failed to open file." << endl;
        return;
    }
    for (size_t i = 0; i < s.size(); ++i) {
        file << i << "," << s[i] << "\n";
    }

    file.close();
    cout << "Data exported to " << filename << endl;
}
/**
 * @brief 五次多项式轨迹生成
 * @param input_step_num 步数
 * @param max_out 最大输出
 * @param s 轨迹向量
 */
void tpoly(int input_step_num, int max_out, vector<int>& s)
{
    s.clear();
    s.reserve(input_step_num);

    // 五次多项式的系数，这里仅作为示例，实际系数应根据具体需求调整
    double a = 6.0 * max_out;
    double b = -15.0 * max_out;
    double c = 10.0 * max_out;
    double d = 0;
    double e = 0;
    double f = 0;

    for (int i = 0; i < input_step_num; ++i) {
        double t = static_cast<double>(i) / input_step_num;
        double position = a*pow(t,5) + b*pow(t,4) + c*pow(t,3) + d*pow(t,2) + e*t + f;
        s.push_back(static_cast<int>(round(position)));
    }
}
