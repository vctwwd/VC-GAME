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

#define EMPTY 0
#define PLAYER 1
#define WALL 2
#define ENEMY 3
#define SHELL 4
#define WEAK 5

#define UP 0
#define LEFT 1
#define DOWN 2
#define RIGHT 3
#define ATTACK 4

#define ROUND_TIME 10
#define PLAYER_WAIT_ROUNDS 25
#define ENEMY_WAIT_ROUNDS 25
#define SHELL_WAIT_ROUNDS 25

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

// Interfaces
class map_object {
protected : 

    char icon;
    bool fixed;

public :

    int type;

    map_object() {};

    void set_icon(char in) { icon = in; }
    char to_char() { return icon; }
    bool cant_move() { return fixed; }

};

class wait_object {
protected : 

    int wait_time;

public : 

    wait_object() {};

    bool ready() { if( wait_time == 0 ) return true; else return false; };
    void count_up(int x) { wait_time = x; }
    void count_down() { if( wait_time > 0 ) wait_time--; }

};

// Classes
class empty : public map_object {
public : 

    empty() { icon = ' '; fixed= false; type = EMPTY; }

};

class wall : public map_object {
public : 

    wall() { icon = '#'; fixed= true; type = WALL; }

    wall(char c) { icon = c; fixed= true; type = WALL; }

};

class weak : public map_object {
    
    weak() { icon = '#'; fixed= false; type = WEAK; }

}; 

class enemy : public map_object, public wait_object {
public : 

    enemy() { icon = '+'; fixed = false; type = ENEMY; wait_time = 0; }

    int logic_machine() {
        std::random_device rd;
        std::default_random_engine r_eng(rd());
        return r_eng() % 5;
    }

};

class shell : public map_object, public wait_object {
protected : 

    int own;
    int ori;
    int life_time;
        
public : 

    shell(int _ori, int _own) { fixed = false; type = SHELL; own = _own; ori = _ori; life_time = 0; wait_time = 0;
        switch (ori) {
            case UP : if(own == PLAYER) set_icon('|'); else set_icon('.'); break;
            case LEFT : if(own == PLAYER) set_icon('-'); else set_icon('.'); break;
            case DOWN : if(own == PLAYER) set_icon('|'); else set_icon('.'); break;
            case RIGHT : if(own == PLAYER) set_icon('-'); else set_icon('.'); break;
            default : break;
        }
    }

    int get_ori() { return ori; }
    int owner() { return own; }
    int life() { return life_time; }

};

class player : public map_object, public wait_object {
protected : 

    int life;
    int score;
    int ori;

public : 

    player() { life = 10; score = 0; ori = UP; fixed = false; type = PLAYER; icon = '^'; wait_time = 0; }

    int get_life() { return life; }
    int get_score() { return score; }
    int get_ori() { return ori; }

    void damage(int _damage) { life -= _damage; }
    void score_up(int _score) { score += _score; }
    void orient(int _ori) {
        ori = _ori;
        switch (ori) {
            case UP : set_icon('^'); break;
            case LEFT : set_icon('<'); break;
            case DOWN : set_icon('v'); break;
            case RIGHT : set_icon('>'); break;
            default : break;
        }
    }

    int action(std::chrono::steady_clock::time_point end) {
        int ch = 0;
        bool captured = false;
        while (std::chrono::steady_clock::now() < end && captured == false) {
            if(_kbhit()) ch = _getch();
            if(ch == 119 || ch == 97 || ch == 115 || ch == 100 || ch == 106) captured = true;
        }
        while (std::chrono::steady_clock::now() < end) Sleep(1);
        if(ch == 119) return UP;
        else if(ch == 97) return LEFT;
        else if(ch == 115) return DOWN;
        else if(ch == 100) return RIGHT;
        else if(ch == 106) return ATTACK;
        else return -1;
    }

};

// Map
class gmap {
public : 
    // Parameters
    int difficulty;

    // Coordinates
    int player_x;
    int player_y;
    std::vector<int> enemy_x;
    std::vector<int> enemy_y;
    std::vector<int> shell_x;
    std::vector<int> shell_y;

    // Sprite & Nodes
    player myself = player();
    std::vector<enemy> enemys;
    std::vector<shell> shells;
    std::vector<std::vector<map_object>> mmap;

    // Constructor
    gmap(std::string map) {
        std::string str;
	    std::string word;
	    std::ifstream file;
        file.open(map, std::ios::in);
        if (!file.is_open())
    	{
        	std::cout << "Cannot open the map." << std::endl;
    	}
        getline(file, str); int m = std::stoi(str);
        getline(file, str); int n = std::stoi(str);
        std::vector<std::vector<map_object>> tmap(m + 2, std::vector<map_object>(n + 2));
        mmap = tmap;
        init_wall();
        getline(file, str); int px = std::stoi(str);
        getline(file, str); int py = std::stoi(str);
        init_player(px, py);
        getline(file, str); int enemy_num = std::stoi(str);
        while(enemy_num > 0) {
            getline(file, str); int ex = std::stoi(str);
            getline(file, str); int ey = std::stoi(str);
            init_enemy(ex, ey);
            enemy_num--;
        }
        getline(file, str); int wall_num = std::stoi(str);
        while(wall_num > 0) {
            getline(file, str); int wx = std::stoi(str);
            getline(file, str); int wy = std::stoi(str);
            getline(file, str); char c = str[0];
            init_special_wall(wx, wy, c);
            wall_num--;
        }
    }

    // Init sprite & nodes
    void init_player(int x, int y) {
        mmap[x][y] = myself;
        player_x = x;
        player_y = y;
    }

    void init_enemy(int x, int y){
        enemys.push_back(enemy());
        mmap[x][y] = enemys[enemys.size() - 1];
        enemy_x.push_back(x);
        enemy_y.push_back(y);
    }

    void init_special_wall(int x, int y, char c) {
        mmap[x][y] = wall(c);
    }

    void init_wall() {
        for(int i = 0; i < mmap.size(); i++) {
            for(int j = 0; j < mmap[0].size(); j++) {
                if(i == 0 || j == 0 || i == mmap.size() - 1 || j == mmap[0].size() - 1) mmap[i][j] = wall();
                else mmap[i][j] = empty();
            }
        }
    }

    // Print information
    void to_string() {
        CONSOLE_CURSOR_INFO cursor;
	    cursor.bVisible = 0;
	    cursor.dwSize = 1;
	    SetConsoleCursorInfo(GetStdHandle(-11), &cursor);
        for(int i = 0; i < mmap.size(); i++) {
            for(int j = 0; j < mmap[0].size(); j++) {
                if(i == player_x && j == player_y) std::cout<<myself.to_char()<<" ";
                else std::cout<<mmap[i][j].to_char()<<" ";
            }
            std::cout<<"\n\r";
        }
        std::cout<<"life = "<<myself.get_life()<<" score = "<<myself.get_score()<<"\n\r";
    }

    // Move, spawn & touch
    void move(int x, int y, int _type, int id, int _logic) {
        switch (_logic) {
            case UP : touch(x, y, x - 1, y, id); break;
            case LEFT : touch(x, y, x, y - 1, id); break;
            case DOWN : touch(x, y, x + 1, y, id); break;
            case RIGHT : touch(x, y, x, y + 1, id); break;
            default : break;
        }
        if(_type == ENEMY) enemys[id].count_up(ENEMY_WAIT_ROUNDS);
        else if(_type == SHELL) shells[id].count_up(SHELL_WAIT_ROUNDS);
    }

    void spawn(int x, int y, int _type, int _ori = UP, int _own = EMPTY) {
        if(mmap[x][y].cant_move() != true) {
            if(_type == SHELL) {
                if(mmap[x][y].type == EMPTY) {
                    shells.push_back(shell(_ori, _own));
                    mmap[x][y] = shells[shells.size() - 1];
                    shell_x.push_back(x);
                    shell_y.push_back(y);
                } 
                else if(mmap[x][y].type == PLAYER && _own == ENEMY) {
                    myself.damage(1);
                }
                else if(mmap[x][y].type == ENEMY && _own == PLAYER) {
                    int id = -1;
                    for(int i = 0; i < enemys.size(); i++) if(enemy_x[i] == x && enemy_y[i] == y) id = i;
                    vectorDel<enemy>(enemys, id);
                    vectorDel<int>(enemy_x, id);
                    vectorDel<int>(enemy_y, id);
                    mmap[x][y] = empty();
                    myself.score_up(1);
                }
                else if(mmap[x][y].type == SHELL) {
                    int id = -1;
                    for(int i = 0; i < shells.size(); i++) if(shell_x[i] == x && shell_y[i] == y) id = i;
                    vectorDel<shell>(shells, id);
                    vectorDel<int>(shell_x, id);
                    vectorDel<int>(shell_y, id);
                    mmap[x][y] = empty();
                }
            }
        } 
    }

    void touch(int ax, int ay, int bx, int by, int id = -1) {
        if(mmap[ax][ay].cant_move() == false) {
            if(mmap[ax][ay].type == PLAYER && mmap[bx][by].type == EMPTY) {
                mmap[bx][by] = mmap[ax][ay];
                mmap[ax][ay] = empty();
                player_x = bx;
                player_y = by;
            }
            else if(mmap[ax][ay].type == PLAYER && mmap[bx][by].type == SHELL) {
                int bid = -1;
                for(int i = 0; i < shells.size(); i++) if(shell_x[i] == bx && shell_y[i] == by) bid = i;
                if(shells[bid].owner() == ENEMY) myself.damage(1);
                objectClean<shell>(shells, shell_x, shell_y, bid);
                mmap[bx][by] = mmap[ax][ay];
                mmap[ax][ay] = empty();
                player_x = bx;
                player_y = by;
            }
            else if(mmap[ax][ay].type == ENEMY && mmap[bx][by].type == EMPTY) {
                mmap[bx][by] = mmap[ax][ay];
                mmap[ax][ay] = empty();
                enemy_x[id] = bx;
                enemy_y[id] = by;
            }
            else if(mmap[ax][ay].type == SHELL && mmap[bx][by].type == EMPTY) {
                mmap[bx][by] = mmap[ax][ay];
                mmap[ax][ay] = empty();
                shell_x[id] = bx;
                shell_y[id] = by;
            }
            else if(mmap[ax][ay].type == SHELL && mmap[bx][by].type == PLAYER) {
                if(shells[id].owner() == ENEMY) myself.damage(1);
                objectClean<shell>(shells, shell_x, shell_y, id);
                mmap[ax][ay] = empty();
            }
            else if(mmap[ax][ay].type == SHELL && mmap[bx][by].type == WALL) {
                objectClean<shell>(shells, shell_x, shell_y, id);
                mmap[ax][ay] = empty();
            }
            else if(mmap[ax][ay].type == SHELL && mmap[bx][by].type == ENEMY) {
                if(shells[id].owner() == PLAYER) {
                    myself.score_up(1);
                    int bid = -1;
                    for(int i = 0; i < enemys.size(); i++) {
                        if(enemy_x[i] == bx && enemy_y[i] == by) bid = i;
                    }
                    objectClean<enemy>(enemys, enemy_x, enemy_y, bid);
                    mmap[bx][by] = empty();
                }
                objectClean<shell>(shells, shell_x, shell_y, id);
                mmap[ax][ay] = empty();
            }
            else if(mmap[ax][ay].type == SHELL && mmap[bx][by].type == SHELL) {
                objectClean<shell>(shells, shell_x, shell_y, id);
                mmap[ax][ay] = empty();
                int bid = -1;
                for(int i = 0; i < shells.size(); i++) {
                    if(shell_x[i] == bx && shell_y[i] == by) bid = i;
                }
                objectClean<shell>(shells, shell_x, shell_y, bid);
                mmap[bx][by] = empty();
            }
        }
    }

    // Run game
    void run_game() {
        while(enemys.size() > 0) {

            // Set the Time of a Round
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now() + std::chrono::milliseconds(ROUND_TIME);

            // Shells' turn
            for(int j = 0; j < shells.size(); j++) {
                if(shells[j].ready()) move(shell_x[j], shell_y[j], SHELL, j, shells[j].get_ori());
            }

            // Enemies' turn
            for(int j = 0; j < enemys.size(); j++) {
                if(enemys[j].ready()) {
                    std::random_device rd;
                    std::default_random_engine r_eng(rd());
                    int action_index = r_eng() % 3;
                    if(action_index == 0 || action_index == 1) move(enemy_x[j], enemy_y[j], ENEMY, j, enemys[j].logic_machine());
                    else {
                        int dx = player_x - enemy_x[j];
                        int dy = player_y - enemy_y[j];
                        if(dx <= 0 && abs(dx) >= abs(dy)) spawn(enemy_x[j] - 1, enemy_y[j], SHELL, UP, ENEMY);
                        else if(dy <= 0 && abs(dy) >= abs(dx)) spawn(enemy_x[j], enemy_y[j] - 1, SHELL, LEFT, ENEMY);
                        else if(dx >= 0 && abs(dx) >= abs(dy)) spawn(enemy_x[j] + 1, enemy_y[j], SHELL, DOWN, ENEMY);
                        else if(dy >= 0 && abs(dy) >= abs(dx)) spawn(enemy_x[j], enemy_y[j] + 1, SHELL, RIGHT, ENEMY);
                        enemys[j].count_up(ENEMY_WAIT_ROUNDS);
                    }
                }
            }

            // Player's turn
            bool actioned = false;
            while (std::chrono::steady_clock::now() < end && myself.ready()) {
                if(actioned == false) {
                    int action_type = myself.action(end);
                    if(action_type >= UP && action_type <= RIGHT) {
                        myself.orient(action_type);
                        switch (action_type) {
                            case 0 : touch(player_x, player_y, player_x - 1, player_y); break;
                            case 1 : touch(player_x, player_y, player_x, player_y - 1); break;
                            case 2 : touch(player_x, player_y, player_x + 1, player_y); break;
                            case 3 : touch(player_x, player_y, player_x, player_y + 1); break;
                            default : break;
                        }
                        myself.count_up(10);
                        actioned = true;
                    }
                    else if(action_type == ATTACK) {
                        switch (myself.get_ori()) {
                            case 0 : spawn(player_x - 1, player_y, SHELL, UP, PLAYER); break;
                            case 1 : spawn(player_x, player_y - 1, SHELL, LEFT, PLAYER); break;
                            case 2 : spawn(player_x + 1, player_y, SHELL, DOWN, PLAYER); break;
                            case 3 : spawn(player_x, player_y + 1, SHELL, RIGHT, PLAYER); break;
                            default : break;
                        }
                        myself.count_up(PLAYER_WAIT_ROUNDS);
                        actioned = true;
                    }
                }
            }

            // Print Screen
            to_string();

            // Count Downs
            myself.count_down();
            for(int i = 0; i < enemys.size(); i++) enemys[i].count_down();
            for(int i = 0; i < shells.size(); i++) shells[i].count_down();

            // Clean Screen
            if(myself.get_life() == 0) break;
            HANDLE hOutput;
            COORD coord={0,0};
            hOutput=GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleCursorPosition(hOutput, coord);
        }

        // Win or Lose
        if(myself.get_life() == 0) std::cout<<"Game Over, your score is "<<myself.get_score()<<" !"<<std::endl;
        else std::cout<<"You win, your score is "<<myself.get_score()<<" !"<<std::endl;
    }

};

int main(){
    gmap game = gmap("map.txt");
    game.run_game();
    system("pause");
    return 0;
}

// w = 119, a = 97, s = 115, d = 100, j = 106, k = 107, l = 108
