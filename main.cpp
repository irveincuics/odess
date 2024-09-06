#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <locale>
#include <codecvt>
#include <algorithm>
#include <fcntl.h>
#include "N_Transform.cpp"
#include "Odess.cpp"

using namespace std;
namespace fs = filesystem;

vector<uint8_t> stringToVector(const string& input) {
    return vector<uint8_t>(input.begin(), input.end());
}

vector<vector<uint8_t>> readFileInChunks(const wstring& filename, size_t chunkSize) {
    vector<vector<uint8_t>> chunks;
    string filename_mb = wstring_convert<codecvt_utf8<wchar_t>>{}.to_bytes(filename);
    ifstream file(filename_mb, ios::binary);

    if (!file.is_open()) {
        wcerr << L"无法打开文件: " << filename << endl;
        return chunks;
    }

    vector<uint8_t> buffer(chunkSize);
    while (file.read((char*)(buffer.data()), chunkSize)) {
        chunks.push_back(buffer);
    }

    if (file.gcount() > 0) {
        buffer.resize(file.gcount());
        chunks.push_back(buffer);
    }

    file.close();
    return chunks;
}

void processFile(const fs::path& inputFilePath, const fs::path& outputFilePath, size_t chunkSize, int num_sketch, int num_group, unsigned long long mask) {
    wstring filename = inputFilePath.wstring();

    // 读取文件内容
    vector<vector<uint8_t>> fileChunks = readFileInChunks(filename, chunkSize);
    if (fileChunks.empty()) {
        wcerr << L"文件内容为空或读取失败: " << filename << endl;
        return;
    }

    wcout << L"成功读取文件内容，块数量为: " << fileChunks.size() << endl;
    //N_Transform odess(num_sketch, num_group, 32);
    Odess odess(num_sketch, num_group, mask);
    vector<int> sortedIndexes;             //已经排好的块的编号
    unordered_set<int> usedIndexes;       //已经进行过相似度检测的块
    sortedIndexes.push_back(0);
    usedIndexes.insert(0);

    //对文件块进行重新排序
    while (usedIndexes.size() < fileChunks.size()) {
        int lastIndex = sortedIndexes.back();            //
        auto sketch1 = odess.build_sketch(fileChunks[lastIndex]);

        vector<pair<int, double>> similarityScores;     //记录与当前块相似的块的编号以及相似度
        for (size_t j = lastIndex; j < fileChunks.size(); ++j) {
            if (usedIndexes.find(j) == usedIndexes.end()) {                //如果之前没有被判断出相似 则这里进行判断
                auto sketch2 = odess.build_sketch(fileChunks[j]);
                double score;
                bool is_similar = odess.resemble(sketch1, sketch2, score);
                if (is_similar) {
                    similarityScores.push_back({j, score});
                }
                if(score != 0)
                    wcout << L"块 " << lastIndex << L" 与块 " << j << L" 的相似度: " << score << endl;
            }
        }

        if (!similarityScores.empty()) {
            sort(similarityScores.begin(), similarityScores.end(), [](const pair<int, double>& a, const pair<int, double>& b) {
                return a.second > b.second;
            });

            for (const auto& pair : similarityScores) {
                sortedIndexes.push_back(pair.first);
                usedIndexes.insert(pair.first);
            }
        }
        //遍历寻找下一个没有被相似处理过的块并加入两个数组，等待下一个大循环
        for (size_t j = 0; j < fileChunks.size(); ++j) {
            if (usedIndexes.find(j) == usedIndexes.end()) {
                sortedIndexes.push_back(j);
                usedIndexes.insert(j);
                break;
            }
        }
    }

    wcout << L"排序后的块原始编号: " << endl;
    for (const auto& index : sortedIndexes) {
        wcout << index << L" ";
    }
    wcout << endl;

    // 创建输出目录（如果不存在）
    fs::create_directories(outputFilePath.parent_path());

    ofstream outputFile(outputFilePath, ios::binary);
    if (!outputFile.is_open()) {
        wcerr << L"无法打开输出文件: " << outputFilePath.wstring() << endl;
        return;
    }

    for (const auto& index : sortedIndexes) {
        outputFile.write((const char*)(fileChunks[index].data()), fileChunks[index].size());
    }
    outputFile.close();

    wcout << L"成功重新排列并写入文件: " << outputFilePath.wstring() << endl;
}

int main() {
    _setmode(_fileno(stdout), _O_U8TEXT);

    wcout << L"输入 : 分块大小（KB）| 特征数 | 超级特征数 | mask " << endl;
    size_t chunkSize;
    int num_sketch;
    int num_group;
    unsigned long long mask;
    wcin >> chunkSize >> num_sketch >> num_group;
    wcin >> hex >> mask;
    chunkSize *= 1024;

    fs::path inputDir = L"D:\\codefile\\odesss\\data\\silesia";
    fs::path outputDir = L"D:\\codefile\\odesss\\data\\sort_sil8";

    for (const auto& entry : fs::directory_iterator(inputDir)) {
        if (entry.is_regular_file()) {
            fs::path inputFilePath = entry.path();
            fs::path outputFilePath = outputDir / inputFilePath.filename();
            processFile(inputFilePath, outputFilePath, chunkSize, num_sketch, num_group, mask);
        }
    }

    return 0;
}
