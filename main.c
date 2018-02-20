#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <windows.h>
#include <process.h>
#include <stdbool.h>

#define NOC 11 //numbers of cities
#define DHPC 5  //Default horses per city
#define NOH 12 //number of horsemen
#define NONE (-10)
#define USING (-100)
#define set_cursor(x, y) printf("\033[%d;%dH", (x), (y))
#define clear() printf("\033[H\033[J")

HANDLE mutex;
/*typedef struct thread_args {
int current_horse;
} thread_args;*/
struct cities {
    int city_id;
    bool is_city;
    double x;
    double y;
    HANDLE mutex;
} city[NOC];

double weight_matrix[NOC][NOC];
double history_matrix[NOC][NOC];

void cities_set_xy(int city_id, double x, double y) {
    city[city_id].x = x;
    city[city_id].y = y;
}

void cities_initialize() {
    for (int i = 0; i < NOC; ++i) {
        city[i].city_id = i;
        city[i].mutex = CreateMutex(NULL, FALSE, NULL);
    }
    cities_set_xy(0, 3, 0);
    cities_set_xy(1, 11, 0);
    cities_set_xy(2, 6, 3);
    cities_set_xy(3, 13, 3);
    cities_set_xy(4, 4, 5);
    cities_set_xy(5, 0, 3);
    cities_set_xy(6, 9, 6);
    cities_set_xy(7, 0, 8);
    cities_set_xy(8, 4, 9);
    cities_set_xy(9, 9, 9);
    cities_set_xy(10, 14, 7);
}

double adjacency_matrix[NOC][NOC] = {
        0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0,
        1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0,
        0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1,
        0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0,
        1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0,
        0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1,
        0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0,
        0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0,
        0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1,
        0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0
};

void create_weight_matrix() {
    for (int i = 0; i < NOC; ++i) {
        for (int j = 0; j < NOC; ++j) {
            if (adjacency_matrix[i][j] == 1) {
                weight_matrix[i][j] = sqrt(pow(city[i].x - city[j].x, 2)
                                           + pow(city[i].y - city[j].y, 2));
            }
            else {
                weight_matrix[i][j] = INT_MAX;
            }
        }
    }
}

void create_history_matrix() {
    for (int i = 0; i < NOC; ++i) {
        for (int j = 0; j < NOC; ++j) {
            if (adjacency_matrix[i][j] == 1) {
                history_matrix[i][j] = j;
            }
            else {
                history_matrix[i][j] = -1;
            }
        }
    }
}

void calculate_history_matrix() {
    for (int i = 0; i < NOC; ++i) {
        for (int j = 0; j < NOC; ++j) {
            if (weight_matrix[i][j] != INT_MAX) {
                for (int k = 0; k < NOC; ++k) {
                    if (weight_matrix[i][k] >(weight_matrix[i][j] + weight_matrix[j][k])) {
                        weight_matrix[i][k] = weight_matrix[i][j] + weight_matrix[j][k];
                        history_matrix[i][k] = history_matrix[i][j];
                    }
                }
            }
        }
    }
}

int map_of_horses[NOC][DHPC * NOC];
struct horsemen {
    int horseman_id;
    int destination_city;
    int departure_city;
    int current_city;
    int current_horse;
    int pos_x;
    int pos_y;
    //pthread_t horseman_thread;
    char horseman_name[20];
    bool is_free;
    int pos;
    HANDLE horseman_handle;

} horseman[NOC];

char first_names[10][10] = { "John ", "Richard ", "Corey ", "Mark ", "Adam ", "Jamie ", "Sean ", "Juffin ", "Max ",
                             "Linus " };
char last_names[10][10] = { "Carmack", "Feynman", "Taylor", "Twain", "Savage", "Bean", "Hally", "Dawkins", "Kojima",
                            "Torvalds" };
struct horses {
    int horse_id;
    int in_city;
    bool tired;
    bool is_free;
    //pthread_t horse_thread;
    HANDLE horse_mutex;
} horse[NOC * DHPC];


unsigned __stdcall horse_resting(void *args) {
    int *ip = (int *)args;
    int resting_horse = *ip;
    Sleep(1000);
    WaitForSingleObject(horse[resting_horse].horse_mutex, INFINITE);
    horse[resting_horse].tired = false;
    ReleaseMutex(horse[resting_horse].horse_mutex);
}

void get_free_horse(int target_horseman) {
    while(true) {
        for (int j = 0; j < NOC * DHPC; ++j) {
            if (map_of_horses[horseman[target_horseman].current_city][j] != -666) {
                if (horse[j].tired == false) {
                    if (horse[j].is_free) {

                        horse[horseman[target_horseman].current_horse].is_free = false;
                        horseman[target_horseman].current_horse = map_of_horses[horseman[target_horseman].current_city][j];
                        map_of_horses[horseman[target_horseman].current_city][j] = -666;

                        int c_horse = horseman[target_horseman].current_horse;
                        bool b = horse[c_horse].is_free;

                        if(c_horse != NONE){
                            return;
                        }

                    }
                }
            }
        }
        Sleep(20);
    }
}

void live_horse(int target_horseman, int target_city) {
    horse[horseman[target_horseman].current_horse].in_city = target_city;
    map_of_horses[horseman[target_horseman].current_city][horseman[target_horseman].current_horse] = horseman[target_horseman].current_horse;
    horse[horseman[target_horseman].current_horse].is_free = true;
    horse[horseman[target_horseman].current_horse].tired = true;
    _beginthreadex(NULL, 0, horse_resting,
                   &horseman[target_horseman].current_horse, 0, NULL);
    horseman[target_horseman].current_horse = NONE;
}

#define K 2
HANDLE handle_output;

void way_between_cities(int city1, int city2, int target_horseman){
    COORD city_1_coord;
    COORD city_2_coord;
    COORD point_coord;
    city_1_coord.X = (short)(city[city1].x * K);
    city_1_coord.Y = (short)(city[city1].y * K);
    city_2_coord.X = (short)(city[city2].x * K);
    city_2_coord.Y = (short)(city[city2].y * K);


    double delta_x = abs(city_2_coord.X - city_1_coord.X);
    double delta_y = abs(city_2_coord.Y - city_1_coord.Y);
    double error = 0;
    double delta_error = delta_y;
    point_coord.Y = city_1_coord.Y;
    int dir_y = city_2_coord.Y - city_1_coord.Y;
    if(dir_y > 0){
        dir_y = 1;
    }
    if(dir_y < 0){
        dir_y = -1;
    }

    for(point_coord.X = (short)(city_1_coord.X + 1); point_coord.X < city_2_coord.X; point_coord.X++){
        horseman[target_horseman].pos_x = point_coord.X;
        horseman[target_horseman].pos_y = point_coord.Y;
        error = error + delta_error;

        if(2 * error >= delta_x){
            point_coord.Y = (short)(point_coord.Y + dir_y);
            error = error - delta_x;


        }
        Sleep(1000);
    }
    ReleaseMutex(horseman[target_horseman].horseman_handle);
}


unsigned __stdcall horseman_way(void *arg) {
    int *ip = (int *)arg;
    int target_horseman = *ip;
    // когда находится в стартовом городе, пытается взять свободную и неусталую лошадь
    // до тех пор пока не появиться такая лошадь

    horseman[target_horseman].current_city = horseman[target_horseman].departure_city;


    get_free_horse(horseman[target_horseman].horseman_id);

    // в пути
    while (true) {
        // попадает в следующий город
        int last_city = horseman[target_horseman].current_city;
        horseman[target_horseman].current_city = (int)history_matrix[horseman[target_horseman].current_city]
        [horseman[target_horseman].destination_city];
        way_between_cities(last_city, horseman[target_horseman].current_city,  horseman[target_horseman].horseman_id);
        // если это его конечный пункт
        if (horseman[target_horseman].current_city == horseman[target_horseman].destination_city) {
            // оставил усталую лошадь

            live_horse(target_horseman, horseman[target_horseman].current_city);


            horseman[target_horseman].is_free = true;
            return 0;
            // если это еще не его конечный пункт
        }
        else if (horseman[target_horseman].current_city != horseman[target_horseman].destination_city) {
            //оставил усталую лошадь

            live_horse(target_horseman, horseman[target_horseman].current_city);
            get_free_horse(target_horseman);

        }
        else break;
    }
}

void horses_initialize() {
    for (int i = 0; i < NOC * DHPC; ++i) {
        horse[i].horse_id = i;
        horse[i].tired = false;
        horse[i].is_free = true;
    }
    for (int m = 0; m < NOC; ++m) {
        for (int i = 0; i < NOC * DHPC; ++i) {
            map_of_horses[m][i] = -666;
            horse[i].horse_mutex = CreateMutex(NULL, FALSE, NULL);
        }
    }
    int l = 0;
    for (int j = 0; j < NOC; ++j) {
        for (int k = 0; k < DHPC; ++k) {
            horse[l].in_city = j;
            map_of_horses[j][k] = l;
            l++;
        }
    }
}

void draw_road(int city1, int city2){
    COORD city_1_coord;
    COORD city_2_coord;
    COORD point_coord;
    city_1_coord.X = (short)(city[city1].x * K);
    city_1_coord.Y = (short)(city[city1].y * K);
    city_2_coord.X = (short)(city[city2].x * K);
    city_2_coord.Y = (short)(city[city2].y * K);


    double delta_x = abs(city_2_coord.X - city_1_coord.X);
    double delta_y = abs(city_2_coord.Y - city_1_coord.Y);
    double error = 0;
    double delta_error = delta_y;
    point_coord.Y = city_1_coord.Y;
    int dir_y = city_2_coord.Y - city_1_coord.Y;
    if(dir_y > 0){
        dir_y = 1;
    }
    if(dir_y < 0){
        dir_y = -1;
    }

    for(point_coord.X = (short)(city_1_coord.X + 1); point_coord.X < city_2_coord.X; point_coord.X++){
        point_coord.X += 50;
        SetConsoleCursorPosition(handle_output, point_coord);
        printf(".");
        point_coord.X -= 50;
        error = error + delta_error;

        if(2 * error >= delta_x){
            point_coord.Y = (short)(point_coord.Y + dir_y);
            error = error - delta_x;


        }
    }
}

void draw_horsemen(){
    COORD point_coord;
    for (int i = 0; i < NOH; ++i) {
        if(!horseman[i].is_free){
            point_coord.X = (short)horseman[i].pos_x;
            point_coord.Y = (short)horseman[i].pos_y;

            point_coord.X += 50;
            SetConsoleCursorPosition(handle_output, point_coord);
            printf("H%d", horseman[i].horseman_id);
        }
    }
}
void draw_map(){
    handle_output = GetStdHandle(STD_OUTPUT_HANDLE);
    for (int i = 0; i < NOC; ++i) {
        COORD city_coord;
        city_coord.X = (short)(city[i].x * K);
        city_coord.Y = (short)(city[i].y * K);

        city_coord.X += 50;
        SetConsoleCursorPosition(handle_output, city_coord);
        printf("%d", i);

        city_coord.Y++;
        SetConsoleCursorPosition(handle_output, city_coord);

        int num = 0;
        int non_tired= 0;
        int free = 0;
        for (int j = 0; j < NOC * DHPC; ++j) {
            if(map_of_horses[i][j] != -666){
                num++;
                if(!horse[map_of_horses[i][j]].tired){
                    non_tired++;
                    if(horse[map_of_horses[i][j]].is_free){
                        free++;
                    }


                }

            }
        }

        printf("(%d, %d, % d)", num, non_tired, free);
    }
    for (int i = 0; i < NOC; ++i) {
        for (int j = 0; j < NOC; ++j) {
            if (adjacency_matrix[i][j] == 1) {
               draw_road(city[i].city_id, city[j].city_id);
            }
        }
    }
}

void draw_info(){
    COORD info_coord;
    info_coord.X = 0;
    info_coord.Y = 0;
    for (int i = 0; i < NOH; ++i) {
        if(!horseman[i].is_free){
            SetConsoleCursorPosition(handle_output, info_coord);
            if(horseman[i].current_city == horseman[i].destination_city) {
                printf("%s (H%d): city - %d, Complete", horseman[i].horseman_name, horseman[i].horseman_id, horseman[i].current_city);
            }else{
                if(horseman[i].current_horse != NONE){
                    printf("%s (H%d): city - %d, horse - %d", horseman[i].horseman_name, horseman[i].horseman_id, horseman[i].current_city, horseman[i].current_horse);
                }else{
                    printf("%s (H%d): city - %d, horse - %s", horseman[i].horseman_name, horseman[i].horseman_id, horseman[i].current_city, "waiting");
                }
            }
            info_coord.Y++;
        }
    }
}

unsigned __stdcall drawing(void *arg){
    while (true){
        system("cls");
        draw_map();
        draw_horsemen();
        draw_info();
        Sleep(1000);
    }
}

void create_horseman(int target_horseman) {

    srand((unsigned int)time(NULL));
    char name[21];
    strcpy(name, first_names[rand() % 10]);
    strcat(name, last_names[rand() % 10]);
    strcpy(horseman[target_horseman].horseman_name, name);
    horseman[target_horseman].is_free = false;
    horseman[target_horseman].horseman_id = target_horseman;
    horseman[target_horseman].departure_city = rand() % NOC;
    horseman[target_horseman].destination_city = rand() % NOC;
    while (true) {
        if (horseman[target_horseman].destination_city != horseman[target_horseman].departure_city) {
            break;
        }
        horseman[target_horseman].destination_city = rand() % NOC;
    }
    _beginthreadex(NULL, 0, horseman_way, &horseman[target_horseman].horseman_id, 0, NULL);
    //_beginthreadex(NULL, 0, show_info, &horseman[target_horseman].horseman_id, 0, NULL);
}


void horsemen_initialize() {
    for (int i = 0; i < NOH; ++i) {
        horseman[i].is_free = true;
        horseman[i].horseman_handle = CreateMutex(NULL, FALSE, NULL);
    }
    int num = 0;
    while (true) {
            for (int i = 0; i < NOH; i++) {
                if (horseman[i].is_free == true) {
                    create_horseman(i);
                    break;
                }
            }
            num++;
            Sleep(1000);
    }
}


int main() {


    cities_initialize();
    create_weight_matrix();
    create_history_matrix();
    calculate_history_matrix();
    horses_initialize();

    _beginthreadex(NULL, 0, drawing, NULL, 0, NULL);

    horsemen_initialize();
    //draw_map();


    getchar();

}
