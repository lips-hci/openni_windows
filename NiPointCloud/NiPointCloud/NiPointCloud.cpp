#include "stdafx.h"
#include <iostream>
#include <XnOpenNI.h>
#include <XnCppWrapper.h>
#include <opencv2/contrib/contrib.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/viz/vizcore.hpp>
#include <opencv2/viz/viz3d.hpp>
#include <sys/types.h>

using namespace std;
using namespace xn;
using namespace cv;

Mat computeCloud( const Mat depthMap,
                  const float fx,
                  const float fy,
                  const float cx,
                  const float cy )
{
    Mat depth;
    depthMap.convertTo( depth, CV_32F );

    Size nsize = depthMap.size();
    vector<Mat> output( 3 );
    output[0] = Mat( nsize, CV_32F );
    output[1] = Mat( nsize, CV_32F );
    output[2] = depth;

    for ( int i = 0; i < nsize.width; i++ )
    {
        output[0].col( i ) = i;
    }
    for ( int j = 0; j < nsize.height; j++ )
    {
        output[1].row( j ) = j;
    }

    float tmpx = 1.0 / fx;
    float tmpy = 1.0 / fy;
    output[0] = ( output[0] - cx ).mul( output[2] ) * tmpx;
    output[1] = ( output[1] - cy ).mul( output[2] ) * tmpy;

    Mat outMat;
    merge( output, outMat );
    return outMat;
}

int main()
{
    XnDouble fx, fy, cx, cy;
    XnStatus status;

    viz::Viz3d mPCWindow( "VIZ Demo" );

    Context mContext;
    mContext.Init();

    DepthGenerator mDepthGen;
    status = mDepthGen.Create( mContext );

    if ( XN_STATUS_OK == status )
    {
        mDepthGen.GetRealProperty( "fx", fx );
        mDepthGen.GetRealProperty( "fy", fy );
        mDepthGen.GetRealProperty( "cx", cx );
        mDepthGen.GetRealProperty( "cy", cy );

        cout << "fx = " << fx << ", fy = " << fy << ", cx = " << cx << ", cy = " << cy << endl;
    }
    else
    {
        cout << "mDepthGen create fails" << endl;
    }

    mPCWindow.showWidget( "Coordinate Widget", viz::WCoordinateSystem( 400.0 ) );

    mContext.StartGeneratingAll();

    DepthMetaData mDepthMD;

    while ( !xnOSWasKeyboardHit() )
    {
        mContext.WaitOneUpdateAll( mDepthGen );
        mDepthGen.GetMetaData( mDepthMD );

        Mat imgDepth( mDepthMD.FullYRes(), mDepthMD.FullXRes(), CV_16UC1, ( void* )mDepthMD.Data() );
        Mat img8bitDepth;
        Mat img8bit3ChDepth;
        Mat img8bit3ChMask = Mat( mDepthMD.FullYRes(), mDepthMD.FullXRes(), CV_8UC3, Scalar( 0, 255, 255 ) );
        imgDepth.convertTo( img8bitDepth, CV_8U, 255.0 / 10000 );
        cvtColor( img8bitDepth, img8bit3ChDepth, CV_GRAY2BGR );
        bitwise_and( img8bit3ChDepth, img8bit3ChMask, img8bit3ChDepth );
        putText( img8bit3ChDepth, string( "LIPS Corp Copyright 2017" ), Point( 20, 20 ), FONT_HERSHEY_SIMPLEX, 0.5, Scalar( 200, 0, 0 ) );
        imshow( "Depth view", img8bit3ChDepth );
        waitKey( 1 );

        Mat mPointCloud = computeCloud( imgDepth, fx, fy, cx, cy );
        applyColorMap( img8bitDepth, img8bitDepth, COLORMAP_JET );
        viz::WCloud pointCloud = viz::WCloud( mPointCloud, img8bitDepth );
        mPCWindow.showWidget( "Depth", pointCloud );
        mPCWindow.showWidget( "text2d", cv::viz::WText( "LIPS Corp Copyright 2017", Point( 20, 20 ), 15, FONT_HERSHEY_SIMPLEX ) );
        mPCWindow.spinOnce();
        mPCWindow.removeWidget( "Depth" );
        mPCWindow.removeWidget( "text2d" );
    }

    mContext.StopGeneratingAll();
    mContext.Release();

    return 0;
}

