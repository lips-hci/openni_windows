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
    while ( true )
    {
        mContext.WaitAndUpdateAll();

        DepthMetaData depthData;
        mDepthGen.GetMetaData( depthData );
        Mat imgDepth( depthData.FullYRes(), depthData.FullXRes(), CV_16UC1, ( void* )depthData.Data() );
        Mat img8bitDepth;
        imgDepth.convertTo( img8bitDepth, CV_8U, 255.0 / 5000 );
        imshow( "Depth view", img8bitDepth );

        ImageMetaData colorData;
        mImageGen.GetMetaData( colorData );
        Mat imgColor( colorData.FullYRes(), colorData.FullXRes(), CV_8UC3, ( void* )colorData.Data() );
        Mat imgBGRColor;
        cvtColor( imgColor, imgBGRColor, CV_RGB2BGR );
        imshow( "Color view", imgBGRColor );

        waitKey( 1 );
    }
    return 0;
}

