#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
using namespace cv;
using namespace std;

#define ESCAPE_KEY (27)
#define SYSTEM_ERROR (-1)

Mat frame;
Mat src, dst, cdst, cdstP;
const char* window_name = "Prob HL Transform";
void HoughLinePTransform();
int main(int argc, char** argv)
{
   VideoCapture cam0(0);
   namedWindow("video_display");
   char winInput;

   if (!cam0.isOpened())
   {
       exit(SYSTEM_ERROR);
   }

   cam0.set(CAP_PROP_FRAME_WIDTH, 640);
   cam0.set(CAP_PROP_FRAME_HEIGHT, 480);

   while(1){
   	cam0.read(frame);
     	imshow("video_display", frame);
    Canny(frame, dst, 80, 240, 3);
    // Copy edges to the images that will display the results in BGR
    cvtColor(dst, cdst, COLOR_GRAY2BGR);
    cdstP = cdst.clone();
      namedWindow( window_name, WINDOW_AUTOSIZE );
      HoughLinePTransform();
      if ((winInput = waitKey(10)) == ESCAPE_KEY)
      //if ((winInput = waitKey(0)) == ESCAPE_KEY)
      {
          break;
      }
      else if(winInput == 'n')
      {
          printf("input %c is ignored\n", winInput);
      }
   }
	
   destroyWindow("video_display");
    // Edge detection
    //
    // See - https://docs.opencv.org/4.1.1/da/d5c/tutorial_canny_detector.html
    //
    //Canny(src, dst, 80, 240, 3);
    // Copy edges to the images that will display the results in BGR
    //cvtColor(dst, cdst, COLOR_GRAY2BGR);
    //cdstP = cdst.clone();
	
    //imshow("Detected Lines (in red) - Standard Hough Line Transform", cdst);
    //imshow("Detected Lines (in red) - Probabilistic Line Transform", cdstP);
    // Wait and Exit
    waitKey();
    return 0;
}

void HoughLinePTransform(){
	vector<Vec4i> linesP; // will hold the results of the detection
   	 HoughLinesP(dst, linesP, 1, CV_PI/180, 50, 50, 10 ); // runs the actual detection
   	 // Draw the lines
   	 for( size_t i = 0; i < linesP.size(); i++ )
   	 {
   	     Vec4i l = linesP[i];
   	     line( cdstP, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, LINE_AA);
   	     line( src, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, LINE_AA);
   	 }
	 imshow(window_name,cdstP);
}
