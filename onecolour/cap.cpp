#include "opencv2/opencv.hpp"
//#include <opencv2/gpu/gpu.hpp>
#include <pthread.h>
#include <semaphore.h>
#include <queue>

#define GLOBAL_COLS 640
#define GLOBAL_ROWS 360


using namespace cv;

//Mat capBuffer[256];
std::queue <Mat> capBuffer;
int bufIndex = 0;
int procIndex = 0;
Mat tempCap;
pthread_mutex_t mutex;
sem_t *s1;

VideoCapture capture(0); // open the default camera


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

        usleep(80000); //increase delay if computer can not keep
        //usleep  (160000);
        //return 0;
    }
}


int main(int argc, char **argv) {
    // open the default camera, use something different from 0 otherwise;
    // Check VideoCapture documentation.
    pthread_mutex_init(&mutex, NULL);
    namedWindow("im1", 0);
    pthread_t thread[1];
    int rc;
    long t;
    sem_unlink("sem1");
    s1 = sem_open("sem1", O_CREAT, 0777, 0);

    rc = pthread_create(&thread[0], NULL, cap, (void *) t); //creates threads to run sort()
    Mat frame, frame2;
    Mat im_gray, im_gray2;
    Mat im_bw, im_bw2;
    Mat im_sub, im_sub2;
    Mat im_filt;
    Mat im_HSV;
    Mat imgThresholded;

    Mat diagH = Mat::zeros(200,200, CV_8UC3 );;
    Mat diagBGR;
  //  Mat diagS = Mat::zeros(150,150, CV_8UC1 );;
  //  Mat diagV = Mat::zeros(150,150, CV_8UC1 );;


    namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"
    namedWindow("HuePreview", CV_WINDOW_AUTOSIZE);

    int iLowH = 122;
    int iHighH = 179;

    int iLowS = 86;
    int iHighS = 255;

    int iLowV = 101;
    int iHighV = 255;

    int gestureSensitivity=(275-100);


    //Create trackbars in "Control" window
    cvCreateTrackbar("Min Hue", "Control", &iLowH, 179); //Hue (0 - 179)
    cvCreateTrackbar("Max Hue", "Control", &iHighH, 179);

    cvCreateTrackbar("Min Saturation", "Control", &iLowS, 255); //Saturation (0 - 255)
    cvCreateTrackbar("Max Saturation", "Control", &iHighS, 255);

    cvCreateTrackbar("Min Value", "Control", &iLowV, 255); //Value (0 - 255)
    cvCreateTrackbar("Max Value", "Control", &iHighV, 255);

    cvCreateTrackbar("Gesture Sensitivity", "Control", &gestureSensitivity, (500-100));

    int iLastX = -1;
    int iLastY = -1;

    double dM01;
    double dM10;
    double dArea;

    int r_dist = 0;
    int l_dist = 0;
    int blank_count=0;
    sem_wait(s1);
    Mat imgLines = Mat::zeros(GLOBAL_ROWS,GLOBAL_COLS, CV_8UC3 );
    for (;;) {
      sem_wait(s1);

      pthread_mutex_lock(&mutex);
      frame = capBuffer.front();
      capBuffer.pop();

      pthread_mutex_unlock(&mutex);

      diagH.setTo(Scalar(iLowH,255,255));
      cvtColor(diagH,diagBGR, COLOR_HSV2RGB);


      cvtColor(frame,im_HSV,COLOR_RGB2HSV);


      inRange(im_HSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

      // gpu::GpuMat gpuErodeDilate;
      // gpuErodeDilate .upload(imgThresholded);
      //
      // // (filter small objects from the foreground)
      // gpu::erode(gpuErodeDilate, gpuErodeDilate, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
      // gpu::dilate( gpuErodeDilate, gpuErodeDilate, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
      //
      //  //(filter small holes in the foreground)
      // gpu::dilate( gpuErodeDilate, gpuErodeDilate, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
      // gpu::erode(gpuErodeDilate, gpuErodeDilate, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
      //
      // gpuErodeDilate.download(imgThresholded);

      // (filter small objects from the foreground)
      erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
      dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

       //(filter small holes in the foreground)
      dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
      erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

      //get the position of detected shapes
      Moments oMoments = moments(imgThresholded);

      dM01 = oMoments.m01;
      dM10 = oMoments.m10;
      dArea = oMoments.m00;

      if (dArea > 10000) //make sure detected area is big enough
      {
       //calculate the position of the object
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
      else{
        if(++blank_count==10){ //reset counter when object out of frame

          r_dist=0;
          l_dist=0;

          blank_count=0;

          iLastX=-1;
          iLastY=-1;

          imgLines = Mat::zeros(GLOBAL_ROWS,GLOBAL_COLS, CV_8UC3 );


          system("./speech"); //wait for speech command

        }

      }

      frame = frame + imgLines;

 			if(!frame.empty()){
        system("./speech"); //wait for speech command

                imshow("im1", imgThresholded);
                imshow("original", frame);
                imshow("HuePreview", diagBGR);
                waitKey(1);
 			 }
    }
    return 0;
}
