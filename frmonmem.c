//frmonmem
//based on captVideo.c
//
//compile 
// C_INCLUDE_PATH=.\OpenCV2.0\include\opencv;
// LIBRARY_PATH=.\OpenCV2.0\lib;
// gcc frmonmem.c -o frmonmem.exe -lcxcore200.dll -lcv200.dll -lhighgui200.dll -lcvaux200.dll -lml200.dll -lwinmm -g
//

#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <ctype.h>
#include <windows.h>
#include "./SetHeaderIplImg_VGA.c"

const char gsVersion[]="FrmOnMem_v00_0_0";

enum{
	MAX_N_FRAME = 100,
	MAX_M_TIME = 1,
	WIDTH = 640,
	HEIGHT = 480,
	AREA = WIDTH * HEIGHT,
	WAIT_FRAME_MS = 50 //33ms = 30fps , 66 ms = 15fps, 50ms = 20fps
	
};

IplImage *gpimgMat[MAX_N_FRAME];
//IplImage *gpimgTmp;
IplImage gMimgMat[MAX_N_FRAME];

IplImage *gpimgRep[MAX_N_FRAME];
IplImage gMimgRep[MAX_N_FRAME];

unsigned int guiFlagFrameOver=0;

//IplImage gImgLgCapt[276480000]; // 30*30*640*480  //2147483647
long gImgLgCapt[MAX_M_TIME][MAX_N_FRAME][AREA]; // 30*30*640*480  //2147483647
long gImgLgRept[MAX_M_TIME][MAX_N_FRAME][AREA]; // 30*30*640*480  //2147483647

void SetHeaderIplImg_VGA(IplImage *phead);



#define NBUF 1024
//int main (int argc, char *argv[]) 
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, int showCmd)
{
	SYSTEMTIME stTime;
	//int argc;char *argv[];
	CvCapture *capture = 0;
	CvCapture *captFile=0;
	IplImage *frame = 0;
	IplImage *LoadImg = 0;
	//double w = 640, h = 480;//change to VGA from QVGA by Dzone 120926_17:56 
	//double w = 320, h = 240;//QVGA
	double w = WIDTH, h = HEIGHT;//QVGA
	int c;
	unsigned int uilp;
	const char FileNameSv[]="captMv.avi";
	const char FileNameLd[]="captImage.avi";
	char FNameSv[NBUF];
	unsigned int uiFlagOpenWin=0;
	unsigned int uiIndxCam=0;
	CvVideoWriter *vw;//for video
	CvFont font;//for video
	char str[64];//for video
	int FlmNum=0;
	unsigned int uilpPX;
	double dRemainWaitTime;
	double dFreqTick = cvGetTickFrequency();
	long long int llTickA;
	long long int llTickB;
	double dTakesTimeResult;
	long long *pllMemDistination;
	long long *pllMemSource;
	unsigned int uiTargetWaitMs=WAIT_FRAME_MS;
	
	
	guiFlagFrameOver=0;
	// usage a.exe [Num camera] [filename] [SwOpenWin]
/*	
	// (1)コマンド引数によって指定された番号のカメラに対するキャプチャ構造体を作成する
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
			printf("Please push ESC for exit\n");
	}
*/
	capture = cvCaptureFromCAM  ( 0 );

	uiFlagOpenWin=1;
	
	
	/* この設定は，利用するカメラに依存する */
	// (2)キャプチャサイズを設定する．
	cvSetCaptureProperty (capture, CV_CAP_PROP_FRAME_WIDTH, w);
	cvSetCaptureProperty (capture, CV_CAP_PROP_FRAME_HEIGHT, h);
	
	
	
//	if(uiFlagOpenWin==1)
//	{
		cvNamedWindow ("Capture", CV_WINDOW_AUTOSIZE);
		cvNamedWindow ("Replay", CV_WINDOW_AUTOSIZE);
		cvMoveWindow ("Capture", 50,50);
		cvMoveWindow ("Replay", 700,50);
//	}
	

	// (3)カメラから画像をキャプチャする
	for(uilp=0;uilp<10;uilp++){
		frame = cvQueryFrame (capture);
	}

//cvGetTickFrequency()

// // --------------------------------------------------------------
// //  make matrix
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
		cvCopyImage(frame,gpimgRep[uilp]);
		uilp++;
	}


// // --------------------------------------------------------------
// //  check processtime
// // --------------------------------------------------------------

	#define N_LOOP_EVAL_SYS 50
	double dTakesTimeCalc[N_LOOP_EVAL_SYS];
	printf("checking process time\n");
	for(uilp=0;uilp<N_LOOP_EVAL_SYS;uilp++)
	{
		llTickA = cvGetTickCount();
		frame = cvQueryFrame (capture);
		for(uilpPX=0;uilpPX<(640*480*3);uilpPX++)
		{
			gpimgMat[uilp]->imageData[uilpPX] = frame->imageData[uilpPX];
		}
		cvShowImage ("Capture", gpimgMat[uilp]);
		cvShowImage ("Replay", gpimgRep[uilp]);
		llTickB = cvGetTickCount();
		dTakesTimeResult = ( llTickB - llTickA)/cvGetTickFrequency() /1000;
		dTakesTimeCalc[uilp]=dTakesTimeResult;
	}
	//printf("takeTicks%e\n",cvGetTickCount() - cvGetTickCount());
	//for(uilp=0;uilp<N_LOOP_EVAL_SYS;uilp++)
	//	printf("time%d\t%e\n",uilp,dTakesTimeCalc[uilp]);
	// takeTicks0.000000e+000
	// time0   8.883266e+000
	// time1   1.295886e+001
	// time2   3.246333e+001
	// time3   3.233828e+001
	// time4   3.217605e+001
	// time5   3.334306e+001
	// time6   3.247026e+001
	// time7   3.251036e+001
	// time8   3.275937e+001
	// time9   3.179434e+001

	double dMaxTmp=0;
	double dSumTmp=0;
	
	for(uilp=0;uilp<N_LOOP_EVAL_SYS;uilp++)
	{
		// printf("time%d\t%e\n",uilp,dTakesTimeCalc[uilp]);
		dSumTmp += dTakesTimeCalc[uilp]*(uilp!=0); //skip first count
		if (dMaxTmp<dTakesTimeCalc[uilp])
		{
			dMaxTmp = dTakesTimeCalc[uilp];
		}
	}
	uiTargetWaitMs = (int)(dMaxTmp+0.9)+10;
	printf("max time= %f\tavefage= %f\n",dMaxTmp, dSumTmp/(N_LOOP_EVAL_SYS-1));




// 	for(uilp=0;uilp<10;uilp++)
// 	{
// 		llTickA = cvGetTickCount();
// 		frame = cvQueryFrame (capture);
// 		pllMemDistination =(long long *)&(gpimgMat[uilp]->imageData[0]);
// 		pllMemSource =(long long *)&(frame->imageData[0]);
// 		for(uilpPX=0;uilpPX<(640*480*3)/8;uilpPX++)
// 		{
// 			pllMemDistination[uilpPX] = pllMemSource[uilpPX];
// 		}
// // 		for(uilpPX=0;uilpPX<(640*480*3);uilpPX++)
// // 		{
// // 			gpimgMat[uilp]->imageData[uilpPX] = frame->imageData[uilpPX];
// // 		}
// 		cvShowImage ("Capture", gpimgMat[uilp]);
// 		cvShowImage ("Replay", gpimgRep[uilp]);
// 		llTickB = cvGetTickCount();
// 		dTakesTimeResult = ( llTickB - llTickA)/cvGetTickFrequency() /1000;
// 		dTakesTimeCalc[uilp]=dTakesTimeResult;
// 	}
// 	printf("takeTicks%e\n",cvGetTickCount() - cvGetTickCount());
// 	for(uilp=0;uilp<10;uilp++)
// 		printf("time%d\t%e\n",uilp,dTakesTimeCalc[uilp]);
// 	// takeTicks0.000000e+000
// 	// time0   1.591157e+001
// 	// time1   3.230511e+001
// 	// time2   4.783168e+001
// 	// time3   3.198355e+001
// 	// time4   3.180892e+001
// 	// time5   3.214761e+001
// 	// time6   3.181767e+001
// 	// time7   3.235141e+001
// 	// time8   3.185340e+001
// 	// time9   3.200032e+001
// 

// // --------------------------------------------------------------
// //  main
// // --------------------------------------------------------------
		dFreqTick = cvGetTickFrequency();
		dRemainWaitTime=0;
//	if(uiFlagOpenWin==1)
//	{
	  PlaySound("c:\\tst.wav",NULL,SND_FILENAME | SND_ASYNC);//sound start
//		while (1) {
		for(uilp=0;uilp<MAX_N_FRAME;uilp++){

	//	  GetLocalTime(&stTime);
	//		cvGetTickCount();
			llTickA = cvGetTickCount();

			frame = cvQueryFrame (capture);
//			cvWriteFrame (vw, frame);//for video
			
//			cvCopyImage(frame,gpimgMat[uilp]);

				for(uilpPX=0;uilpPX<(640*480*3);uilpPX++)
				{
					gpimgMat[uilp]->imageData[uilpPX] = frame->imageData[uilpPX];
				}

// 			pllMemDistination =(long long *)&(gpimgMat[uilp]->imageData[0]);
// 			pllMemSource =(long long *)&(frame->imageData[0]);
// 			for(uilpPX=0;uilpPX<(640*480*3)/8;uilpPX++)
// 			{
// 				pllMemDistination[uilpPX] = pllMemSource[uilpPX];
// 			}

//			 printf("in\t%d\t %X\n",uilp,gpimgMat[uilp]->imageData); // ok
			
			cvShowImage ("Capture", gpimgMat[uilp]);
			cvShowImage ("Replay", gpimgRep[uilp]);
			//frame=cvQueryFrame(captFile);
			//cvShowImage ("Replay", frame);

			llTickB = cvGetTickCount();
			dTakesTimeResult = ( llTickB - llTickA)/dFreqTick /1000;
			dRemainWaitTime = uiTargetWaitMs - dTakesTimeResult;

			if (dRemainWaitTime <= 0)
			{
				guiFlagFrameOver=1;
				dRemainWaitTime=1;
			}
			else
			{
				c = cvWaitKey ((int)dRemainWaitTime); // wait n milliseconds
			}
			//c = cvWaitKey (33); // wait n milliseconds
			if (c == '\x1b')
				break;
		}

		PlaySound(NULL,NULL,0);//sound stop


		cvReleaseCapture (&capture);
		cvDestroyWindow ("Capture");
		cvDestroyWindow ("Replay");


// // --------------------------------------------------------------
// //  replay
// // --------------------------------------------------------------
		cvNamedWindow ("ReplayNow", CV_WINDOW_AUTOSIZE);
//		c = cvWaitKey (1000); // wait n milliseconds

		for(uilp=0;uilp<MAX_N_FRAME;uilp++)
		{
			// cvCopyImage(gpimgMat[uilp],frame);
//			frame = cvCloneImage(gpimgMat[uilp]);
//			cvWriteFrame (vw, frame);//for video
//			printf("%d\t%X\n",uilp,gpimgMat[uilp]->imageData);

			cvShowImage ("ReplayNow", gpimgMat[uilp]);

			c = cvWaitKey (uiTargetWaitMs); // wait n milliseconds
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
//	}


// // --------------------------------------------------------------
// //  file save
// // --------------------------------------------------------------
		printf("guiFlagFrameOver=%d\n",guiFlagFrameOver);
		printf("waitTime=%d[ms]\n",uiTargetWaitMs);
		double dRecodeFps = (1/(double)(uiTargetWaitMs)*1000);
		printf("Frame Per Sec =%f\n",dRecodeFps);
		printf(" save file now....");
//	vw = cvCreateVideoWriter (FNameSv, -1, 15, cvSize ((int) w, (int) h),1);
//		vw = cvCreateVideoWriter (FNameSv, CV_FOURCC ('M', 'S', 'V', 'C'), 15, cvSize (WIDTH, HEIGHT),1);
		vw = cvCreateVideoWriter (FNameSv, CV_FOURCC ('M', 'S', 'V', 'C'), dRecodeFps, cvSize (WIDTH, HEIGHT),1);

		for(uilp=0;uilp<MAX_N_FRAME;uilp++)
		{
			 cvWriteFrame (vw, gpimgMat[uilp]);//for video
			// it takes too much time!
			if(uilp%(MAX_N_FRAME/8)==0)	printf("..");
		}
		cvReleaseVideoWriter (&vw);//for video
		printf("...done\n");

// // --------------------------------------------------------------
// // epilogue
// // --------------------------------------------------------------

	cvReleaseCapture (&capture);
	cvReleaseCapture (&captFile);
	//cvDestroyWindow ("Capture");

  return 0;
}
