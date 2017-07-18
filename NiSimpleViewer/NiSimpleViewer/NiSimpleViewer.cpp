// NiSimpleViewer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;
using namespace xn;
using namespace cv;

#define FPG_AVG_COUNT 60

void onMouse(int Event, int x, int y, int flags, void* param);
int Xres = 0;
int Yres = 0;
UINT16 Zres = 0;

enum showOp {
    DEPTH = 1,
    IMAGE = 2,
    IR = 4
};

int getUserInput() {
    int option = 0;
    cout << "1) Depth only" << endl;
    cout << "2) Image only" << endl;
    cout << "3) IR only" << endl;
    cout << "4) Depth and Image" << endl;
    cout << "5) Depth and IR" << endl;
    cout << "6) Image and IR" << endl;
    cout << "7) All" << endl;
    cout << "0) Exit" << endl;
    cout << "Please input your choice : ";
    cin >> option;
    switch (option) {
    case 1:
        return DEPTH;
        break;
    case 2:
        return IMAGE;
        break;
    case 3:
        return IR;
        break;
    case 4:
        return (DEPTH + IMAGE);
        break;
    case 5:
        return (DEPTH + IR);
        break;
    case 6:
        return (IMAGE + IR);
        break;
    case 7:
        return (DEPTH + IMAGE + IR);
        break;
    case 0:
        return 0;
        break;
    default:
        return getUserInput();
        break;
    };
}

void onMouse( int Event, int x, int y, int flags, void* param )
{
    if (Event == EVENT_MOUSEMOVE)
    {
        Xres = x;
        Yres = y;
    }
}

int _tmain(int argc, _TCHAR* argv[])
{
    int option = getUserInput();

    if ( 0 == option ) {
        cout << "Exit program!" << endl;
        return 0;
    }

    Context mContext;

    DepthGenerator mDepthGen;
    ImageGenerator mImageGen;
    IRGenerator mIrGen;

    DepthMetaData depthData;
    ImageMetaData colorData;
    IRMetaData irData;

    XnFPSData xnDepthFPS, xnColorFPS, xnIrFPS;

    mContext.Init();

    if ( option & DEPTH ) {
        mDepthGen.Create( mContext );
        xnFPSInit(&xnDepthFPS, FPG_AVG_COUNT);
    }
    if ( option & IMAGE ) {
        mImageGen.Create( mContext );
        xnFPSInit(&xnColorFPS, FPG_AVG_COUNT);
    }
    if ( option & IR ) {
        mIrGen.Create( mContext );
        xnFPSInit(&xnIrFPS, FPG_AVG_COUNT);
    }

    mContext.StartGeneratingAll();

    char fpsstr[7];
    char xyzstr[128];
    double tStart = 0;

    while ( !xnOSWasKeyboardHit() )
    {
        tStart = (double)getTickCount();
        mContext.WaitAndUpdateAll();

        if ( option & DEPTH ) {
            mDepthGen.GetMetaData( depthData );
            Mat imgDepth( depthData.FullYRes(), depthData.FullXRes(), CV_16UC1, ( void* )depthData.Data() );
            Mat img8bitDepth;
            imgDepth.convertTo( img8bitDepth, CV_8U, 255.0 / 4096 );
            xnFPSMarkFrame(&xnDepthFPS);
            sprintf_s(fpsstr, sizeof(fpsstr), "%.2f", xnFPSCalc(&xnDepthFPS));
            putText(img8bitDepth, string("FPS:") + fpsstr, Point(5, 20), FONT_HERSHEY_DUPLEX, 1.0, Scalar(200, 0, 0));
            Zres = depthData(Xres, Yres);
            sprintf_s(xyzstr, sizeof(xyzstr), "X : %d, Y : %d, Depth : %u", Xres, Yres, Zres);
            putText(img8bitDepth, xyzstr, Point(5, 50), FONT_HERSHEY_DUPLEX, 1.0, Scalar(200, 0, 0));
            imshow( "Depth view", img8bitDepth );
            setMouseCallback( "Depth view", onMouse, NULL );
        }

        if ( option & IMAGE ) {
            mImageGen.GetMetaData( colorData );
            Mat imgColor( colorData.FullYRes(), colorData.FullXRes(), CV_8UC3, ( void* )colorData.Data() );
            Mat imgBGRColor;
            cvtColor( imgColor, imgBGRColor, CV_RGB2BGR );
            xnFPSMarkFrame(&xnColorFPS);
            sprintf_s(fpsstr, sizeof(fpsstr), "%.2f", xnFPSCalc(&xnColorFPS));
            putText(imgBGRColor, string("FPS:") + fpsstr, Point(5, 20), FONT_HERSHEY_DUPLEX, 1.0, Scalar(200, 0, 0));
            imshow( "Color view", imgBGRColor );
        }

        if ( option & IR ) {
            mIrGen.GetMetaData( irData );
            Mat imgIR( irData.FullYRes(), irData.FullXRes(), CV_16UC1, ( void * )irData.Data() );
            Mat img8bitIR;
            imgIR.convertTo( img8bitIR, CV_8U, 255.0 / 4096 );
            xnFPSMarkFrame(&xnIrFPS);
            sprintf_s(fpsstr, sizeof(fpsstr), "%.2f", xnFPSCalc(&xnIrFPS));
            putText(img8bitIR, string("FPS:") + fpsstr, Point(5, 20), FONT_HERSHEY_DUPLEX, 1.0, Scalar(200, 0, 0));
            imshow( "IR view", img8bitIR );
        }

        waitKey( 1 );
    }
    if ( option & DEPTH ) {
        mDepthGen.Release();
    }
    if ( option & IMAGE ) {
        mImageGen.Release();
    }
    if ( option & IR ) {
        mIrGen.Release();
    }
    mContext.Release();
    return 0;
}

