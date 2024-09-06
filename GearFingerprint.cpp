
#include <iostream>
#include <vector>
#include <random>
#include <climits>
#include <unordered_set>
#include <algorithm>

using namespace std;

class GearFingerprint {

private:

    unsigned int hash;
    const unsigned long long base;
    const unsigned long long mod;
    vector<uint8_t> window;
    vector<unsigned long long> table;

public:
    GearFingerprint() :  hash(0), base(256), mod(1000000007) {
        mt19937 gen(42); // 固定种子
        uniform_int_distribution<unsigned long long> dis(1, UINT_MAX);

        table.resize(256); // 生成256个随机32位随机数
        for (int i = 0; i < 256; ++i) {
            table[i] = dis(gen);
        }
    }

    void update(uint8_t byte) {

        hash = (hash << 1) + table[byte];


    }

    unsigned int get_hash() const {
        return hash;
    }

    void clear() {
        hash = 0;
    }
};
