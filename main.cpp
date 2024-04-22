#include <iostream>
#include <unordered_map>
#include <vector>
#include <fstream>

using namespace std;

typedef struct key_freq_t {
    unordered_map<string, uint32_t> key_word_freq;
} key_freq_t;

typedef struct key_freq_count_t {
    unordered_map<uint32_t, key_freq_t> key_freq;
    uint32_t count;
} key_freq_count_t;

typedef struct key_freq_map_t {
    unordered_map<string, key_freq_count_t> map;
    unordered_map<uint32_t, key_freq_t> all_freq;
    uint32_t count;
} key_freq_map_t;

void get_key_freq_map(string filepath, key_freq_map_t *trainset)
{
    FILE *file = fopen(filepath.c_str(), "rb");

    if(!file) exit(-1);
    
    string key = "";
    string word = "";
    uint32_t count = 0;
    char c;
    do {
        c = fgetc(file);
        switch(c) {
            case ',': {
                if(count == 0) {
                    trainset->map[key].count++;
                } else {
                    trainset->map[key].key_freq[count].key_word_freq[word]++;
                    trainset->all_freq[count].key_word_freq[word]++;
                    word = "";
                }
                count++;
                break;
            }
            case '\r': {
                break;
            }
            case EOF:
            case '\n': {
                trainset->map[key].key_freq[count].key_word_freq[word]++;
                trainset->all_freq[count].key_word_freq[word]++;
                trainset->count++;
                count = 0;
                key = "";
                word = "";
                break;
            }
            default: {
                if(count == 0) {
                    key += c;
                } else {
                    word += c;
                }
                break;
            }
        }
    } while(c != EOF);
}

typedef struct input_t {
    string type;
    vector<string> values;
} input_t;

typedef vector<input_t> testset_t;

void get_testset(string filepath, testset_t *testset)
{
    FILE *file = fopen(filepath.c_str(), "rb");

    if(!file) exit(-1);
    
    string key = "";
    string word = "";
    input_t input = {};
    uint32_t count = 0;
    char c;
    do {
        c = fgetc(file);
        switch(c) {
            case ',': {
                if(count == 0) {
                    input.type = key;
                } else {
                    input.values.push_back(word);
                    word = "";
                }
                count++;
                break;
            }
            case '\r': {
                break;
            }
            case EOF:
            case '\n': {
                count = 0;
                input.values.push_back(word);
                testset->push_back(input);
                key = "";
                word = "";
                input = {};
                break;
            }
            default: {
                if(count == 0) {
                    key += c;
                } else {
                    word += c;
                }
                break;
            }
        }
    } while(c != EOF);
}

void classify(key_freq_map_t *trainset, testset_t *testset)
{
    uint32_t correct = 0;
    if(trainset->map.size() != 2) return;
    std::vector<std::string> keys;
    for (const auto& pair : trainset->map) {
        keys.push_back(pair.first);
    }
    uint32_t table[4] = {0, 0, 0, 0};
    for(input_t input : *testset) {
        unordered_map<string, float> key_probs;
        for(pair<string, key_freq_count_t> kfc_pair : trainset->map) {
            key_probs[kfc_pair.first] = kfc_pair.second.count / (float) trainset->count;
            uint32_t count = 1;
            for(string value : input.values) {
                uint32_t freq = kfc_pair.second.key_freq[count].key_word_freq[value];
                uint32_t total = kfc_pair.second.count;
                float partial_prob = 0.0f;
                if(freq > 0) {
                    partial_prob = freq / (float) total;
                } else {
                    freq++;
                    partial_prob = freq / (float)(total + trainset->all_freq[count].key_word_freq.size());
                }
                key_probs[kfc_pair.first] *= partial_prob;
                count++;
            }
        }
        pair<string, float> max_prob;
        for(pair<string, float> key_prob : key_probs) {
            if(key_prob.second > max_prob.second) {
                max_prob = key_prob;
            }
        }
        if(input.type == max_prob.first) correct++;
        if(input.type == keys.at(0)) {
            if(input.type == max_prob.first) {
                table[0]++;
            } else {
                table[1]++;
            }
        } else if(input.type == keys.at(1)) {
            if(input.type == max_prob.first) {
                table[3]++;
            } else {
                table[2]++;
            }
        }
    }
    float accuracy = correct / (float) testset->size();
    float precision = table[0] / (float) (table[0] + table[2]);
    float fullness = table[0] / (float) (table[0] + table[1]);
    float fmeasure = (2.0f * precision * fullness) / (precision + fullness);
    std::cout << "Accuracy = " << accuracy << endl;
    std::cout << "Precision = " << precision << endl;
    std::cout << "Fullness = " << fullness << endl;
    std::cout << "F-Measure = " << fmeasure << endl;
}

int main(void)
{
    key_freq_map_t trainset = {};
    get_key_freq_map("agaricus-lepiota.data", &trainset);

    testset_t testset = {};
    get_testset("agaricus-lepiota.test.data", &testset);
    classify(&trainset, &testset);
}