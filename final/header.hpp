#include "opencv2/opencv.hpp"
#include <opencv2/gpu/gpu.hpp>
#include <pthread.h>
#include <semaphore.h>
#include <queue>
#include <signal.h>
#include <cmath>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>

#define GLOBAL_COLS 640
#define GLOBAL_ROWS 360

#define SCREEN_RES_HOR 1440
#define SCREEN_RES_VER 900

#define res_mapx(X) ((X * SCREEN_RES_HOR / GLOBAL_COLS)) //map 640x360 to screen resolution
#define res_mapy(X) ((X * SCREEN_RES_VER / GLOBAL_ROWS))

using namespace cv;

struct mouseMove{
  int x;
  int y;
};

extern int speechRunning;
extern double dArea3;
extern int params[18];
extern std::queue <Mat> capBuffer;
extern int bufIndex;
extern int procIndex;
extern Mat tempCap;
extern pthread_mutex_t mutex;
extern sem_t *s1;
extern sem_t *s2;
extern struct mouseMove mouse;

void exit_handler(int s);

void saveSettings(int a, int b, int c,\
                  int d, int e, int f,\
                  int g, int h, int i,\
                  int j, int k, int l,\
                  int m, int n, int o,\
                  int p, int q, int r);

int readSettings(int index);

void *mouseControl(void *threadid);
