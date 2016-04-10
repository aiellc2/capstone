#include "header.hpp"

void *mouseControl(void *threadid){
  int x_avg = 0;
  int y_avg = 0;
  int count = 0;
  while(1){
    //printf("mouse wait\n");
    sem_wait(s2); //wait for signal
    //printf("mouse move\n");
    //printf("mouse.x %d\n",mouse.x);

    if(dArea3>10000){
      //do a left click
      // "cliclick kd:ctrl c:." for right click
      system("cliclick c:.");
    }


    x_avg += mouse.x;
    y_avg += mouse.y;

    count++;


    if (count == 2){

      count = 0;

      char cmd[64];
      sprintf(cmd, "cliclick m:%d,%d", x_avg/2, y_avg/2); //use average for smoothing
      system(cmd);

      x_avg = 0;
      y_avg = 0;
    }
  }

}
