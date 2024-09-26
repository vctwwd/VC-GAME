#include <iostream>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <chrono>
#include <vector>
#include <random>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <conio.h>
#include <windows.h>

// Q 113
// W 119
// E 101
// R 114
// T 116
// Y 121
// U 117
// I 105
// O 111
// P 112
// A 97
// S 115
// D 100
// F 102
// G 103
// H 104
// J 106
// K 107
// L 108
// Z 122
// X 120
// C 99
// V 118
// B 98
// N 110
// M 109
// TAB 9
// BACKSPACE 8
// ENTER 13
// 1 49
// 2 50
// 3 51
// 4 52
// 5 53
// 6 54
// 7 55
// 8 56
// 9 57
// 0 48

// Utils
template<typename T>
inline void vectorDel(std::vector<T>& vec, int pos) {
    //if(pos >= vec.size()) return;
    auto it = vec.begin() + pos;
    vec.erase(it);
}

template<typename T>
inline void objectClean(std::vector<T>& vec, std::vector<int>& vx, std::vector<int>& vy, int id) {
    vectorDel<T>(vec, id);
    vectorDel<int>(vx, id);
    vectorDel<int>(vy, id);
}

// Printer

#define SCRNX 96
#define SCRNY 36
#define CHARY 18

class Printer {
public : 
    std::vector<std::vector<char>> map;

    Printer() {
        for(int i = 0; i < SCRNY; i++) {
            std::vector<char> row;
            for(int j = 0; j < SCRNX; j++) {
                if(i == 0 || j <= 1 || i == SCRNY - 1 || j >= SCRNX - 2) row.push_back('#');
                else row.push_back(' ');
            }
            map.push_back(row);
        }

        for(int i = 0; i < CHARY; i++) {
            std::vector<char> row;
            for(int j = 0; j < SCRNX; j++) {
                if(i == 0 || j <= 1 || i == CHARY - 1 || j >= SCRNX - 2) row.push_back('#');
                else row.push_back(' ');
            }
            map.push_back(row);
        }
    }

    void room(int type) {
        
        std::string room = "map\\room_";
        std::string str;
	    std::ifstream file;
        file.open(room + std::to_string(type) + ".txt", std::ios::in);
        if (!file.is_open())
    	{
        	   std::cout << "Cannot open " + room + std::to_string(type) + ".txt" + "." << std::endl;
    	}
        for(int i = 1; i < SCRNY - 1; i++) {
            getline(file, str);
            for(int j = 2; j < SCRNX - 2; j++) {
                map[i][j] = str[j-2];
            }
        }
    }

    void to_string() {
        for(int i = 0; i < 54; i++) {
            for(int j = 0; j < 96; j++) {
                std::cout<<map[i][j];
            }
            std::cout<<std::endl;
        }
    }
};

int main() {
	Printer GAME = Printer();
    GAME.room(15);
    GAME.to_string();
    return 0;
}
