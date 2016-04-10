#include "header.hpp"
using namespace cv;

int speechRunning = 0;
double dArea3 = 0;
int params[18];
std::queue <Mat> capBuffer;
int bufIndex = 0;
int procIndex = 0;
Mat tempCap;
pthread_mutex_t mutex;
sem_t *s1;
sem_t *s2;
struct mouseMove mouse;


VideoCapture capture(0); // open the default camera

void *cap(void *threadid) {
    std::cout << "\n\n\n\tA Handsfree E-Reading Apparatus\n\t\t Group L02\n\n\n";
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

        usleep(100000); //increase delay if computer can not keep up
        //usleep  (160000);
        //return 0;
    }
}


int main(int argc, char **argv) {
    // open the default camera, use something different from 0 otherwise;
    // Check VideoCapture documentation.
    pthread_mutex_init(&mutex, NULL);
    namedWindow("colours", WINDOW_AUTOSIZE);
    namedWindow("original", WINDOW_AUTOSIZE);

    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = exit_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    pthread_t thread[3];
    int rc;
    long t;
    sem_unlink("sem1");
    sem_unlink("sem2");

    s1 = sem_open("sem1", O_CREAT, 0777, 0);
    s2 = sem_open("sem2", O_CREAT, 0777, 0);

    rc = pthread_create(&thread[0], NULL, cap, (void *) t); //creates threads to run video capture
    rc = pthread_create(&thread[1], NULL, mouseControl, (void *) t); //creates thread to run mouseControl

    system("./speech&"); //call external app for voice recognition

    Mat frame;
    Mat im_HSV, im_HSV2, im_HSV3;

    Mat imgThresholded, imgThresholded2, imgThresholded3;

    namedWindow("RED", CV_WINDOW_AUTOSIZE); //create a window called "Control"
    namedWindow("GREEN", CV_WINDOW_AUTOSIZE); //create a window called "Control"
    namedWindow("BLUE", CV_WINDOW_AUTOSIZE); //create a window called "Control"


    readSettings(18);

    int iLowH3 = params[12];
    int iHighH3 = params[15];

    int iLowS3 = params[13];   //blues
    int iHighS3 = params[16];

    int iLowV3= params[14];
    int iHighV3 = params[17];



    int iLowH2 = params[6];
    int iHighH2 = params[9];

    int iLowS2 = params[7]; //greens
    int iHighS2 = params[10];

    int iLowV2= params[8];
    int iHighV2 = params[11];



    int iLowH = params[0];
    int iHighH = params[3];

    int iLowS = params[1];   //reds
    int iHighS = params[4];

    int iLowV = params[2];
    int iHighV = params[5];

    int gestureSensitivity=(275-100);
    int save = 0;
    int zoomSensitivity = 6;

    //Add trackbars to "Control" window
    cvCreateTrackbar("Min Hue", "RED", &iLowH, 179); //Hue (0 - 179)
    cvCreateTrackbar("Max Hue", "RED", &iHighH, 179);

    cvCreateTrackbar("Min Saturation", "RED", &iLowS, 255); //Saturation (0 - 255)
    cvCreateTrackbar("Max Saturation", "RED", &iHighS, 255);

    cvCreateTrackbar("Min Value", "RED", &iLowV, 255); //Value (0 - 255)
    cvCreateTrackbar("Max Value", "RED", &iHighV, 255);

    cvCreateTrackbar("Gesture Sensitivity", "RED", &gestureSensitivity, (500-100)); //add controls for sensitivity
    cvCreateTrackbar("Zoom Sensitivity", "RED", &zoomSensitivity, 14);

    cvCreateTrackbar("Save", "RED", &save, 1);

    //Create trackbars in "Control" window
    cvCreateTrackbar("Min Hue", "GREEN", &iLowH2, 179); //Hue (0 - 179)
    cvCreateTrackbar("Max Hue", "GREEN", &iHighH2, 179);

    cvCreateTrackbar("Min Saturation", "GREEN", &iLowS2, 255); //Saturation (0 - 255)
    cvCreateTrackbar("Max Saturation", "GREEN", &iHighS2, 255);

    cvCreateTrackbar("Min Value", "GREEN", &iLowV2, 255); //Value (0 - 255)
    cvCreateTrackbar("Max Value", "GREEN", &iHighV2, 255);

    //Create trackbars in "Control" window
    cvCreateTrackbar("Min Hue", "BLUE", &iLowH3, 179); //Hue (0 - 179)
    cvCreateTrackbar("Max Hue", "BLUE", &iHighH3, 179);

    cvCreateTrackbar("Min Saturation", "BLUE", &iLowS3, 255); //Saturation (0 - 255)
    cvCreateTrackbar("Max Saturation", "BLUE", &iHighS3, 255);

    cvCreateTrackbar("Min Value", "BLUE", &iLowV3, 255); //Value (0 - 255)
    cvCreateTrackbar("Max Value", "BLUE", &iHighV3, 255);

    // cvMoveWindow("RED", 15, 30);
    // cvMoveWindow("GREEN", 223, 30);         //optional manual placement of windows
    // cvMoveWindow("BLUE", 430, 30);
    // cvMoveWindow("original", 1475, 50);
    // cvMoveWindow("colours", 1475, 500);


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

    double dM013;
    double dM103;

    int shrink = 0 , grow = 0;
    int lastDist = 0;

    int r_dist = 0;
    int l_dist = 0;
    int blank_count=0;
    sem_wait(s1);

    Mat imgLines = Mat::zeros(GLOBAL_ROWS,GLOBAL_COLS, CV_8UC3 );

    Mat disp = Mat::zeros(GLOBAL_ROWS,GLOBAL_COLS, CV_8UC3 );

    for (;;) {
      sem_wait(s1);

      pthread_mutex_lock(&mutex);
      frame = capBuffer.front();
      capBuffer.pop();

      pthread_mutex_unlock(&mutex);
      //imshow("step1",frame);

      cvtColor(frame,im_HSV,COLOR_RGB2HSV);
      im_HSV2 = im_HSV;
      im_HSV3 = im_HSV;

      inRange(im_HSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image
      inRange(im_HSV2, Scalar(iLowH2, iLowS2, iLowV2), Scalar(iHighH2, iHighS2, iHighV2), imgThresholded2); //Threshold the image
      inRange(im_HSV3, Scalar(iLowH3, iLowS3, iLowV3), Scalar(iHighH3, iHighS3, iHighV3), imgThresholded3); //Threshold the image


      disp = Mat::zeros(GLOBAL_ROWS,GLOBAL_COLS, CV_8UC3 );

      // (filter noise from  background)
      erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
      dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

       //(filter small holes in the foreground)
      dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
      erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );


      erode(imgThresholded2, imgThresholded2, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
      dilate( imgThresholded2, imgThresholded2, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
      //repeat for three colours
      dilate( imgThresholded2, imgThresholded2, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
      erode(imgThresholded2, imgThresholded2, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );


      erode(imgThresholded3, imgThresholded3, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
      dilate( imgThresholded3, imgThresholded3, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

      dilate( imgThresholded3, imgThresholded3, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
      erode(imgThresholded3, imgThresholded3, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );


      disp.setTo(Scalar(0,0,255), imgThresholded); //red display
      disp.setTo(Scalar(0,255,0), imgThresholded2); //green display
      disp.setTo(Scalar(255,0,0), imgThresholded3); //red display


      if(save){
        saveSettings(iLowH, iLowS, iLowV, iHighH, iHighS, iHighV,\
                    iLowH2, iLowS2, iLowV2, iHighH2, iHighS2, iHighV2,\
                    iLowH3, iLowS3, iLowV3, iHighH3, iHighS3, iHighV3);
        save = 0;
        printf("Settings Saved\n");

      }

      //get the position of detected shapes
      Moments oMoments = moments(imgThresholded);
      Moments oMoments2 = moments(imgThresholded2);
      Moments oMoments3 = moments(imgThresholded3);

      dM01 = oMoments.m01; //y coord
      dM10 = oMoments.m10; //c coord
      dArea = oMoments.m00;//area

      dM012 = oMoments2.m01;
      dM102 = oMoments2.m10;
      dArea2 = oMoments2.m00;

      dM013 = oMoments3.m01;
      dM103 = oMoments3.m10;
      dArea3 = oMoments3.m00;

      if (dArea > 10000 && dArea2 <1000) //make sure detected area is big enough
      {
       //calculate the position
       int posX = dM10 / dArea;
       int posY = dM01 / dArea;

       if (iLastX >= 0 && iLastY >= 0 && posX >= 0 && posY >= 0) //if something moved
       {

         line(imgLines, Point(posX, posY), Point(iLastX, iLastY), Scalar(0,0,255), 10);

        if(iLastX > posX){
          l_dist = 0;

          r_dist += (iLastX - posX);
          if(r_dist >= (gestureSensitivity+100)){
            printf("dArea: %f RIGHT DETECTED!!! - %d\n",dArea, r_dist);

            system("osascript -e 'activate application \"iBooks\"'");
            //usleep(10000);
            system("osascript -e 'tell application \"System Events\"' -e 'key code 124' -e 'end'");
            imgLines = Mat::zeros(360,640, CV_8UC3 );

            r_dist = 0;
          }
        }

        else if(iLastX < posX){
          r_dist = 0;

          l_dist += (posX - iLastX);
          if(l_dist >= (gestureSensitivity+100)){
            printf ("LEFT DETECTED!!! - %d\n",l_dist);
            system("osascript -e 'activate application \"iBooks\"'");
            //usleep(10000);
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

        int posX = dM10 / dArea;
        int posY = dM01 / dArea;

        int posX2 = dM102 / dArea2;
        int posY2 = dM012 / dArea2;


        if(abs(posX - posX2) < lastDist){
            shrink++;
            grow=0;
            lastDist = abs(posX - posX2);
            if(shrink >= zoomSensitivity){
              shrink = 0;
              printf("SHRINK\n");
              system("osascript -e 'activate application \"iBooks\"'");
              //usleep(10000);
              system("osascript -e 'tell application \"System Events\"' -e 'key code 27 using {shift down, command down}' -e 'end'");
            }
        }
        else if(abs(posX - posX2) > lastDist){
            grow++;
            shrink=0;
            lastDist = abs(posX - posX2);
            if(grow >= zoomSensitivity){
              grow = 0;
              printf("GROW\n");
              system("osascript -e 'activate application \"iBooks\"'");
              //usleep(10000);
              system("osascript -e 'tell application \"System Events\"' -e 'key code 24 using {shift down, command down}' -e 'end'");
            }

        }


        iLastX = posX;
        iLastY = posY;

        iLastX2 = posX2;
        iLastY2 = posY2;

      }

      else if(dArea2 > 10000 && dArea <1000){ //if only green

        int posX2 = dM102 / dArea2;
        int posY2 = dM012 / dArea2;

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
      //add red line to original image
      frame = frame + imgLines;

      //show windows on screen
 			if(!frame.empty() ){
                imshow("colours", disp);
                //imshow("im2", imgThresholded2);f
                imshow("original", frame);
                //imshow("HuePreview", diagBGR);
                waitKey(1);
 			 }
    }
    return 0;
}
