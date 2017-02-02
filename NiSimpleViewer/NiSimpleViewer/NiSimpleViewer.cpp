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

    mContext.StartGeneratingAll();

    char fpsstr[5];
    double tStart = 0;
    string fpsString;

    while ( true )
    {
        tStart = (double)getTickCount();
        mContext.WaitAndUpdateAll();

        DepthMetaData depthData;
        mDepthGen.GetMetaData( depthData );
        sprintf_s (fpsstr, 9, "%.2f", 1.0 / (((double)getTickCount() - tStart) / getTickFrequency()));
        fpsString = string("FPS:") + fpsstr;
        Mat imgDepth( depthData.FullYRes(), depthData.FullXRes(), CV_16UC1, ( void* )depthData.Data() );
        Mat img8bitDepth;
        imgDepth.convertTo( img8bitDepth, CV_8U, 255.0 / 5000 );
        putText(img8bitDepth, fpsString, Point(5, 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(200, 0, 0));
        imshow( "Depth view", img8bitDepth );

        ImageMetaData colorData;
        mImageGen.GetMetaData( colorData );
        sprintf_s (fpsstr, 9, "%.2f", 1.0 / (((double)getTickCount() - tStart) / getTickFrequency()));
        fpsString = string("FPS:") + fpsstr;
        Mat imgColor( colorData.FullYRes(), colorData.FullXRes(), CV_8UC3, ( void* )colorData.Data() );
        Mat imgBGRColor;
        cvtColor( imgColor, imgBGRColor, CV_RGB2BGR );
        putText(imgBGRColor, fpsString, Point(5, 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(200, 0, 0));
        imshow( "Color view", imgBGRColor );

        waitKey( 1 );
    }
    return 0;
}

