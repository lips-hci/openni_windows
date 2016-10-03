// NiRecorder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define RECORDING_FILE_NAME "recording.oni"
#define RECORDING_FRAME 100
#define RECORDING_WIDTH 640
#define RECORDING_HEIGHT 480
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

    ImageGenerator mImageGen;
    mImageGen.Create( mContext );
    mImageGen.SetMapOutputMode( mapMode );

    DepthGenerator mDepthGen;
    mDepthGen.Create( mContext );
    mDepthGen.SetMapOutputMode( mapMode );
    mDepthGen.GetAlternativeViewPointCap().SetViewPoint( mImageGen );

    Recorder mRecorder;
    mRecorder.Create( mContext );
    mRecorder.SetDestination( XN_RECORD_MEDIUM_FILE, RECORDING_FILE_NAME );
    mRecorder.AddNodeToRecording( mImageGen, XN_CODEC_JPEG );
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

