// NiSimpleViewer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;
using namespace xn;
using namespace cv;

int _tmain(int argc, _TCHAR* argv[])
{
    Context mContext;
    mContext.Init();

    DepthGenerator mDepthGen;
    mDepthGen.Create( mContext );

    ImageGenerator mImageGen;
    mImageGen.Create( mContext );

    IRGenerator mIrGen;
    mIrGen.Create( mContext );

    mContext.StartGeneratingAll();

    char fpsstr[7];
    double tStart = 0;

    while ( !xnOSWasKeyboardHit() )
    {
        tStart = (double)getTickCount();
        mContext.WaitAndUpdateAll();

        DepthMetaData depthData;
        mDepthGen.GetMetaData( depthData );
        Mat imgDepth( depthData.FullYRes(), depthData.FullXRes(), CV_16UC1, ( void* )depthData.Data() );
        Mat img8bitDepth;
        imgDepth.convertTo( img8bitDepth, CV_8U, 255.0 / 4096 );
        sprintf_s(fpsstr, sizeof(fpsstr), "%.2f", 1.0 / (((double)getTickCount() - tStart) / getTickFrequency()));
        putText(img8bitDepth, string("FPS:") + fpsstr, Point(5, 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(200, 0, 0));
        imshow( "Depth view", img8bitDepth );

        ImageMetaData colorData;
        mImageGen.GetMetaData( colorData );
        Mat imgColor( colorData.FullYRes(), colorData.FullXRes(), CV_8UC3, ( void* )colorData.Data() );
        Mat imgBGRColor;
        cvtColor( imgColor, imgBGRColor, CV_RGB2BGR );
        sprintf_s(fpsstr, sizeof(fpsstr), "%.2f", 1.0 / (((double)getTickCount() - tStart) / getTickFrequency()));
        putText(imgBGRColor, string("FPS:") + fpsstr, Point(5, 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(200, 0, 0));
        imshow( "Color view", imgBGRColor );

        IRMetaData irData;
        mIrGen.GetMetaData( irData );
        Mat imgIR( irData.FullYRes(), irData.FullXRes(), CV_16UC1, ( void * )irData.Data() );
        Mat img8bitIR;
        imgIR.convertTo( img8bitIR, CV_8U, 255.0 / 4096 );
        sprintf_s(fpsstr, sizeof(fpsstr), "%.2f", 1.0 / (((double)getTickCount() - tStart) / getTickFrequency()));
        putText(img8bitIR, string("FPS:") + fpsstr, Point(5, 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(200, 0, 0));
        imshow( "IR view", img8bitIR );

        waitKey( 1 );
    }
    mDepthGen.Release();
    mImageGen.Release();
    mContext.Release();
    return 0;
}

