// NiSimpleViewer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;
using namespace xn;
using namespace cv;

#define FPG_AVG_COUNT 120

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

void showResolution ( XnMapOutputMode* mapMode ) {
    if ( XN_VGA_X_RES == mapMode->nXRes && XN_VGA_Y_RES == mapMode->nYRes ) {
        cout << "VGA ( 640 x 480 ), FPS = " << mapMode->nFPS << endl;
    } else if ( XN_QVGA_X_RES == mapMode->nXRes && XN_QVGA_Y_RES == mapMode->nYRes ) {
        cout << "QVGA ( 320 x 240 ), FPS = " << mapMode->nFPS << endl;
    } else if ( XN_QQVGA_X_RES == mapMode->nXRes && XN_QQVGA_Y_RES == mapMode->nYRes ) {
        cout << "QQVGA ( 160 x 120 ), FPS = " << mapMode->nFPS << endl;
    } else if ( 80 == mapMode->nXRes && 60 == mapMode->nYRes ) {
        cout << "QQQVGA ( 80 x 60 ), FPS = " << mapMode->nFPS << endl;
    } else if ( XN_1080P_X_RES == mapMode->nXRes && XN_1080P_Y_RES == mapMode->nYRes ) {
        cout << "1080P ( 1920 x 1080 ), FPS = " << mapMode->nFPS << endl;
    } else {
        cout << "Unknown ( " << mapMode->nXRes << " x " << mapMode->nYRes << "), FPS = " << mapMode->nFPS << endl;
    }
}

void getResolutionSetting( int query_option, DepthGenerator* depthData, ImageGenerator* imageData, IRGenerator* irData,
                          XnMapOutputMode* depthOutputMode, XnMapOutputMode* imageOutputMode, XnMapOutputMode* irOutputMode ) {
    unsigned int depthModeCount = 0;
    unsigned int imageModeCount = 0;
    unsigned int irModeCount = 0;
    XnMapOutputMode* depthModes;
    XnMapOutputMode* imageModes;
    XnMapOutputMode* irModes;
    XnStatus status;
    if ( query_option & DEPTH ) {
        depthModeCount = depthData->GetSupportedMapOutputModesCount();
        depthModes = new XnMapOutputMode[depthModeCount];
        status = depthData->GetSupportedMapOutputModes( depthModes, depthModeCount );
        if ( XN_STATUS_OK != status ) {
            cout << "[Depth] GetSupportedMapOutputModes fail" << endl;
        } else {
            unsigned int answer;
            unsigned int i;
            cout << endl << endl;
            cout << "Available options for DEPTH : " << endl;
            for ( i = 0; i < depthModeCount; i++ ) {
                cout << i + 1 << ") ";
                showResolution ( &depthModes[i] );
            }
            cout << ++i << ") Use SDK default " << endl;
            cout << "Please select resolution and FPS for Depth : ";
            cin >> answer;
            if ( answer > depthModeCount ) {
                answer = 1;
            }
            depthOutputMode->nXRes = depthModes[answer-1].nXRes;
            depthOutputMode->nYRes = depthModes[answer-1].nYRes;
            depthOutputMode->nFPS = depthModes[answer-1].nFPS;
        }
    }
    if ( query_option & IMAGE ) {
        imageModeCount = imageData->GetSupportedMapOutputModesCount();
        imageModes = new XnMapOutputMode[imageModeCount];
        status = imageData->GetSupportedMapOutputModes( imageModes, imageModeCount );
        if ( XN_STATUS_OK != status ) {
            cout << "[Image] GetSupportedMapOutputModes fail" << endl;
        } else {
            unsigned int answer;
            unsigned int i;
            cout << endl << endl;
            cout << "Available options for IMAGE : " << endl;
            for ( i = 0; i < imageModeCount; i++ ) {
                cout << i + 1 << ") ";
                showResolution ( &imageModes[i] );
            }
            cout << ++i << ") Use SDK default " << endl;
            cout << "Please select resolution and FPS for IMAGE : ";
            cin >> answer;
            if ( answer > imageModeCount ) {
                answer = 1;
            }
            imageOutputMode->nXRes = imageModes[answer-1].nXRes;
            imageOutputMode->nYRes = imageModes[answer-1].nYRes;
            imageOutputMode->nFPS = imageModes[answer-1].nFPS;
        }
    }
    if ( query_option & IR ) {
        irModeCount = irData->GetSupportedMapOutputModesCount();
        irModes = new XnMapOutputMode[irModeCount];
        status = irData->GetSupportedMapOutputModes( irModes, irModeCount );
        if ( XN_STATUS_OK != status ) {
            cout << "[IR] GetSupportedMapOutputModes fail" << endl;
        } else {
            unsigned int answer;
            unsigned int i;
            cout << endl << endl;
            cout << "Available options for IR : " << endl;
            for ( i = 0; i < irModeCount; i++ ) {
                cout << i + 1 << ") ";
                showResolution ( &irModes[i] );
            }
            cout << ++i << ") Use SDK default " << endl;
            cout << "Please select resolution and FPS for IR : ";
            cin >> answer;
            if ( answer > irModeCount ) {
                answer = 1;
            }
            irOutputMode->nXRes = irModes[answer-1].nXRes;
            irOutputMode->nYRes = irModes[answer-1].nYRes;
            irOutputMode->nFPS = irModes[answer-1].nFPS;
        }
    }
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

    XnMapOutputMode mapDepthOutputMode;
    XnMapOutputMode mapImageOutputMode;
    XnMapOutputMode mapIrOutputMode;

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

    getResolutionSetting( option, &mDepthGen, &mImageGen, &mIrGen, &mapDepthOutputMode, &mapImageOutputMode, &mapIrOutputMode);

    if ( option & DEPTH ) {
        mDepthGen.SetMapOutputMode( mapDepthOutputMode );
    }
    if ( option & IMAGE ) {
        mImageGen.SetMapOutputMode( mapImageOutputMode );
    }
    if ( option & IR ) {
        mImageGen.SetMapOutputMode( mapIrOutputMode );
    }

    mContext.StartGeneratingAll();

    char fpsstr[7];
    char xyzstr[128];
    double tStart = 0;
    bool quit = false;
    bool capture = false;
    bool showText = true;
    vector<int> quality;
    quality.push_back(CV_IMWRITE_PNG_COMPRESSION);
    quality.push_back(0);
    while ( true )
    {
        tStart = (double)getTickCount();
        mContext.WaitAndUpdateAll();

        if ( option & DEPTH ) {
            mDepthGen.GetMetaData( depthData );
            Mat imgDepth( depthData.FullYRes(), depthData.FullXRes(), CV_16UC1, ( void* )depthData.Data() );
            Mat img8bitDepth;
            Mat img8bit3ChDepth;
            Mat img8bit3ChMask = Mat( depthData.FullYRes(), depthData.FullXRes(), CV_8UC3, Scalar(0, 255, 255) );
            imgDepth.convertTo( img8bitDepth, CV_8U, 255.0 / 10000 );
            cvtColor( img8bitDepth, img8bit3ChDepth, CV_GRAY2BGR );
            bitwise_and( img8bit3ChDepth, img8bit3ChMask, img8bit3ChDepth );
            xnFPSMarkFrame(&xnDepthFPS);
            sprintf_s(fpsstr, sizeof(fpsstr), "%.2f", xnFPSCalc(&xnDepthFPS));
            if ( showText ) {
                putText(img8bit3ChDepth, string("FPS:") + fpsstr, Point(5, 20), FONT_HERSHEY_DUPLEX, (depthData.FullXRes()>320)?1.0:0.5, Scalar(255, 255, 255));
            }
            Zres = depthData(Xres, Yres);
            sprintf_s(xyzstr, sizeof(xyzstr), "X : %d, Y : %d, Depth : %u", Xres, Yres, Zres);
            if ( showText ) {
                putText(img8bit3ChDepth, xyzstr, Point(5, 50), FONT_HERSHEY_DUPLEX, (depthData.FullXRes()>320)?1.0:0.5, Scalar(255, 255, 255));
            }
            imshow( "Depth view", img8bit3ChDepth );
            if ( capture ) {
                imwrite( "depth_" + std::to_string(depthData.FrameID()) + ".png", img8bit3ChDepth, quality );
            }
            cv::setMouseCallback( "Depth view", onMouse, NULL );
        }

        if ( option & IMAGE ) {
            mImageGen.GetMetaData( colorData );
            Mat imgColor( colorData.FullYRes(), colorData.FullXRes(), CV_8UC3, ( void* )colorData.Data() );
            Mat imgBGRColor;
            cvtColor( imgColor, imgBGRColor, CV_RGB2BGR );
            xnFPSMarkFrame(&xnColorFPS);
            sprintf_s(fpsstr, sizeof(fpsstr), "%.2f", xnFPSCalc(&xnColorFPS));
            if ( showText ) {
                putText(imgBGRColor, string("FPS:") + fpsstr, Point(5, 20), FONT_HERSHEY_DUPLEX, (colorData.FullXRes()>320)?1.0:0.5, Scalar(200, 0, 0));
            }
            imshow( "Color view", imgBGRColor );
            if ( capture ) {
                imwrite( "image_" + std::to_string(colorData.FrameID()) + ".png", imgBGRColor, quality );
            }
        }

        if ( option & IR ) {
            mIrGen.GetMetaData( irData );
            Mat imgIR( irData.FullYRes(), irData.FullXRes(), CV_16UC1, ( void * )irData.Data() );
            Mat img8bitIR;
            imgIR.convertTo( img8bitIR, CV_8U, 255.0 / 4096 );
            xnFPSMarkFrame(&xnIrFPS);
            sprintf_s(fpsstr, sizeof(fpsstr), "%.2f", xnFPSCalc(&xnIrFPS));
            if ( showText ) {
                putText(img8bitIR, string("FPS:") + fpsstr, Point(5, 20), FONT_HERSHEY_DUPLEX, (irData.FullXRes()>320)?1.0:0.5, Scalar(200, 0, 0));
            }
            imshow( "IR view", img8bitIR );
            if ( capture ) {
                imwrite( "ir_" + std::to_string(irData.FrameID()) + ".png", img8bitIR, quality );
            }
        }

        int keyInput = waitKey( 1 );
        if ( keyInput != -1 ) {
            switch ( keyInput ) {
            case 'Q': // Q = 81
            case 'q': // q = 113
                //q for exit
                quit = true;
                break;
            case 'C': // C = 67
            case 'c': // c = 99
                // depth
                capture = true;
                break;
            case 'F': // F = 70
            case 'f': // f = 102
                showText = (showText)?false:true;
                break;
            default:
                break;
            }
        } else {
            capture = false;
        }
        if ( quit ) {
            break;
        }
    }
    mContext.StopGeneratingAll();
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

