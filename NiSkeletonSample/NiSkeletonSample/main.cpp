//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnOpenNI.h>
#include <XnCppWrapper.h>
#include <XnFPSCalculator.h>
#include <iostream>
#include <tchar.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define DEBUG         1
#define FPG_AVG_COUNT 60
#define MAX_NUM_USERS 4
#define MAX_NUM_SKELETONS 15

#define CHECK_RC(nRetVal, what)                                     \
    if (nRetVal != XN_STATUS_OK)                                    \
    {                                                                   \
        printf("%s failed: %s\n", what, xnGetStatusString(nRetVal));    \
        return nRetVal;                                                 \
    }
//---------------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------------
using namespace xn;
using namespace cv;

UserGenerator g_UserGenerator;
DepthGenerator g_DepthGenerator;

XnBool g_bNeedPose = FALSE;
XnChar g_strPose[20] = "";
bool showDepthImg = false;
bool showFPS = false;

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
// Callback: New user was detected
void XN_CALLBACK_TYPE User_NewUser( xn::UserGenerator& /*generator*/, XnUserID nId, void* /*pCookie*/ )
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime( &epochTime );
    printf( "%d New User %d\n", epochTime, nId );
    // New user found
    if ( g_bNeedPose )
    {
        g_UserGenerator.GetPoseDetectionCap().StartPoseDetection( g_strPose, nId );
    }
    else
    {
        g_UserGenerator.GetSkeletonCap().RequestCalibration( nId, TRUE );
    }
}

// Callback: An existing user was lost
void XN_CALLBACK_TYPE User_LostUser( xn::UserGenerator& /*generator*/, XnUserID nId, void* /*pCookie*/ )
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime( &epochTime );
    printf( "%d Lost user %d\n", epochTime, nId );
}

// Callback: Detected a pose
void XN_CALLBACK_TYPE UserPose_PoseDetected( xn::PoseDetectionCapability& /*capability*/, const XnChar* strPose, XnUserID nId, void* /*pCookie*/ )
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime( &epochTime );
    printf( "%d Pose %s detected for user %d\n", epochTime, strPose, nId );
    g_UserGenerator.GetPoseDetectionCap().StopPoseDetection( nId );
    g_UserGenerator.GetSkeletonCap().RequestCalibration( nId, TRUE );
}
// Callback: Started calibration
void XN_CALLBACK_TYPE UserCalibration_CalibrationStart( xn::SkeletonCapability& /*capability*/, XnUserID nId, void* /*pCookie*/ )
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime( &epochTime );
    printf( "%d Calibration started for user %d\n", epochTime, nId );
}
// Callback: check calibration success or failed
void XN_CALLBACK_TYPE UserCalibration_CalibrationComplete( xn::SkeletonCapability& /*capability*/, XnUserID nId, XnCalibrationStatus eStatus, void* /*pCookie*/ )
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime( &epochTime );
    if ( eStatus == XN_CALIBRATION_STATUS_OK )
    {
        // Calibration succeeded
        printf( "%d Calibration complete, start tracking user %d\n", epochTime, nId );
        g_UserGenerator.GetSkeletonCap().StartTracking( nId );
    }
    else
    {
        // Calibration failed
        printf( "%d Calibration failed for user %d\n", epochTime, nId );
        if ( eStatus == XN_CALIBRATION_STATUS_MANUAL_ABORT )
        {
            printf( "Manual abort occured, stop attempting to calibrate!" );
            return;
        }
        if ( g_bNeedPose )
        {
            g_UserGenerator.GetPoseDetectionCap().StartPoseDetection( g_strPose, nId );
        }
        else
        {
            g_UserGenerator.GetSkeletonCap().RequestCalibration( nId, TRUE );
        }
    }
}

int handleKeyEvent( char keyEvent )
{
    int value = 0;
    if ( keyEvent == -1 || keyEvent == 255 )
    {
        return value;
    }
#if DEBUG
    std::cout << "Receive key event: " << keyEvent << std::endl;
#endif
    switch ( keyEvent )
    {
        case 27:            //esc key
            value = -1;
            break;
        case 'd':
        case 'D':
            showDepthImg = !showDepthImg;
            if ( !showDepthImg )
            {
                destroyWindow( "Depth View" );
            }
            break;
        case 'f':
        case 'F':
            showFPS = !showFPS;
        default:
            break;
    }
    return value;
}

int main()
{
    XnStatus nRetVal = XN_STATUS_OK;
    EnumerationErrors errors;

    Context mContext;
    nRetVal = mContext.Init();

    if ( nRetVal != XN_STATUS_OK )
    {
        printf( "mContext init failed: %s\n", xnGetStatusString( nRetVal ) );
        return ( nRetVal );
    }

    nRetVal = g_DepthGenerator.Create( mContext );
    CHECK_RC( nRetVal, "Find user generator" );
    if ( nRetVal != XN_STATUS_OK )
    {
        printf( "Create g_DepthGenerator failed: %s\n", xnGetStatusString( nRetVal ) );
        mContext.Release();
        return 1;
    }
    //Init counting fps
    XnFPSData xnDepthFPS;
    xnFPSInit( &xnDepthFPS, FPG_AVG_COUNT );
    //Set depth generator output mode;
    XnMapOutputMode mapMode;
    mapMode.nXRes = 640;
    mapMode.nYRes = 480;
    mapMode.nFPS = 30;
    g_DepthGenerator.SetMapOutputMode( mapMode );

    nRetVal = g_UserGenerator.Create( mContext );
    CHECK_RC( nRetVal, "Find user generator" );
    if ( nRetVal != XN_STATUS_OK )
    {
        printf( "Create UserGenerator failed: %s\n", xnGetStatusString( nRetVal ) );
        mContext.Release();
        g_DepthGenerator.Release();
        return 1;
    }

    XnCallbackHandle hUserCallbacks, hCalibrationStart, hCalibrationComplete, hPoseDetected;
    if ( !g_UserGenerator.IsCapabilitySupported( XN_CAPABILITY_SKELETON ) )
    {
        printf( "Supplied user generator doesn't support skeleton\n" );
        g_DepthGenerator.Release();
        g_UserGenerator.Release();
        mContext.Release();
        return 1;
    }
    nRetVal = g_UserGenerator.RegisterUserCallbacks( User_NewUser, User_LostUser, NULL, hUserCallbacks );
    CHECK_RC( nRetVal, "Register to user callbacks" );
    nRetVal = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationStart( UserCalibration_CalibrationStart, NULL, hCalibrationStart );
    CHECK_RC( nRetVal, "Register to calibration start" );
    nRetVal = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationComplete( UserCalibration_CalibrationComplete, NULL, hCalibrationComplete );
    CHECK_RC( nRetVal, "Register to calibration complete" );

    if ( g_UserGenerator.GetSkeletonCap().NeedPoseForCalibration() )
    {
        g_bNeedPose = TRUE;
        if ( !g_UserGenerator.IsCapabilitySupported( XN_CAPABILITY_POSE_DETECTION ) )
        {
            printf( "Pose required, but not supported\n" );
            g_DepthGenerator.Release();
            g_UserGenerator.Release();
            mContext.Release();
            return 1;
        }
        nRetVal = g_UserGenerator.GetPoseDetectionCap().RegisterToPoseDetected( UserPose_PoseDetected, NULL, hPoseDetected );
        CHECK_RC( nRetVal, "Register to Pose Detected" );
        g_UserGenerator.GetSkeletonCap().GetCalibrationPose( g_strPose );
    }

    g_UserGenerator.GetSkeletonCap().SetSkeletonProfile( XN_SKEL_PROFILE_ALL );

    nRetVal = mContext.StartGeneratingAll();
    CHECK_RC( nRetVal, "StartGenerating" );

    XnUserID aUsers[MAX_NUM_USERS];
    XnUInt16 nUsers;
    XnSkeletonJointTransformation skeletonJoint;

    printf( "Starting to run\n" );
    if ( g_bNeedPose )
    {
        printf( "Assume calibration pose\n" );
    }

    int arr_skeleton_part[MAX_NUM_SKELETONS] =
    {
        XN_SKEL_HEAD,           //0
        XN_SKEL_NECK,           //1
        XN_SKEL_TORSO,          //2
        XN_SKEL_LEFT_SHOULDER,  //3
        XN_SKEL_RIGHT_SHOULDER, //4
        XN_SKEL_LEFT_ELBOW,     //5
        XN_SKEL_RIGHT_ELBOW,    //6
        XN_SKEL_LEFT_HAND,      //7
        XN_SKEL_RIGHT_HAND,     //8
        XN_SKEL_LEFT_HIP,       //9
        XN_SKEL_RIGHT_HIP,      //10
        XN_SKEL_LEFT_KNEE,      //11
        XN_SKEL_RIGHT_KNEE,     //12
        XN_SKEL_LEFT_FOOT,      //13
        XN_SKEL_RIGHT_FOOT      //14
    };
    char *str_skeleton_name[] = {"Head", "Neck", "Torso", "Left shoulder", "Right shoulder", "Left Elbow", "Right Elbow",
                                 "Left hand", "Right Hand", "Left hip", "Right hip", "Left knee", "Right, knee",
                                 "Left foot", "Right foot"
                                };
    while ( true )
    {
        mContext.WaitOneUpdateAll( g_UserGenerator );
        // print the skeleton information for the first user already tracking

        DepthMetaData depthData;
        g_DepthGenerator.GetMetaData( depthData );
        Mat imgDepth( depthData.FullYRes(), depthData.FullXRes(), CV_16UC1, ( void* )depthData.Data() );
        Mat img8bitDepth;
        Mat imgSkeleton ( depthData.FullYRes(), depthData.FullXRes(), CV_8UC3, Scalar( 0, 0, 0 ) );
        imgDepth.convertTo( img8bitDepth, CV_8U, 255.0 / 5000 );
        double _fps = xnFPSCalc( &xnDepthFPS );
        xnFPSMarkFrame( &xnDepthFPS );

        nUsers = MAX_NUM_USERS;
        g_UserGenerator.GetUsers( aUsers, nUsers );
        for ( XnUInt16 i = 0; i < nUsers; i++ )
        {
            if ( g_UserGenerator.GetSkeletonCap().IsTracking( aUsers[i] ) == FALSE )
            {
                continue;
            }
            // Print Head  coordinate
            XnPoint3D skeletonPoint[MAX_NUM_SKELETONS] = {-1};
            for ( int s = 0; s < MAX_NUM_SKELETONS ; s++ )
            {
                g_UserGenerator.GetSkeletonCap().GetSkeletonJoint( aUsers[i], ( XnSkeletonJoint ) arr_skeleton_part[s], skeletonJoint );
                if ( skeletonJoint.position.fConfidence >=  0.5f || skeletonJoint.orientation.fConfidence >= 0.5f )
                {
                    XnPoint3D pt = skeletonJoint.position.position;
                    g_DepthGenerator.ConvertRealWorldToProjective( 1, &pt, &pt );
                    skeletonPoint[s] = pt;
                    printf( "user %d: %s at (%6.2f,%6.2f,%6.2f)\n", aUsers[i], str_skeleton_name[s], pt.X, pt.Y, pt.Z );
                    circle( img8bitDepth, Point2f( pt.X, pt.Y ), 10, Scalar( 200, 150, 150 ) );
                    circle( imgSkeleton, Point2f( pt.X, pt.Y ), 10, Scalar( 200, 150, 150 ) );
                }
            }
            Scalar color( 100, 50, 200 );
            if ( skeletonPoint[0].X != -1 )
            {
                line( imgSkeleton, Point( skeletonPoint[0].X, skeletonPoint[0].Y ),  Point( skeletonPoint[1].X, skeletonPoint[1].Y ), color, 2, 8, 0 );
                line( imgSkeleton, Point( skeletonPoint[1].X, skeletonPoint[1].Y ),  Point( skeletonPoint[2].X, skeletonPoint[2].Y ), color, 2, 8, 0 );
                line( imgSkeleton, Point( skeletonPoint[3].X, skeletonPoint[3].Y ),  Point( skeletonPoint[4].X, skeletonPoint[4].Y ), color, 2, 8, 0 );
                line( imgSkeleton, Point( skeletonPoint[3].X, skeletonPoint[3].Y ),  Point( skeletonPoint[5].X, skeletonPoint[5].Y ), color, 2, 8, 0 );
                line( imgSkeleton, Point( skeletonPoint[5].X, skeletonPoint[5].Y ),  Point( skeletonPoint[7].X, skeletonPoint[7].Y ), color, 2, 8, 0 );
                line( imgSkeleton, Point( skeletonPoint[4].X, skeletonPoint[4].Y ),  Point( skeletonPoint[6].X, skeletonPoint[6].Y ), color, 2, 8, 0 );
                line( imgSkeleton, Point( skeletonPoint[6].X, skeletonPoint[6].Y ),  Point( skeletonPoint[8].X, skeletonPoint[8].Y ), color, 2, 8, 0 );

                line( imgSkeleton, Point( skeletonPoint[2].X, skeletonPoint[2].Y ),  Point( skeletonPoint[9].X, skeletonPoint[9].Y ), color, 2, 8, 0 );
                line( imgSkeleton, Point( skeletonPoint[2].X, skeletonPoint[2].Y ),  Point( skeletonPoint[10].X, skeletonPoint[10].Y ), color, 2, 8, 0 );
                line( imgSkeleton, Point( skeletonPoint[9].X, skeletonPoint[9].Y ),  Point( skeletonPoint[11].X, skeletonPoint[11].Y ), color, 2, 8, 0 );
                line( imgSkeleton, Point( skeletonPoint[10].X, skeletonPoint[10].Y ),  Point( skeletonPoint[12].X, skeletonPoint[12].Y ), color, 2, 8, 0 );
                line( imgSkeleton, Point( skeletonPoint[11].X, skeletonPoint[11].Y ),  Point( skeletonPoint[13].X, skeletonPoint[13].Y ), color, 2, 8, 0 );
                line( imgSkeleton, Point( skeletonPoint[12].X, skeletonPoint[12].Y ),  Point( skeletonPoint[14].X, skeletonPoint[14].Y ), color, 2, 8, 0 );
            }
        }
        if ( showFPS )
        {
            string fps = std::to_string ( _fps );
            cv::putText( img8bitDepth, string( "FPS:" ) + fps, Point( 5, 20 ), FONT_ITALIC, 0.5, Scalar( 200, 0, 0 ), 1, 7, false );
            cv::putText( imgSkeleton, string( "FPS:" ) + fps, Point( 5, 20 ), FONT_ITALIC, 0.5, Scalar( 200, 0, 0 ), 1, 7, false );
        }

        if ( showDepthImg )
        {
            cv::imshow( "Depth View", img8bitDepth );
        }
        cv::imshow( "Skeleton", imgSkeleton );
        if ( handleKeyEvent( waitKey( 1 ) )  == -1 )
        {
            break;
        }
    }
    g_DepthGenerator.Release();
    g_UserGenerator.Release();
    mContext.Release();
    return 0;
}