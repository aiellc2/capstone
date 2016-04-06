#include "opencv2/opencv.hpp"
#include <opencv2/gpu/gpu.hpp>
#include <pthread.h>
#include <semaphore.h>
#include <queue>
#include <cmath>
#include <stdlib.h>
#include <stdio.h>


#define GLOBAL_COLS 640
#define GLOBAL_ROWS 360

#define SCREEN_RES_HOR 1440
#define SCREEN_RES_VER 900

#define res_mapx(X) ((X * SCREEN_RES_HOR / GLOBAL_COLS)) //map 640x360 to screen resolution
#define res_mapy(X) (X * SCREEN_RES_VER / GLOBAL_ROWS)




using namespace cv;

std::queue <Mat> capBuffer;
int bufIndex = 0;
int procIndex = 0;
Mat tempCap;
pthread_mutex_t mutex;
sem_t *s1;
sem_t *s2;

struct mouseMove{
  int x;
  int y;
};

struct mouseMove mouse;

VideoCapture capture(0); // open the default camera


void *mouseControl(void *threadid){
  int x_avg = 0;
  int y_avg = 0;
  int count = 0;
  while(1){
    //printf("mouse wait\n");
    sem_wait(s2); //wait for signal
    //printf("mouse move\n");
    //printf("mouse.x %d\n",mouse.x);
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

void *cap(void *threadid) {
    std::cout << "1this is cap\n";
    capture >> tempCap;
    while (1) {

        Mat frame;
        capture >> frame;
        resize(frame, frame, Size(GLOBAL_COLS, GLOBAL_ROWS), 0, 0, INTER_CUBIC);
        pthread_mutex_lock(&mutex);
        capBuffer.push(frame);
        pthread_mutex_unlock(&mutex);
        sem_post(s1);
        if (bufIndex == 250) {
            printf("buf getting full...\n");
        }

        usleep(100000); //increase delay if computer can not keep
        //usleep  (160000);
        //return 0;
    }
}


int main(int argc, char **argv) {
    // open the default camera, use something different from 0 otherwise;
    // Check VideoCapture documentation.
    pthread_mutex_init(&mutex, NULL);
    namedWindow("im1", 0);
    pthread_t thread[2];
    int rc;
    long t;
    sem_unlink("sem1");
    sem_unlink("sem2");

    s1 = sem_open("sem1", O_CREAT, 0777, 0);
    s2 = sem_open("sem2", O_CREAT, 0777, 0);


    rc = pthread_create(&thread[0], NULL, cap, (void *) t); //creates threads to run video capture
    rc = pthread_create(&thread[1], NULL, mouseControl, (void *) t); //creates thread to run mouseControl

    Mat frame;
    Mat im_HSV, im_HSV2;
    Mat imgThresholded, imgThresholded2;

    namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"
    namedWindow("Control2", CV_WINDOW_AUTOSIZE); //create a window called "Control"

    //namedWindow("HuePreview", CV_WINDOW_AUTOSIZE);
    //114 141 75 161 255 255 default red

    //36 86 101 88 255 255 default green

    int iLowH2 = 10;
    int iHighH2 = 88;

    int iLowS2 = 86; //greens
    int iHighS2 = 255;

    int iLowV2= 101;
    int iHighV2 = 255;

    int iLowH = 114;
    int iHighH = 151;

    int iLowS = 141;   //reds
    int iHighS = 255;

    int iLowV = 75;
    int iHighV = 255;

    int gestureSensitivity=(275-100);


    //Create trackbars in "Control" window
    cvCreateTrackbar("Min Hue", "Control", &iLowH, 179); //Hue (0 - 179)
    cvCreateTrackbar("Max Hue", "Control", &iHighH, 179);

    cvCreateTrackbar("Min Saturation", "Control", &iLowS, 255); //Saturation (0 - 255)
    cvCreateTrackbar("Max Saturation", "Control", &iHighS, 255);

    cvCreateTrackbar("Min Value", "Control", &iLowV, 255); //Value (0 - 255)
    cvCreateTrackbar("Max Value", "Control", &iHighV, 255);

    //Create trackbars in "Control" window
    cvCreateTrackbar("Min Hue", "Control2", &iLowH2, 179); //Hue (0 - 179)
    cvCreateTrackbar("Max Hue", "Control2", &iHighH2, 179);

    cvCreateTrackbar("Min Saturation", "Control2", &iLowS2, 255); //Saturation (0 - 255)
    cvCreateTrackbar("Max Saturation", "Control2", &iHighS2, 255);

    cvCreateTrackbar("Min Value", "Control2", &iLowV2, 255); //Value (0 - 255)
    cvCreateTrackbar("Max Value", "Control2", &iHighV2, 255);

    cvCreateTrackbar("Gesture Sensitivity", "Control", &gestureSensitivity, (500-100));

    int iLastX = -1;
    int iLastY = -1;

    int iLastX2 = -1;
    int iLastY2 = -1;

    double dM01;
    double dM10;
    double dArea;

    double dM012;
    double dM102;
    double dArea2;

    int shrink = 0 , grow = 0;
    int lastDist = 0;

    int r_dist = 0;
    int l_dist = 0;
    int blank_count=0;
    sem_wait(s1);
    Mat imgLines = Mat::zeros(GLOBAL_ROWS,GLOBAL_COLS, CV_8UC3 );
    Mat imgLines2 = Mat::zeros(GLOBAL_ROWS,GLOBAL_COLS, CV_8UC3 );
    for (;;) {
      sem_wait(s1);

      pthread_mutex_lock(&mutex);
      frame = capBuffer.front();
      capBuffer.pop();

      pthread_mutex_unlock(&mutex);

      cvtColor(frame,im_HSV,COLOR_RGB2HSV);
      im_HSV2 = im_HSV;

      inRange(im_HSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image
      inRange(im_HSV2, Scalar(iLowH2, iLowS2, iLowV2), Scalar(iHighH2, iHighS2, iHighV2), imgThresholded2); //Threshold the image



      // (filter small objects from the foreground)
      erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
      dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

       //(filter small holes in the foreground)
      dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
      erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

      erode(imgThresholded2, imgThresholded2, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
      dilate( imgThresholded2, imgThresholded2, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

       //(filter small holes in the foreground)
      dilate( imgThresholded2, imgThresholded2, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
      erode(imgThresholded2, imgThresholded2, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

      //get the position of detected shapes
      Moments oMoments = moments(imgThresholded);
      Moments oMoments2 = moments(imgThresholded2);


      dM01 = oMoments.m01;
      dM10 = oMoments.m10;
      dArea = oMoments.m00;

      dM012 = oMoments2.m01;
      dM102 = oMoments2.m10;
      dArea2 = oMoments2.m00;

      if (dArea > 10000 && dArea2 <1000) //make sure detected area is big enough
      {
       //calculate the position
       int posX = dM10 / dArea;
       int posY = dM01 / dArea;

       if (iLastX >= 0 && iLastY >= 0 && posX >= 0 && posY >= 0) //if something moved
       {

         line(imgLines, Point(posX, posY), Point(iLastX, iLastY), Scalar(0,0,255), 10);
        //Draw a red line from the previous point to the current point

        if(iLastX > posX){
          l_dist = 0;

          r_dist += (iLastX - posX);
          if(r_dist >= (gestureSensitivity+100)){
            printf("dArea: %f RIGHT DETECTED!!! - %d\n",dArea, r_dist);
            //osascript -e 'activate application "iBooks"'

            //right osascript -e 'tell application "System Events" to keystroke 124'
            system("osascript -e 'activate application \"iBooks\"'");
            usleep(10000);
            system("osascript -e 'tell application \"System Events\"' -e 'key code 124' -e 'end'");
            imgLines = Mat::zeros(360,640, CV_8UC3 );

            //printf("iLowH: %d iLowS: %d iLowV: %d",iLowH, iLowS, iLowV);
            r_dist = 0;
          }
        }

        else if(iLastX < posX){
          //printf("L_dist: %d\n",l_dist);
          r_dist = 0;

          l_dist += (posX - iLastX);
          if(l_dist >= (gestureSensitivity+100)){
            printf ("LEFT DETECTED!!! - %d\n",l_dist);
            system("osascript -e 'activate application \"iBooks\"'");
            usleep(10000);
            system("osascript -e 'tell application \"System Events\"' -e 'key code 123' -e 'end'");
            imgLines = Mat::zeros(GLOBAL_ROWS,GLOBAL_COLS, CV_8UC3 );
            l_dist = 0;
          }
        }

       }

        iLastX = posX;
        iLastY = posY;
      }

      else if (dArea > 10000 && dArea2 > 10000){ //both colours detected
        //printf("both...\n");
        //printf("%d %d %d %d %d %d \n %d %d %d %d %d %d",iLowH, iLowS, iLowV, iHighH, iHighS, iHighV, iLowH2, iLowS2, iLowV2, iHighH2, iHighS2, iHighV2);


        int posX = dM10 / dArea;
        int posY = dM01 / dArea;

        int posX2 = dM102 / dArea2;
        int posY2 = dM012 / dArea2;

        //printf("shrink: %d grow: %d dist: %d\n",shrink, grow, abs(posX - posX2));




        if(abs(posX - posX2) < lastDist){
            shrink++;
            grow=0;
            lastDist = abs(posX - posX2);
            if(shrink >= 5){
              shrink = 0;
              printf("SHRINK\n");
              system("osascript -e 'activate application \"iBooks\"'");
              usleep(10000);
              system("osascript -e 'tell application \"System Events\"' -e 'key code 27 using {shift down, command down}' -e 'end'");
            }
        }
        else if(abs(posX - posX2) > lastDist){
            grow++;
            shrink=0;
            lastDist = abs(posX - posX2);
            if(grow >= 6){
              grow = 0;
              printf("GROW\n");
              system("osascript -e 'activate application \"iBooks\"'");
              usleep(10000);
              system("osascript -e 'tell application \"System Events\"' -e 'key code 24 using {shift down, command down}' -e 'end'");
            }

        }


        iLastX = posX;
        iLastY = posY;

        iLastX2 = posX2;
        iLastY2 = posY2;

      }

      else if(dArea2 > 10000 && dArea <1000){ //if only green
        //mouse Control
        //printf("Mouse!\n");

        int posX2 = dM102 / dArea2;
        int posY2 = dM012 / dArea2;

        //iLastX2 = posX2;
        //iLastY2 = posY2;

        //printf("posx %d\n",posX2);
        posX2 = posX2 > (GLOBAL_COLS - 15) ? posX2+7 : posX2;
        posX2 = posX2 < (GLOBAL_COLS - (GLOBAL_COLS-15)) ? posX2-7 : posX2;

        posY2 = posY2 > (GLOBAL_ROWS - 15) ? posY2+7 : posY2;
        posY2 = posY2 < (GLOBAL_ROWS - (GLOBAL_ROWS-15)) ? posY2-7 : posY2;

        mouse.x = res_mapx((GLOBAL_COLS - posX2));
        mouse.y = res_mapy(posY2);

        sem_post(s2); //signal mousecontrol to proceed

      }

      else{
        if(++blank_count==10){ //reset counter when object out of frame

          r_dist=0;
          l_dist=0;

          blank_count=0;

          iLastX=-1;
          iLastY=-1;

          imgLines = Mat::zeros(GLOBAL_ROWS,GLOBAL_COLS, CV_8UC3 );
        }
      }
      //add red line to original image to trace gesture
      frame = frame + imgLines;

      //show windows on screen
 			if(!frame.empty() ){
                imshow("im1", imgThresholded);
                imshow("im2", imgThresholded2);
                imshow("original", frame);
                //imshow("HuePreview", diagBGR);
                waitKey(1);
 			 }
    }
    return 0;
}
