#include <iostream>
#include <cstdio>
#include <cmath>
#include <numeric>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <random>
#include <windows.h>

#ifdef _WIN32
#include <Windows.h>
#endif

struct statistics {
    std::vector<int> f = { 0, 0, 0, 0, 0, 0 };
    int up = 0;
    double sum() { return (double)(f[1] + f[2] + f[3] + f[4] + f[5]); }
    double frequency_4x() { return (double)f[4] / sum(); }
    double frequency_5x() { return (double)f[5] / sum(); }
    double frequency_up() { return (double)up / sum(); }
    void print() { std::cout << "1* = " << f[1] << ", 2* = " << f[2] << ", 3* = " << f[3] << ", 4* = " << f[4] << ", 5* = " << f[5] << ", up = " << up << ", P(4*) = " << 100 * frequency_4x() << "%, P(5*) = " << 100 * frequency_5x() << "%, P(up) = " << 100 * frequency_up() << "%." << std::endl; }
};

class gacha {
public:
    int count_4x = 0;
    int count_5x = 0;
    int count_up = 0;

    std::vector<int> distribution;
    int floor_5x;
    int floor_inc;
    int inc_offset;
    int floor_4x;
    int floor_3_to_4;
    int floor_big;
    std::vector<std::vector<int>> weights;
    std::vector<std::vector<std::string>> items;

    std::random_device rd;
    std::mt19937 generator;

    bool debug = false;
    statistics frequency;

    gacha(std::vector<int> _distribution, int _floor_5x, int _floor_inc, int _inc_offset, int _floor_4x, int _floor_3_to_4, int _floor_big, std::vector<std::vector<int>> _weights, std::vector<std::vector<std::string>> _items) {
        generator = std::mt19937(rd());
        distribution = _distribution;
        floor_5x = _floor_5x;
    	floor_inc = _floor_inc;
    	inc_offset = _inc_offset;
    	floor_4x = _floor_4x;
    	floor_3_to_4 = _floor_3_to_4;
    	floor_big = _floor_big;
        weights = _weights;
        items = _items;
    }

    std::string gacha_api() {
        int floor = 1, offset = 0;
        if (count_4x + 1 == floor_4x) { count_4x = 0; if(floor_3_to_4 == 0) floor = 4; else floor = (generator() % 2) + 3; }
        if (count_5x + 1 > floor_inc) offset = (count_5x - floor_inc) * ( (-1) * inc_offset);
        if (count_5x + 1 == floor_5x) { count_5x = 0; floor = 5; }
        if (count_up + 1 == floor_big) { count_up = 0; if(debug) std::cout << "Gacha : up!"; frequency.up++; frequency.f[5]++; return items[5][0]; }
        if(debug) std::cout<<"Gacha : floor = "<<floor<<", offset = "<<offset<<", 4x = "<<count_4x<<", 5x = "<<count_5x<<", up = "<<count_up<<", ";
        return gacha_distributor(gacha_basic(floor, offset));
    }

    int gacha_basic(int floor, int offset) {
        int ans = (generator() % 1000000) + offset;
        for(int i = 4; i >= 0; i--) {
            if(ans < distribution[i]) {
                if (debug) std::cout<<"rand_1 = "<<ans<<", rare = "<<std::max(floor, i + 1)<<", ";
                frequency.f[std::max(floor, i + 1)]++;
                return std::max(floor, i + 1);
            }
        }
    }

    std::string gacha_distributor(int rare) {
        if(rare >= 4) count_4x = 0;
        else count_4x++;
        if(rare >= 5) count_5x = 0;
        else count_5x++;
        int ans = generator() % (distribution[rare-1] - distribution[rare]);
        int total = 0;
        for (int i = 0; i < items[rare].size(); i++) {
            if(ans < weights[rare][i] + total) {
                if (rare == 5 && i == 0) { count_up = 0; frequency.up++;}
                else count_up++;
                if (debug) std::cout<<"rand_2 = "<<ans<<", from = "<< (distribution[rare-1] - distribution[rare]) << ", item = " << items[rare][i] << std::endl;
                return items[rare][i];
            }
            else total += weights[rare][i];
        }
    }
};

class inputer {
public : 
	std::string str;
	std::string word;
	std::ifstream file;
	std::vector<int> distribution;
	int floor_5x;
	int floor_inc;
	int inc_offset;
	int floor_4x;
	int floor_3_to_4;
	int floor_big;
	std::vector<double> ups_5x;
    std::vector<double> ups_4x;
    std::vector<std::vector<int>> weights;
    std::vector<std::vector<std::string>> items;
    
    inputer() {
    	file.open("input.txt", std::ios::in);
    	if (!file.is_open())
    	{
        	std::cout << "�ļ���ʧ�ܡ�" << std::endl;
    	}
    	input_distribution();
    	ups_5x = input_ups();
    	ups_4x = input_ups();
    	input_items_weights();
	}
	
	void input_distribution() {
		std::vector<int> _distribution;
    	getline(file, str);
    	std::string word;
    	std::istringstream ss(str);
    	while (ss >> word) _distribution.push_back(std::stoi(word));
    	distribution = _distribution;
    	getline(file, str);
		floor_5x = std::stoi(str);
		getline(file, str);
		floor_inc = std::stoi(str);
		getline(file, str);
		inc_offset = std::stoi(str);
		getline(file, str);
		floor_4x = std::stoi(str);
		getline(file, str);
		floor_3_to_4 = std::stoi(str);
		getline(file, str);
		floor_big = std::stoi(str);
	}
	
	std::vector<double> input_ups() {
		std::vector<double> ups;
		getline(file, str);
    	if (std::stoi(str) > 0) {
    		int num = std::stoi(str);
        	getline(file, str);
        	std::istringstream ss(str);
        	for (int i = 0; i < num; i++) {
            	ss >> word;
            	ups.push_back(std::stof(word));
        	}
    	}
    	return ups;
	}
    
    std::vector<int> weight_generator(int rare ,std::vector<int> distribution, std::vector<std::string> item, int up, std::vector<double> up_weight) {
    	std::vector<int> weight;
    	if (item.size() == 0) return weight;
    	else {
        	if (up == 0) {
            	for (int i = 0; i < item.size(); i++) {
                	weight.push_back((distribution[rare - 1] - distribution[rare]) / item.size());
            	}
        	}
        	else {
            	int up_total = 0;
            	for (int i = 0; i < up; i++) {
                	weight.push_back((distribution[rare - 1] - distribution[rare]) * up_weight[i]);
                	up_total += (distribution[rare - 1] - distribution[rare]) * up_weight[i];
            	}
            	for (int i = up; i < item.size(); i++) {
                	weight.push_back((distribution[rare - 1] - distribution[rare] - up_total) / (item.size() - up));
            	}
        	}
        	if (std::accumulate(weight.begin(), weight.end(), 0) != (distribution[rare - 1] - distribution[rare])) weight[item.size() - 1]++;
    	}
    	return weight;
	}
    
    void input_items_weights() {
    	std::vector<int> weight_empty;
    	std::vector<std::string> item_empty;
    	weights.push_back(weight_empty);
    	items.push_back(item_empty);
    	for (int i = 1; i <= 5; i++) {
        	int up;
        	std::vector<double> ups;
        	if (i <= 3) { up = 0; }
        	else if (i == 4) { up = ups_4x.size(); ups = ups_4x; }
        	else if (i == 5) { up = ups_5x.size(); ups = ups_5x; }
        	std::vector<int> weight;
        	std::vector<std::string> item;
        	getline(file, str);
        	if (std::stoi(str) > 0) {
            	int num = std::stoi(str);
            	getline(file, str);
            	std::istringstream ss(str);
            	for (int j = 0; j < num; j++) {
                	ss >> word;
                	item.push_back(word);
            	}
            	weight = weight_generator(i, distribution, item, up, ups);
        	}
        	weights.push_back(weight);
        	items.push_back(item);
    	}
	}
};

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
	int order;
	bool logout = false;
    inputer input;
    gacha machine(input.distribution, input.floor_5x, input.floor_inc, input.inc_offset, input.floor_4x, input.floor_3_to_4, input.floor_big, input.weights, input.items);
    std::cout<<"Input 1 gacha 1 time, input 2 gacha 10 times, input 3 check statistics, input 0 to quit."<<std::endl;
    std::cout<<"Order : ";
    while(std::cin >> order) {
    	switch(order) {
    		case 0 :
				logout = true;
    			break;
    		case 1 :
    			std::cout<<machine.gacha_api()<<std::endl;
    			Sleep(500);
    			break;
    		case 2 : 
    			for(int i=0;i<10;i++) { std::cout<<machine.gacha_api()<<std::endl; Sleep(500); }
    			break;
    		case 3 : 
    			machine.frequency.print();    			
				break;
			case 730907 : 
				std::cin >> order;
				for(int i=0;i<order;i++) { std::cout<<machine.gacha_api()<<std::endl; }
				break;
			default : 
				std::cout<<"Unknown order."<<std::endl; 
				break;
		}
		if(logout) break;
		std::cout<<"Input 1 gacha 1 time, input 2 gacha 10 times, input 3 check statistics, input 0 to quit."<<std::endl;
		std::cout<<"Order : ";
	}
    return 0;
}
