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

    double fps;
    char string[5];
    double t_start = 0, t_depth = 0, t_color = 0;

    while ( true )
    {
        t_start = (double)getTickCount();
        mContext.WaitAndUpdateAll();

        DepthMetaData depthData;
        mDepthGen.GetMetaData( depthData );
        t_depth = ((double)getTickCount() - t_start) / getTickFrequency();
        fps = 1.0 / t_depth;
        sprintf_s(string, 9, "%.2f", fps);
        std::string fpsString("FPS:");
        fpsString += string;
        Mat imgDepth( depthData.FullYRes(), depthData.FullXRes(), CV_16UC1, ( void* )depthData.Data() );
        Mat img8bitDepth;
        imgDepth.convertTo( img8bitDepth, CV_8U, 255.0 / 5000 );
        putText(img8bitDepth, fpsString, Point(5, 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(200, 0, 0));
        imshow( "Depth view", img8bitDepth );

        ImageMetaData colorData;
        mImageGen.GetMetaData( colorData );
        t_color = ((double)getTickCount() - t_start) / getTickFrequency();
        fps = 1.0 / t_color;
        sprintf_s(string, 9, "%.2f", fps);
        fpsString = "FPS:";
        fpsString += string;
        Mat imgColor( colorData.FullYRes(), colorData.FullXRes(), CV_8UC3, ( void* )colorData.Data() );
        Mat imgBGRColor;
        cvtColor( imgColor, imgBGRColor, CV_RGB2BGR );
        putText(imgBGRColor, fpsString, Point(5, 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(200, 0, 0));
        imshow( "Color view", imgBGRColor );

        waitKey( 1 );
    }
    return 0;
}

