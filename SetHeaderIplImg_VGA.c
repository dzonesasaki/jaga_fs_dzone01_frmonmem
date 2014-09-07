#include <ctype.h>


void SetHeaderIplImg_VGA(IplImage *phead)
{
	
	phead->nSize=112;
	phead->ID=0;
	phead->nChannels=3;
	phead->alphaChannel=0;
	phead->depth=8;
	phead->colorModel[0]=82;
	phead->colorModel[1]=71;
	phead->colorModel[2]=66;
	phead->colorModel[3]=0;
	phead->channelSeq[0]=66;
	phead->channelSeq[1]=71;
	phead->channelSeq[2]=82;
	phead->channelSeq[3]=0;
	phead->dataOrder=0;
	phead->origin=0;
	phead->align=4;
	phead->width=640;
	phead->height=480;
	//phead->roi=0;
	phead->imageSize=921600;
	//phead->imageData=4235456;
	phead->widthStep=1920;
	phead->BorderMode[0]=0;
	phead->BorderMode[1]=0;
	phead->BorderMode[2]=0;
	phead->BorderMode[3]=0;
	phead->BorderConst[0]=0;
	phead->BorderConst[1]=0;
	phead->BorderConst[2]=0;
	phead->BorderConst[3]=0;
	//phead->imageDataOrigin=1100351184;

	return;
}
