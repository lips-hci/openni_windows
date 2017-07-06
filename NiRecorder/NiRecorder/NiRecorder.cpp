// NiRecorder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

//#define DEVICE_M3

#define RECORDING_FILE_NAME "recording.oni"
#define RECORDING_FRAME 100
#ifdef DEVICE_M3
#define RECORDING_WIDTH 80
#define RECORDING_HEIGHT 60
#else
#define RECORDING_WIDTH 640
#define RECORDING_HEIGHT 480
#endif
#define RECORDING_FPS 30

using namespace std;
using namespace xn;

int _tmain(int argc, _TCHAR* argv[])
{
    Context mContext;
    mContext.Init();

    XnMapOutputMode mapMode;
    mapMode.nXRes = RECORDING_WIDTH;
    mapMode.nYRes = RECORDING_HEIGHT;
    mapMode.nFPS = RECORDING_FPS;

#ifndef DEVICE_M3
    ImageGenerator mImageGen;
    mImageGen.Create( mContext );
    mImageGen.SetMapOutputMode( mapMode );
#endif

    DepthGenerator mDepthGen;
    mDepthGen.Create( mContext );
    mDepthGen.SetMapOutputMode( mapMode );
#ifndef DEVICE_M3
    mDepthGen.GetAlternativeViewPointCap().SetViewPoint( mImageGen );
#endif

    Recorder mRecorder;
    mRecorder.Create( mContext );
    mRecorder.SetDestination( XN_RECORD_MEDIUM_FILE, RECORDING_FILE_NAME );
#ifndef DEVICE_M3
    mRecorder.AddNodeToRecording( mImageGen, XN_CODEC_JPEG );
#endif
    mRecorder.AddNodeToRecording( mDepthGen, XN_CODEC_16Z_EMB_TABLES );

    mContext.StartGeneratingAll();

    unsigned int i = 0;
    while ( true )
    {
        if ( ++i > RECORDING_FRAME )
        {
            break;
        }
        cout << i << endl;
        mContext.WaitAndUpdateAll();
    }

    mRecorder.Release();
    mContext.StopGeneratingAll();

    mContext.Release();
    return 0;
}

