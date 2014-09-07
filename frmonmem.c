//frmonmem
//based on captVideo.c

#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <ctype.h>
#include "./SetHeaderIplImg_VGA.c"

const char gsVersion[]="FrmOnMem_v00_0_0";

enum{
	MAX_N_FRAME = 100,
	MAX_M_TIME = 1,
	WIDTH = 640,
	HEIGHT = 480,
	AREA = WIDTH * HEIGHT
};

IplImage *gpimgMat[MAX_N_FRAME];
//IplImage *gpimgTmp;
IplImage gMimgMat[MAX_N_FRAME];

IplImage *gpimgRep[MAX_N_FRAME];
IplImage gMimgRep[MAX_N_FRAME];

//IplImage gImgLgCapt[276480000]; // 30*30*640*480  //2147483647
long gImgLgCapt[MAX_M_TIME][MAX_N_FRAME][AREA]; // 30*30*640*480  //2147483647
long gImgLgRept[MAX_M_TIME][MAX_N_FRAME][AREA]; // 30*30*640*480  //2147483647

void SetHeaderIplImg_VGA(IplImage *phead);



#define NBUF 1024
int main (int argc, char *argv[]) 
{
  SYSTEMTIME stTime;

	CvCapture *capture = 0;
	CvCapture *captFile=0;
	IplImage *frame = 0;
	IplImage *LoadImg = 0;
	//double w = 640, h = 480;//change to VGA from QVGA by Dzone 120926_17:56 
	//double w = 320, h = 240;//QVGA
	double w = WIDTH, h = HEIGHT;//QVGA
	int c;
	unsigned int uilp;
	const char FileNameSv[]="captImage.avi";
	const char FileNameLd[]="captImage.avi";
	char FNameSv[NBUF];
	unsigned int uiFlagOpenWin=0;
	unsigned int uiIndxCam=0;
	CvVideoWriter *vw;//for video
	CvFont font;//for video
	char str[64];//for video
	int FlmNum=0;
	unsigned int uilpPX;
	
	// usage a.exe [Num camera] [filename] [SwOpenWin]
	
	// (1)�R�}���h�����ɂ���Ďw�肳�ꂽ�ԍ��̃J�����ɑ΂���L���v�`���\���̂��쐬����
//	if (argc == 1 || (argc == 2 && strlen (argv[1]) == 1 && isdigit (argv[1][0])))
	if (argc >1  && strlen (argv[1]) == 1 && isdigit (argv[1][0]))
	{
		//121002_23:01 , capture = cvCreateCameraCapture (argc == 2 ? argv[1][0] - '0' : 0);
		uiIndxCam = argv[1][0] - '0';
		// capture = cvCreateCameraCapture ( uiIndxCam );
		capture = cvCaptureFromCAM  ( uiIndxCam );
	}
	else
		// debug : it can select CamNum if argc > 2, and 2 digit ,by Dzone 121002_23:06 
		if (argc >1  && strlen (argv[1]) == 2 && isdigit (argv[1][0]) && isdigit (argv[1][1]))
		{
			uiIndxCam =(argv[1][1] - '0')+(argv[1][0] - '0')*10;
			// capture = cvCreateCameraCapture ( uiIndxCam );
			capture = cvCaptureFromCAM  ( uiIndxCam );
			// for debug, printf("set %d\n",uiIndxCam);
		}
		else
			// capture = cvCreateCameraCapture (0);
			capture = cvCaptureFromCAM  (0);
			// it will be cared for filename on argument[2]

	//check came by Dzone 121002_22:43 
	if (capture == NULL)
	{
		printf("can not find Camera\n");
		return(255);
	}

	if (argc >2 )
		strcpy(FNameSv,argv[2]);
	else
		if  ((argc == 2) && (isdigit (argv[1][0])!=1))
			strcpy(FNameSv,argv[1]);
		else
		{
			strcpy(FNameSv,FileNameSv);
			printf("save to : %s\n",FNameSv);
		}

	if (argc >3 )
	{
			uiFlagOpenWin = (atoi(argv[3])==1);
			printf("Please push ESC for shutter\n");
	}

	
	/* ���̐ݒ�́C���p����J�����Ɉˑ����� */
	// (2)�L���v�`���T�C�Y��ݒ肷��D
	cvSetCaptureProperty (capture, CV_CAP_PROP_FRAME_WIDTH, w);
	cvSetCaptureProperty (capture, CV_CAP_PROP_FRAME_HEIGHT, h);
	
//	vw = cvCreateVideoWriter (FNameSv, -1, 15, cvSize ((int) w, (int) h),1);
	vw = cvCreateVideoWriter (FNameSv, CV_FOURCC ('M', 'S', 'V', 'C'), 15, cvSize ((int) w, (int) h),1);
	
	
	if(uiFlagOpenWin==1)
	{
		cvNamedWindow ("Capture", CV_WINDOW_AUTOSIZE);
		cvNamedWindow ("Replay", CV_WINDOW_AUTOSIZE);
		cvMoveWindow ("Capture", 50,50);
		cvMoveWindow ("Replay", 700,50);
	}
	

	// (3)�J��������摜���L���v�`������
	for(uilp=0;uilp<10;uilp++){
		frame = cvQueryFrame (capture);
	}

//cvGetTickFrequency()

// // --------------------------------------------------------------
// //  main
// // --------------------------------------------------------------

	//gpimgTmp = cvCreateImage(cvSize(w,h),IPL_DEPTH_8U,3);
	unsigned int uilpT=0;
	for(uilp=0;uilp<MAX_N_FRAME;uilp++)
	{
		// // // gpimgMat[uilp] = cvCreateImage(cvSize(w,h),IPL_DEPTH_8U,3);
//		gpimgMat[uilp] = gpimgTmp;
		gpimgMat[uilp] = &gMimgMat[uilp];
		SetHeaderIplImg_VGA(gpimgMat[uilp]);
		gpimgMat[uilp]->imageData = &gImgLgCapt[uilpT][uilp][0];
//		printf("in %X\n",gpimgMat[uilp]->imageData); // OK

		gpimgRep[uilp] = &gMimgRep[uilp];
		SetHeaderIplImg_VGA(gpimgRep[uilp]);
		gpimgRep[uilp]->imageData = &gImgLgRept[uilpT][uilp][0];

	}


	captFile = cvCaptureFromAVI(FileNameLd);
	

	uilp=0;
	while(1)
	{
		if(NULL==(frame=cvQueryFrame(captFile)))
		{
		    break;
		}
			for(uilpPX=0;uilpPX<(640*480*3);uilpPX++)
			{
		//		gpimgRep[uilp]->imageData[uilpPX] = frame->imageData[uilpPX];
			}
			cvCopyImage(frame,gpimgRep[uilp]);
		//cvShowImage ("Replay", gpimgRep[uilp]);
		c = cvWaitKey (25); // wait n milliseconds
		uilp++;
	}
     

	if(uiFlagOpenWin==1)
	{
//		while (1) {
		for(uilp=0;uilp<MAX_N_FRAME;uilp++){

	//	  GetLocalTime(&stTime);
	//		cvGetTickCount();

			frame = cvQueryFrame (capture);
//			cvWriteFrame (vw, frame);//for video
			
//			cvCopyImage(frame,gpimgMat[uilp]);

			for(uilpPX=0;uilpPX<(640*480*3);uilpPX++)
			{
				gpimgMat[uilp]->imageData[uilpPX] = frame->imageData[uilpPX];
			}

//			 printf("in\t%d\t %X\n",uilp,gpimgMat[uilp]->imageData); // ok
			
			cvShowImage ("Capture", gpimgMat[uilp]);
			cvShowImage ("Replay", gpimgRep[uilp]);
			//frame=cvQueryFrame(captFile);
			//cvShowImage ("Replay", frame);

			c = cvWaitKey (33); // wait n milliseconds
//			c = cvWaitKey (1); // wait n milliseconds
			//c = cvWaitKey (10);
			if (c == '\x1b')
				break;
		}


		cvReleaseCapture (&capture);
//		c = cvWaitKey (1000); // wait n milliseconds


		for(uilp=0;uilp<MAX_N_FRAME;uilp++)
		{
			 cvWriteFrame (vw, gpimgMat[uilp]);//for video
			// it takes too much time!
		}
		cvReleaseVideoWriter (&vw);//for video
		
		cvDestroyWindow ("Capture");
		cvDestroyWindow ("Replay");
		cvNamedWindow ("ReplayNow", CV_WINDOW_AUTOSIZE);
//		c = cvWaitKey (1000); // wait n milliseconds

		for(uilp=0;uilp<MAX_N_FRAME;uilp++)
		{
			// cvCopyImage(gpimgMat[uilp],frame);
//			frame = cvCloneImage(gpimgMat[uilp]);
//			cvWriteFrame (vw, frame);//for video
//			printf("%d\t%X\n",uilp,gpimgMat[uilp]->imageData);

			cvShowImage ("ReplayNow", gpimgMat[uilp]);

			c = cvWaitKey (33); // wait n milliseconds
//			c = cvWaitKey (40); // wait n milliseconds
			//c = cvWaitKey (10);
			if (c == '\x1b')
				break;
		}

//		for(uilp=0;uilp<MAX_N_FRAME;uilp++)
//		{
//			printf("%X\n",gpimgMat[uilp]->imageData);
//		}

	//	cvDestroyWindow ("Capture");
		cvDestroyWindow ("ReplayNow");
	}
// // --------------------------------------------------------------
// // epilogue
// // --------------------------------------------------------------

	cvReleaseCapture (&capture);
	cvReleaseCapture (&captFile);
	//cvDestroyWindow ("Capture");

  return 0;
}