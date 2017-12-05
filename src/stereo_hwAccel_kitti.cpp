
/*
  Author : Brian (Wu Cheng He)
  Email :  brianwchh@gmail.com
  github : https://github.com/brianwchh/zynq-VDMA-driver-StereoVisionApp.git

*/
#define u32 unsigned int

#include "stereo.h"

#include <opencv/highgui.h>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv/cv.h>
#include "opencv2/objdetect/objdetect.hpp"
#include <opencv2/opencv.hpp>

#include <sys/time.h>
#include <sstream>
#include <fstream>
#include <termios.h>


using namespace std ;
using namespace cv;


#define vdma_contrl_address 0x43000000

static char * VDMA_BASEADDR      ; 

// ioctl cmds
#define MEMDEV_IOC_MAGIC  'k'

#define MEMDEV_IOCPRINT         _IO(MEMDEV_IOC_MAGIC, 1)
#define MEMDEV_IOCGETPYADDRESS _IOR(MEMDEV_IOC_MAGIC, 2, int)
#define SET_COLOR_MAP     _IOW(MEMDEV_IOC_MAGIC, 3, int)
#define START_ONE_FRAME   _IOW(MEMDEV_IOC_MAGIC, 4, int)
#define MEMDEV_IOC_MAXNR 5

#define ERR_CLKDIVIDER (1 << CLK_BIT_WEDGE | 1 << CLK_BIT_NOCOUNT)

#define ERR_CLKCOUNTCALC 0xFFFFFFFF //This value is used to signal an error

#define TOTAL_BUF_SIZE           1280 * 720 * 4 * 8
#define FRAME_BUFFER_SIZE (1280 * 720 * 4)

std::string vdevice0 = "/dev/video0" ;


static u32  FRAMEBUFFER_PHY1,FRAMEBUFFER_PHY2,FRAMEBUFFER_PHY3;  //Physical Address of Vbuf
static char*  FRAMEBUFFER_VIR1 ,*FRAMEBUFFER_VIR2,*FRAMEBUFFER_VIR3;    //Virtual Address of Vbuf

static char*  DYNCLK_CONFIG_BASEADDR_VIR ;
static char*  VTC_CONFIG_BASEADDR_VIR ;

#define STARTX   300
#define HW_STRIDE 640


namespace patch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}

// detect keyboard
char getch()
{
//    cout << "key pressed  \n " << endl ;
    fd_set set;
    struct timeval timeout;
    int rv;
    char buff = 0;
    int len = 1;
    int filedesc = 0;
    FD_ZERO(&set);
    FD_SET(filedesc, &set);

    timeout.tv_sec = 0;
    timeout.tv_usec = 1000;

    rv = select(filedesc + 1, &set, NULL, NULL, &timeout);

    struct termios old = {0};
    if (tcgetattr(filedesc, &old) < 0)
        printf("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(filedesc, TCSANOW, &old) < 0)
        printf("tcsetattr ICANON");

    if(rv == -1)
        printf("select");
    else if(rv == 0)
//        ROS_INFO("no_key_pressed");
        rv = 0 ;
    else
        ssize_t siz = read(filedesc, &buff, len );

    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(filedesc, TCSADRAIN, &old) < 0)
        printf ("tcsetattr ~ICANON");
    return (buff);
}


static void Xil_Out32(char * OutAddress, u32 Value)
{
    *(u32 *)OutAddress = Value;
}

static u32 Xil_In32(char* Addr)
{
    return *(u32* ) Addr;
}



int main(int argc, char** argv)
{
    printf("press c for color depth map , g  for gray depth display  \n\r") ;

    int count = 0 ;
    int fbmem;
    int fbVbuf;

    char *FB[6] ;
    char * VDMA1_MM2S_FB[7] ;
    char * VDMA1_S2MM_FB[6] ;
    char * VDMA0_MM2S_FB[6];

    fbmem = open("/dev/mem", O_RDWR | O_SYNC);
    //  allocate video buffers
    fbVbuf = open("/dev/vdma",O_RDWR | O_SYNC);
    if(fbVbuf < 0)
    {
        printf("Vbuf open failed\n");
        return -1 ;
    }

    int ioct_args ;

    ioctl(fbVbuf, MEMDEV_IOCGETPYADDRESS , &FRAMEBUFFER_PHY1);

    FRAMEBUFFER_VIR1 = (char*) mmap(NULL,TOTAL_BUF_SIZE  , PROT_READ | PROT_WRITE, MAP_SHARED, fbmem, (off_t)FRAMEBUFFER_PHY1);
    if(FRAMEBUFFER_VIR1 == MAP_FAILED) {
        perror("VIDEO_BASEADDR_vir mapping for absolute memory access failed.\n");
        return -1;
    }

    /*************  fill in the frame buffer content here***************** */
    int  iPixelAddr = 0 ;
    int heigh = 720 ;
    int width = 1280 ;
    int stride = 1280 * 4 ;

    u32 dataWord ;
    iPixelAddr = 0;
    int stride1280  = 1280 ;

    FB[0] = FRAMEBUFFER_VIR1 ;
    FB[1] = FRAMEBUFFER_VIR1 + FRAME_BUFFER_SIZE ;
    FB[2] = FRAMEBUFFER_VIR1 + FRAME_BUFFER_SIZE * 2 ;
    FB[3] = FRAMEBUFFER_VIR1 + FRAME_BUFFER_SIZE * 3 ;
    FB[4] = FRAMEBUFFER_VIR1 + FRAME_BUFFER_SIZE * 4 ;
    FB[5] = FRAMEBUFFER_VIR1 + FRAME_BUFFER_SIZE * 5 ;
    FB[6] = FRAMEBUFFER_VIR1 + FRAME_BUFFER_SIZE * 6 ;
    FB[7] = FRAMEBUFFER_VIR1 + FRAME_BUFFER_SIZE * 7 ;

    VDMA0_MM2S_FB[0] =  FB[0] ;

    VDMA1_MM2S_FB[0] =  FB[7] ;
    VDMA1_MM2S_FB[1] =  FB[7] ;
    VDMA1_MM2S_FB[2] =  FB[7] ;

    VDMA1_S2MM_FB[0] =  FB[0] ;
    VDMA1_S2MM_FB[1] =  FB[0] ;
    VDMA1_S2MM_FB[2] =  FB[0] ;

     int imgCnt = 0;

     clock_t start,ends;

     Mat   back_ground(720-480,1280,CV_8UC3);

     for(int r=0; r< back_ground.rows ; r++)
     {
         for(int c=0; c<back_ground.cols ; c++)
         {
             back_ground.at<cv::Vec3b>(r,c)[0]  = 0xFF;
             back_ground.at<cv::Vec3b>(r,c)[1]  = 0x00;
             back_ground.at<cv::Vec3b>(r,c)[2]  = 0x00;
         }
     }

     putText(back_ground, "early stage project demo , will be updated soon...... ", Point(80, 30), CV_FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 200), 2);
     putText(back_ground, "Stereo vision proccessed by FPGA, ", Point(600, 80), CV_FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 200), 2);
     putText(back_ground, "core speed 60+FPS , 640 X 480 resolution", Point(600, 120), CV_FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 200), 2);
     putText(back_ground, "email:1503352326@qq.com", Point(80, 80), CV_FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 200), 2);
     putText(back_ground, "brianwchh@gmail.com", Point(80, 140), CV_FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 200), 2);

     iPixelAddr = 0 ;
     for(int r=0; r< back_ground.rows ; r++)
     {
         for(int c= 0; c<back_ground.cols ; c++)
         {
             char B  = back_ground.at<cv::Vec3b>(r,c)[0]  ;
             char G = back_ground.at<cv::Vec3b>(r,c)[1]  ;
             char R =  back_ground.at<cv::Vec3b>(r,c)[2]  ;
             u32 data_w= 0;
            data_w  = data_w  | ( (R << 16)&0x00FF0000) | ( (G << 8)&0x0000FF00) | ( (B << 0)&0x000000FF) ;

             Xil_Out32(VDMA0_MM2S_FB[0]  +  1280*480*4 +    iPixelAddr +  c * 4  , data_w);

         }
         iPixelAddr += stride1280 *4;
     }

     uint16_t*    dispFromMM2S                     = new uint16_t[640 * 480];
     uint16_t*    filteredDisp2MM2S                = new uint16_t[640 * 480];

    while(1){

                        char leftimgstring [200] ;
                        char rightimgstring [200] ;
                        int stingLen ;
                        stingLen = sprintf(leftimgstring, "image_00/data/%010d.png", imgCnt);
                        stingLen = sprintf(rightimgstring, "image_01/data/%010d.png", imgCnt);

                       Mat  left = imread(leftimgstring, 0);
                       Mat right = imread(rightimgstring, 0);

                       if(left.empty() || right.empty()){
                           imgCnt = 0 ;
                           stingLen = sprintf(leftimgstring, "image_00/data/%010d.png", imgCnt);
                           stingLen = sprintf(rightimgstring, "image_01/data/%010d.png", imgCnt);


                          left = imread(leftimgstring, 0);
                          right = imread(rightimgstring, 0);
                       }

                        // write original image data to frame buffer  1280 * 480
                          iPixelAddr = 0 ;
                          for(int r=0; r< left.rows ; r++)
                          {
                              for(int c=640; c<640 * 2; c++)
                              {
                                  u32 data_r = right.at<uchar>(r,c + STARTX -640);
                                  u32 data_r_w = 0 ;
                                  data_r_w   |= ( (data_r << 16)&0x00FF0000) | ( (data_r << 8)&0x0000FF00) | ( (data_r << 0)&0x000000FF) ;
                                  Xil_Out32(VDMA0_MM2S_FB[0]  + iPixelAddr +  c * 4  , data_r_w);
                              }
                              iPixelAddr += stride1280 *4;
                          }

                        // *********  cpy to MM2S frame buffer *****************8
                         iPixelAddr = 0;
                        for (int i = 0; i < left.rows; i++)
                        {
                            int  sj = 0;
                            for (int j = STARTX; j <STARTX + HW_STRIDE ; j+=4,sj+=8)
                            {
                                dataWord = 0x00 ;
                                dataWord = dataWord | ((0xFF << 0   )& (right.at<uchar>(i,j+1)        << 0    )) ;
                                dataWord =  dataWord| ((0xFF << 8   )& (left.at<uchar>(i,j+1)     << 8    )) ;    // left1,right1 (low 16 bits)
                                dataWord = dataWord | ((0xFF << 16)& (right.at<uchar>(i,j)   << 16  )) ;
                                dataWord = dataWord | ((0xFF << 24)& (left.at<uchar>(i,j) << 24 )) ; // left0,right0,  (high 16 bits)
                              Xil_Out32(VDMA1_MM2S_FB[0]  + iPixelAddr +  sj   , dataWord);

                              dataWord = 0x00 ;
                              dataWord = dataWord | ((0xFF << 0   )& (right.at<uchar>(i,j+3)        << 0    )) ;
                              dataWord =  dataWord| ((0xFF << 8   )& (left.at<uchar>(i,j+3)     << 8    )) ;    // left1,right1 (low 16 bits)
                              dataWord = dataWord | ((0xFF << 16)& (right.at<uchar>(i,j+2)   << 16  )) ;
                              dataWord = dataWord | ((0xFF << 24)& (left.at<uchar>(i,j+2) << 24 )) ; // left0,right0,  (high 16 bits)
                                Xil_Out32(VDMA1_MM2S_FB[0]  + iPixelAddr +  sj  + 4 , dataWord);

                            }
                            iPixelAddr += stride1280 ;
                        }

                            int keyV  = getch();
                            int tint = 0x01 ;

                            if(keyV == 'c')
                            {
                                    tint = 0x01 ;
                                     ioctl(fbVbuf,SET_COLOR_MAP, &tint) ;
                            }
                            if(keyV == 'g')
                            {
                                    tint = 0x00;
                                     ioctl(fbVbuf,SET_COLOR_MAP, &tint) ;
                            }

                        // start stereo process in FPGA
                        ioct_args = 1 ;
                        ioctl(fbVbuf, START_ONE_FRAME,&ioct_args );
                        ioct_args = 0 ;
                        ioctl(fbVbuf, START_ONE_FRAME,&ioct_args );

                        imgCnt ++ ;
        }

    // ----------------------------------------------------------------------------

    munmap((void*)FRAMEBUFFER_VIR1,TOTAL_BUF_SIZE) ;

    close(fbVbuf);

    cout<< " done" << endl;

  return 0 ;
}


