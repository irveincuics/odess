#include <iostream>
#include <vector>
#include <random>
#include <climits>
#include <unordered_set>
#include <algorithm>
#include "GearFingerprint.cpp"
using namespace std;


class Odess  {


private:
    int num_sketch;                     //特征的数量，即线性变换式的数量
    int num_group;                      //超级特征的数量
    unsigned long long mask;
    GearFingerprint gear;
    vector<pair<unsigned long long, unsigned long long>> coefficients; //变换的系数



public:
    Odess(int num_sketch = 12 , int num_group = 3 ,unsigned long long mask  = 0x1c )
            : num_sketch(num_sketch), num_group(num_group), mask(mask){


        mt19937 gen(42);         //使用固定的随机数种子 确保参数的一致性
        uniform_int_distribution<unsigned long long> dis(1, UINT_MAX);

        for (int i = 0; i < num_sketch; ++i) {
            coefficients.emplace_back(dis(gen), dis(gen));
        }
    }


    //构建特征描述，即论文中伪代码部分

    vector<unsigned long long> build_sketch(const vector<uint8_t>& byte_data) {

        vector<unsigned long long> features(num_sketch, UINT_MAX);

        for (auto ch : byte_data) {
            gear.update(ch);

            unsigned long long fingerprint = gear.get_hash();
            if((fingerprint & mask) == 0) {
                for (int i = 0; i < num_sketch; ++i) {
                    unsigned long long m = coefficients[i].first;
                    unsigned long long a = coefficients[i].second;
                    unsigned long long linear_transform = (m * fingerprint + a) % UINT_MAX;
                    features[i] = min(features[i], linear_transform);
                    //cout << linear_transform << ' ';
                }
            }

        }
       // for(auto feature :features) {
        //    cout << feature << ' ' ;
       // }
       // cout << endl;

        //通过上面生成的特征构建super_features
        int step = num_sketch / num_group;
        vector<unsigned long long> super_features;
        for (int i = 0; i < num_sketch; i += step) {
            super_features.push_back(grouping(vector<unsigned long long>(features.begin() + i, features.begin() + i + step)));
        }
       // for(auto super : super_features){
        //    cout << super << ' ';
       // }
       // cout  << endl;
        return super_features;
    }

    unsigned long long grouping(vector<unsigned long long> feature_list) {
        gear.clear();
        for (auto feature : feature_list) {
            auto feature_bin = feature;            //逐字节处理feature
            for (int i = 0; i < 8; ++i) {
                gear.update((uint8_t)(feature_bin >> (8 * i)));
            }
        }
        //cout << gear.get_hash() << ' ';
        return gear.get_hash();
    }

    //broder的理论
    bool resemble(vector<unsigned long long> ref_sketch, vector<unsigned long long> inc_sketch, double& score) {
        unordered_set<unsigned long long> ref_set(ref_sketch.begin(), ref_sketch.end());
        int sum_size = ref_set.size();
        double frequency_sum = 0;
        for (auto item : inc_sketch) {
            if (ref_set.find(item) != ref_set.end()) {
                frequency_sum += 1;
            }
            else{
                sum_size ++;
            }
        }
        //cout << sum_size;

        score = sum_size > 0 ? frequency_sum / sum_size : 0.0;

        return score > 0;
    }

};
