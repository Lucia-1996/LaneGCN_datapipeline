#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <algorithm>
#include <set>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <iomanip>


// struct Point
// {
//     double x;
//     double y;
//     Point()
//     {
//         x = 0;
//         y = 0;
//     }
// };

void read_csv_1(std::string csv_path,
std::map<int, std::vector<std::string>> &csv_content,
std::vector<std::string> &time_stamp,
std::vector<Eigen::Vector2d> &trajs){
std::ifstream inFile(csv_path);
std::string linestr;

std::getline(inFile,linestr);

std::vector<std::string> items;
std::vector<std::string> values;
Eigen::Vector2d pos;

int counter = 0;
while(std::getline(inFile,linestr)){
    std::stringstream ss(linestr);
    std::string str;
    
    while(std::getline(ss,str, ',')){
        items.push_back(str);
    }
    pos.x() = std::atof(items[4].c_str());
    pos.y() = std::atof(items[5].c_str());
    values.push_back(items[2]);
    values.push_back(items[3]);
    time_stamp.push_back(items[1]);
    trajs.push_back(pos);

    csv_content.insert(std::make_pair(counter, values));
    items.clear();
    values.clear();
    counter = counter + 1;
}
}

void get_traj_step(std::map<int, std::vector<std::string>> &csv_content,
                   std::vector<Eigen::Vector2d> trajs,
                   std::vector<std::string> time_stamp,
                   std::vector<std::vector<int>> &step,
                   std::vector<std::vector<Eigen::Vector2d>> &traj)
{
    std::vector<int> agent_idcs;
    std::vector<std::string> agt_time;
    std::map<std::string, std::vector<int>> actor_idcs;

    



 
    std::map<int, std::vector<std::string>>::iterator it = csv_content.begin();
    std::map<int, std::vector<std::string>>::iterator it_end = csv_content.end();

    while (it != it_end)
    {
        if (it->second[1] == "AGENT")
        {
            agent_idcs.push_back(it->first);
            agt_time.push_back(time_stamp[it->first]);
        }
        else
        {
            actor_idcs[it->second[0]].push_back(it->first);
        }
        it++;
    }

    int time_size = agt_time.size();
    std::map<std::string, int> time_index;
    //已知：agent的时间戳包含整个csv文件中的所有时间戳
    for (int i = 0; i < time_size; i++)
    {
        time_index.insert(std::make_pair(agt_time[i], i));
    }
    std::vector<int> steps;
    for (auto t : time_stamp)
    {
        steps.push_back(time_index[t]);
    }
    std::vector<int> agt_step;
    std::vector<Eigen::Vector2d> agt_traj;
    for (int i : agent_idcs)
    {
        agt_traj.push_back(trajs[i]);
        agt_step.push_back(steps[i]);
    }
    traj.push_back(agt_traj);
    step.push_back(agt_step);

    for (const auto &actor : actor_idcs){
    std::vector<Eigen::Vector2d> actor_traj;
    std::vector<int> actor_step;
    for (const auto &it : actor.second) {
        actor_traj.push_back(trajs[it]);
        actor_step.push_back(steps[it]);
    }
    traj.push_back(actor_traj);
    step.push_back(actor_step);
    }
    // std::map<std::string, std::vector<int>>::iterator a_t = actor_idcs.begin();
    // std::map<std::string, std::vector<int>>::iterator a_t_end = actor_idcs.end();

    // while (a_t != a_t_end)
    // {
    //     for (int i : a_t->second)
    //     {
    //         actor_traj.push_back(trajs[i]);
    //         actor_step.push_back(steps[i]);
    //     }
    //     traj.push_back(actor_traj);
    //     step.push_back(actor_step);
    //     a_t++;
    //     actor_step.clear();
    //     actor_traj.clear();
    // }
}

void get_agent_feats(std::vector<std::vector<Eigen::Vector2d>> traj,
                     std::vector<std::vector<int>> step,
                     std::vector<Eigen::Matrix<double, 20, 3, Eigen::RowMajor>> &feats,
                     std::vector<std::vector<double>> &ctrs,
                     std::vector<Eigen::Matrix<double, 30, 2, Eigen::RowMajor>> &gt_preds,
                     std::vector<std::vector<bool> >&has_preds,
                     double &theta,
                     Eigen::Matrix<double, 2, 2, Eigen::RowMajor> &rot)
{

    float orig[2];
    orig[0] = traj[0][19].x();
    orig[1] = traj[0][19].y();

    theta = atan2(traj[0][18].y() - orig[1], traj[0][18].x() - orig[0]);
    theta = M_PI - theta;
    rot(0, 0) = cos(theta);
    rot(0, 1) = -sin(theta);
    rot(1, 0) = -rot(0, 1);
    rot(1, 1) = rot(0, 0);

    Eigen::Matrix<double, 20, 3, Eigen::RowMajor> feat;
    Eigen::Matrix<double, 20, 2, Eigen::RowMajor> traject;
    Eigen::Matrix<double, 30, 2, Eigen::RowMajor> gt_pred;
    std::vector<int>::iterator ite0;
    std::map<int,int> post_step;\

    // int has_pred[30] = {0};
    std::vector<double> ctr;

    for (int i = 0; i < traj.size(); i++)
    {
        //不含第20时刻的，过滤（python逻辑）
        if (std::find(step[i].begin(), step[i].end(), 19) == step[i].end())
        {
            continue;
        }

        gt_pred.setZero();
        std::vector<bool> has_pred(30, false);
        // memset(has_pred, 0, sizeof(int)*30);
        int index = 0;
    for (const auto &s : step[i]) {
      if (s >= 20 && s < 50) {
        gt_pred(s - 20, 0) = traj[i][index].x();
        gt_pred(s - 20, 1) = traj[i][index].y();
        has_pred[s - 20] = true;
      }
      ++index;
    }
        // ite0 = find(step[i].begin(), step[i].end(), 50);
        // auto gt_end = std::distance(std::begin(step[i]), ite0);//如果查找不到，返回step[i].end()
        // ite0 = step[i].begin();
        // while(ite0 != step[i].end()){
        //     if (*ite0>=20){
        //         break;
        //     }
        //     ite0++;
        // }
        // ite0 = find(step[i].begin(), step[i].end(), 20);
        // auto gt_start = std::distance(std::begin(step[i]), ite0);

        // while(ite0 !=step[i].end() && gt_start != gt_end){
        //     post_step[gt_start] = step[i][gt_start]-20;
        //     gt_start++;
        // }
        // std::map<int,int> ::iterator iter;
        // for (iter = post_step.begin();iter != post_step.end();iter++){
        //     gt_pred(iter->second,0) = traj[i][iter->first].x();
        //     gt_pred(iter->second,1) = traj[i][iter->first].y();
        //     has_pred[iter->second] = 1;
        // }

        feat.setZero();
        traject.setZero();

        int beg = 19;
        for (; beg >= 0; --beg)
        {
            if (std::find(step[i].begin(), step[i].end(), beg) == step[i].end())
            {
                break;
            }
        }
        ++beg;
        int end = 19;
        ite0 = find(step[i].begin(), step[i].end(), beg);
        auto index_beg = std::distance(std::begin(step[i]), ite0);
        int length = end - beg;
        for (int j = 0; j <= length; j++)
        {
            traject(j, 0) = traj[i][index_beg].x() - orig[0];
            traject(j, 1) = traj[i][index_beg].y() - orig[1];
            index_beg++;
        }
        auto tmp = (rot * (traject.transpose())).transpose();
        for (int j = beg; j < 20; ++j)
        {
            feat(j, 0) = tmp(j - beg, 0);
            feat(j, 1) = tmp(j - beg, 1);
            feat(j, 2) = 1.0;
        }
        ctr.push_back(feat(19, 0));
        ctr.push_back(feat(19, 1));
        ctrs.push_back(ctr);

        double xpre = feat(0, 0);
        double ypre = feat(0, 1);
        double xtmp, ytmp;
        for (int j = 1; j < 20; ++j)
        {
            xtmp = feat(j, 0);
            ytmp = feat(j, 1);
            feat(j, 0) -= xpre;
            feat(j, 1) -= ypre;
            xpre = xtmp;
            ypre = ytmp;
        }
        feat(beg, 0) = feat(beg, 1) = 0.0;
        feats.push_back(feat);
        gt_preds.push_back(gt_pred);
        // std::vector<int> has_predd(has_pred,has_pred+30);
        has_preds.push_back(has_pred);

        post_step.clear();
        ctr.clear();
    }
}

void print_feat(Eigen::Matrix<double, 20, 3, Eigen::RowMajor> feat){
    for (int i = 0;i<20;i++){
        for(int j = 0;j<3;j++){
            std::cout<<feat(i,j)<<",";
        }
        std::cout<<std::endl;
        
    }
    std::cout<<"==============================="<<std::endl;
}

void print_gt_pred(Eigen::Matrix<double, 30, 2, Eigen::RowMajor> gt_pred){
    std::cout.setf(std::ios::fixed,std::ios::floatfield);
    for (int i = 0;i<30;i++){
        for(int j = 0;j<2;j++){
            std::cout<<std::setprecision(5) << double(gt_pred(i,j))<<",";
        }
        std::cout<<std::endl;
    }
    std::cout<<"==============================="<<std::endl;
}

int main()
{
    std::string csv_path = "740.csv";
    std::map<int, std::vector<std::string>> csv_content;
    std::vector<std::string> time_stamp;
    std::vector<Eigen::Vector2d> trajs;
    std::vector<std::vector<int>> step;
    std::vector<std::vector<Eigen::Vector2d>> traj;

    std::vector<Eigen::Matrix<double, 20, 3, Eigen::RowMajor>> feats;
    std::vector<std::vector<double>> ctrs;
    std::vector<Eigen::Matrix<double, 30, 2, Eigen::RowMajor>> gt_preds;
    std::vector<std::vector<bool>> has_preds;
    double theta;
    Eigen::Matrix<double, 2, 2, Eigen::RowMajor> rot;

    read_csv_1(csv_path, csv_content, time_stamp, trajs);
    get_traj_step(csv_content, trajs, time_stamp, step, traj);
    get_agent_feats(traj,step,feats,ctrs,gt_preds,has_preds,theta,rot);
    
    for (int i = 0; i < feats.size(); i++){
        print_feat(feats[i]);
    }
    for (int i = 0; i < gt_preds.size(); i++){
        print_gt_pred(gt_preds[i]);
    }
}
