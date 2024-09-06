
#include <iostream>
#include <vector>
#include <random>
#include <climits>
#include <unordered_set>
#include <algorithm>

using namespace std;

class RabinFingerprint {

private:
    int window_size;                            //窗口大小
    unsigned long long hash;                    //哈希值
    const unsigned long long base;             //哈希函数基数
    const unsigned long long mod;
    unsigned long long highest_base;           //为了清除窗口最前端数据的基数，可变
    vector<uint8_t> window;                    //滑动窗口


public:
    RabinFingerprint(int window_size) : window_size(window_size), hash(0), base(256), mod(1000000007) {
        highest_base = 1;
        for (int i = 0; i < window_size - 1; ++i) {
            highest_base = (highest_base * base) % mod;
        }
    }

    void update(uint8_t byte) {
        if (window.size() == window_size) {
            // 移除最左边的字节
            uint8_t oldest_byte = window.front();
            window.erase(window.begin());
            hash = (hash - (oldest_byte * highest_base) % mod + mod) % mod;
        }

        // 添加新字节
        window.push_back(byte);
        hash = (hash * base + byte) % mod;
    }

    unsigned long long get_hash(){
        return hash;
    }

    void clear() {
        hash = 0;
        window.clear();
    }


};
