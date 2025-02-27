#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
using namespace cv;
using namespace std;
#define ESCAPE_KEY (27)
#define SYSTEM_ERROR (-1)

void HoughCircleCam();
Mat frame;
Mat src, dst, cdst, cdstP;
const char* window_name = "Prob HC Transform";
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
	HoughCircleCam();

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
    waitKey();
    return 0;
}
void HoughCircleCam()
{
    Mat gray;
    cvtColor(frame, gray, COLOR_BGR2GRAY);
    medianBlur(gray, gray, 5);
    vector<Vec3f> circles;
    HoughCircles(gray, circles, HOUGH_GRADIENT, 1,
                 gray.rows/16,  // change this value to detect circles with different distances to each other
                 100, 30, 1, 30 // change the last two parameters
            // (min_radius & max_radius) to detect larger circles
    );
    for( size_t i = 0; i < circles.size(); i++ )
    {
        Vec3i c = circles[i];
        Point center = Point(c[0], c[1]);
        // circle center
        circle( frame, center, 1, Scalar(0,100,100), 3, LINE_AA);
        // circle outline
        int radius = c[2];
        circle( frame, center, radius, Scalar(255,0,255), 3, LINE_AA);
    }
    imshow("detected circles", frame);

}

