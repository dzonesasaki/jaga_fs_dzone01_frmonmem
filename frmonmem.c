//frmonmem
//based on captVideo.c
//
//compile 
// C_INCLUDE_PATH=.\OpenCV2.0\include\opencv;
// LIBRARY_PATH=.\OpenCV2.0\lib;
// gcc frmonmem.c -o frmonmem.exe -lcxcore200.dll -lcv200.dll -lhighgui200.dll -lcvaux200.dll -lml200.dll -lwinmm -g  -mwindows -lwinmm
//

#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <ctype.h>
#include <windows.h>
#include "./SetHeaderIplImg_VGA.c"

const char gsVersion[]="FrmOnMem_v00_1_0";

enum{
	MAX_N_FRAME = 400,
	MAX_M_TIME = 1,
	WIDTH = 640,
	HEIGHT = 480,
	AREA = WIDTH * HEIGHT,
	WAIT_FRAME_MS = 105 //33ms = 30fps , 66 ms = 15fps, 50ms = 20fps , 105 is macBook
	
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

unsigned int uiMatHBuf[WIDTH*3];

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
	double w = WIDTH, h = HEIGHT;//QVGA
	int c;
	unsigned int uilp;
	const char FileNameSv[]=".\\captMv.avi"; //"captMv.avi";
	const char FileNameLd[]="captImage.avi";
	char FNameSv[NBUF];
	char FNameLd[NBUF];
	unsigned int uiFlagOpenWin=0;
	unsigned int uiIndxCam=0;
	CvVideoWriter *vw;//for video
	CvFont font;//for video
	char str[64];//for video
	int FlmNum=0;
	unsigned int uilpPX,uilpPY,uilpPBGR;
	double dRemainWaitTime;
	double dFreqTick = cvGetTickFrequency();
	long long int llTickA;
	long long int llTickB;
	double dTakesTimeResult;
	long long *pllMemDistination;
	long long *pllMemSource;
	unsigned int uiTargetWaitMs=WAIT_FRAME_MS;
	unsigned int uiNumFrameMax=MAX_N_FRAME;
	unsigned int uiFlCounter =1;
	FILE *fid;
	char cMessBuf[256];

	guiFlagFrameOver=0;
	capture = cvCaptureFromCAM  ( 0 );

	cvSetCaptureProperty (capture, CV_CAP_PROP_FRAME_WIDTH, w);
	cvSetCaptureProperty (capture, CV_CAP_PROP_FRAME_HEIGHT, h);
	


// // --------------------------------------------------------------
// //  make matrix
// // --------------------------------------------------------------

	//gpimgTmp = cvCreateImage(cvSize(w,h),IPL_DEPTH_8U,3);
	unsigned int uilpT=0;
	for(uilp=0;uilp<MAX_N_FRAME;uilp++)
	{
		gpimgMat[uilp] = &gMimgMat[uilp];
		SetHeaderIplImg_VGA(gpimgMat[uilp]);
		gpimgMat[uilp]->imageData = (char *)&gImgLgCapt[uilpT][uilp][0];

		gpimgRep[uilp] = &gMimgRep[uilp];
		SetHeaderIplImg_VGA(gpimgRep[uilp]);
		gpimgRep[uilp]->imageData = (char *)&gImgLgRept[uilpT][uilp][0];
	}

// // --------------------------------------------------------------
// //  check last file
// // --------------------------------------------------------------

	for (uilp=0;uilp<1000;uilp++) // todo: change max num
	{
		wsprintf(FNameLd,".\\captMv%05d.avi",uilp);
		fid=fopen(FNameLd,"r");
		if (fid==NULL)
		{
			wsprintf(cMessBuf,"no file: %X",uilp);
			//MessageBox(NULL,cMessBuf , "error", MB_OK);
			break;
		}
		fclose(fid);
	}
	uiFlCounter = uilp-1;

// // --------------------------------------------------------------
// //  load previous file
// // --------------------------------------------------------------
	wsprintf(FNameLd,".\\captMv%05d.avi",uiFlCounter);
	captFile = cvCaptureFromAVI(FNameLd);
	uilp=0;
	while(1) // todo: change max loop
	{
		if(NULL==(frame=cvQueryFrame(captFile)))
		{
		    break;
		}
		cvCopyImage(frame,gpimgMat[uilp]);
		uilp++;
	}
	uiNumFrameMax = uilp;
	cvReleaseCapture (&captFile);

	// flip LR
	for(uilp=0;uilp<uiNumFrameMax;uilp++)
	for(uilpPY=0;uilpPY<(HEIGHT);uilpPY++)
	for(uilpPX=0;uilpPX<(WIDTH);uilpPX++)
	for(uilpPBGR=0;uilpPBGR<(3);uilpPBGR++)
	{
		gpimgRep[uilp]->imageData[WIDTH*3*uilpPY+ WIDTH*3-uilpPX*3 + uilpPBGR ] = gpimgMat[uilp]->imageData[WIDTH*3*uilpPY+uilpPX*3 +uilpPBGR];
	}


// // --------------------------------------------------------------
// //  check processtime
// // --------------------------------------------------------------

	cvNamedWindow ("Capture", CV_WINDOW_AUTOSIZE);
	cvNamedWindow ("Replay", CV_WINDOW_AUTOSIZE);
	cvMoveWindow ("Capture", 50,50);
	cvMoveWindow ("Replay", 700,50);

	#define N_LOOP_EVAL_SYS 50
	double dTakesTimeCalc[N_LOOP_EVAL_SYS];
	printf("checking process time\n");
	for(uilp=0;uilp<N_LOOP_EVAL_SYS;uilp++)
	{
		llTickA = cvGetTickCount();
		frame = cvQueryFrame (capture);
		for(uilpPY=0;uilpPY<(HEIGHT);uilpPY++)
		for(uilpPX=0;uilpPX<(WIDTH);uilpPX++)
		for(uilpPBGR=0;uilpPBGR<(3);uilpPBGR++)
		{
			gpimgMat[uilp]->imageData[WIDTH*3*uilpPY+ WIDTH*3-uilpPX*3 + 2-uilpPBGR ] = frame->imageData[WIDTH*3*uilpPY+uilpPX*3 +uilpPBGR];
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
	cvDestroyWindow ("Capture");
	cvDestroyWindow ("Replay");

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
	if (uiTargetWaitMs < WAIT_FRAME_MS)
	{
		uiTargetWaitMs = WAIT_FRAME_MS;
		printf("change wait to = %d\n",uiTargetWaitMs);
	}


// // --------------------------------------------------------------
// //  title
// // --------------------------------------------------------------

	IplImage *imgPict = 0;

	cvNamedWindow ("Capture", CV_WINDOW_AUTOSIZE);
	cvNamedWindow ("Replay", CV_WINDOW_AUTOSIZE);
	cvMoveWindow ("Capture", 50,50);
	cvMoveWindow ("Replay", 700,50);

	cvNamedWindow ("Title", CV_WINDOW_AUTOSIZE);
	cvMoveWindow ("Title", 50,50);
	imgPict=cvLoadImage(".\\title.png",CV_LOAD_IMAGE_ANYCOLOR);
	cvShowImage ("Title", imgPict);
	c = cvWaitKey (2000); // wait n milliseconds
	// cvDestroyWindow ("Title");


// // --------------------------------------------------------------
// //  instruction
// // --------------------------------------------------------------

	imgPict=cvLoadImage(".\\inst.png",CV_LOAD_IMAGE_ANYCOLOR);
	cvShowImage ("Title", imgPict);
	c = cvWaitKey (2000); // wait n milliseconds
	imgPict=cvLoadImage(".\\3.png",CV_LOAD_IMAGE_ANYCOLOR);
	cvShowImage ("Title", imgPict);
	c = cvWaitKey (1000); // wait n milliseconds
	imgPict=cvLoadImage(".\\2.png",CV_LOAD_IMAGE_ANYCOLOR);
	cvShowImage ("Title", imgPict);
	c = cvWaitKey (1000); // wait n milliseconds
	imgPict=cvLoadImage(".\\1.png",CV_LOAD_IMAGE_ANYCOLOR);
	cvShowImage ("Title", imgPict);
	c = cvWaitKey (1000); // wait n milliseconds

	cvDestroyWindow ("Title");

// // --------------------------------------------------------------
// //  main
// // --------------------------------------------------------------
	dFreqTick = cvGetTickFrequency();
	dRemainWaitTime=0;
	PlaySound("kakumabon03.wav",NULL,SND_FILENAME | SND_ASYNC);//sound start
	for(uilp=0;uilp<uiNumFrameMax;uilp++)
	{
		llTickA = cvGetTickCount();
		frame = cvQueryFrame (capture);
		for(uilpPY=0;uilpPY<(HEIGHT);uilpPY++)
		for(uilpPX=0;uilpPX<(WIDTH);uilpPX++)
		for(uilpPBGR=0;uilpPBGR<(3);uilpPBGR++)
		{
			gpimgMat[uilp]->imageData[WIDTH*3*uilpPY+ WIDTH*3-uilpPX*3 + uilpPBGR ] = frame->imageData[WIDTH*3*uilpPY+uilpPX*3 +uilpPBGR];
		}
//				for(uilpPX=0;uilpPX<(WIDTH*HEIGHT*3);uilpPX++)
//				{
//					gpimgMat[uilp]->imageData[uilpPX] = frame->imageData[uilpPX];
//				}
// 			pllMemDistination =(long long *)&(gpimgMat[uilp]->imageData[0]);
// 			pllMemSource =(long long *)&(frame->imageData[0]);
// 			for(uilpPX=0;uilpPX<(640*480*3)/8;uilpPX++)
// 			{
// 				pllMemDistination[uilpPX] = pllMemSource[uilpPX];
// 			}

//			 printf("in\t%d\t %X\n",uilp,gpimgMat[uilp]->imageData); // ok
			
		cvShowImage ("Capture", gpimgMat[uilp]);
		cvShowImage ("Replay", gpimgRep[uilp]);

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
		if (c == '\x1b') break;
	}// end of for uilp

	PlaySound(NULL,NULL,0);//sound stop

	cvDestroyWindow ("Capture");
	cvDestroyWindow ("Replay");
	cvReleaseCapture (&capture);

// // --------------------------------------------------------------
// //  invert
// // --------------------------------------------------------------

	for(uilp=0;uilp<uiNumFrameMax;uilp++)
	for(uilpPY=0;uilpPY<(HEIGHT);uilpPY++)
	{
		for(uilpPX=0;uilpPX<(WIDTH);uilpPX++)
		for(uilpPBGR=0;uilpPBGR<(3);uilpPBGR++)
		{
			uiMatHBuf[WIDTH*3-uilpPX*3 + uilpPBGR ] = gpimgMat[uilp]->imageData[WIDTH*3*uilpPY+uilpPX*3 +uilpPBGR];
		}
		for(uilpPX=0;uilpPX<(WIDTH*3);uilpPX++)
			gpimgMat[uilp]->imageData[WIDTH*3*uilpPY+ uilpPX ] = uiMatHBuf[uilpPX];
	}


// // --------------------------------------------------------------
// //  replay
// // --------------------------------------------------------------

	cvNamedWindow ("ReplayNow", CV_WINDOW_AUTOSIZE);
	cvMoveWindow ("Capture", 50,50);
	PlaySound("kakumabon03.wav",NULL,SND_FILENAME | SND_ASYNC);//sound start
	for(uilp=0;uilp<uiNumFrameMax;uilp++)
	{
		llTickA = cvGetTickCount();
		cvShowImage ("ReplayNow", gpimgMat[uilp]);
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
		if (c == '\x1b') break;
	}
	cvDestroyWindow ("ReplayNow");


	int iFlgRtn = 0;
// // --------------------------------------------------------------
// //  file save
// // --------------------------------------------------------------
	cvNamedWindow ("Title", CV_WINDOW_AUTOSIZE);
	cvMoveWindow ("Title", 50,50);
	imgPict=cvLoadImage(".\\savenow.png",CV_LOAD_IMAGE_ANYCOLOR);
	cvShowImage ("Title", imgPict);
	c = cvWaitKey (2000); // wait n milliseconds

	double dRecodeFps = (1/(double)(uiTargetWaitMs)*1000);
//	MessageBox(NULL, sprintf("ovFlg=%d intrvl=%d[ms]",guiFlagFrameOver,uiTargetWaitMs), "message", MB_OK);
	char cFnamesvbuf[16];
	uiFlCounter = uiFlCounter+1;
	wsprintf(cFnamesvbuf,".\\captMv%05d.avi",uiFlCounter);
	vw = cvCreateVideoWriter (cFnamesvbuf, CV_FOURCC ('M', 'S', 'V', 'C'), dRecodeFps, cvSize (WIDTH, HEIGHT),1);

	if (vw == NULL)
	{
		wsprintf(cMessBuf,"ERROR in create video writer %X",vw);
		MessageBox(NULL,cMessBuf , "error", MB_OK);
	}
	else
	{
		wsprintf(cMessBuf,"create video writer %X",vw);
//			MessageBox(NULL,cMessBuf , "message", MB_OK);
	}

//		MessageBox(NULL, "save file start", "message", MB_OK);
	for(uilp=0;uilp<uiNumFrameMax;uilp++)
	{
		iFlgRtn=cvWriteFrame (vw, gpimgMat[uilp]);//for video
		//return 1 is correct. if (iFlgRtn!=0)
/*			if (iFlgRtn!=1)
		{
			wsprintf(cMessBuf,"error in write frame %d",iFlgRtn);
			MessageBox(NULL, cMessBuf, "error", MB_OK);
			break;
		}*/
		// it takes too much time!
	}
	cvReleaseVideoWriter (&vw);//for video
//	MessageBox(NULL, "save file done", "message", MB_OK);
	cvDestroyWindow ("Title");

// // --------------------------------------------------------------
// // epilogue
// // --------------------------------------------------------------


  return 0;
}


// 
// to do
// 
// 1: change flow to loop
// 2: add demo routine
// 3: change max num of search record file
// 4: sound play from memory
// 5: lead replay and ontime replay on small window
// 6: 
