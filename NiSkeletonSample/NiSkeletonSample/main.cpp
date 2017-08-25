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
#define DEBUG         0
#define FPG_AVG_COUNT 60
#define MAX_NUM_USERS 4
#define MAX_NUM_SKELETONS 24
#define CONFIDENCE_THRESHOLD 0.5
#define SMOOTHING 0.8

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


struct joint_s {
    XnPoint3D point;
    XnConfidence confidence;
};
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

void addLine( Mat imgSkeleton, joint_s *skeletonPoint, int start, int end, XnFloat confidence_threshold )
{
    start--;
    end--;
    if ( skeletonPoint[start].point.X <= 0 || skeletonPoint[start].point.Y <= 0 || skeletonPoint[end].point.X <= 0 || skeletonPoint[end].point.Y <= 0 ) {
        return;
    } else {
        if ( skeletonPoint[start].confidence < confidence_threshold || skeletonPoint[end].confidence < confidence_threshold) {
            line( imgSkeleton, Point( skeletonPoint[start].point.X, skeletonPoint[start].point.Y ), Point( skeletonPoint[end].point.X, skeletonPoint[end].point.Y ), Scalar( 150, 200, 150 ), 1, CV_AA, 0 );
        } else {
            line( imgSkeleton, Point( skeletonPoint[start].point.X, skeletonPoint[start].point.Y ), Point( skeletonPoint[end].point.X, skeletonPoint[end].point.Y ), Scalar( 150, 200, 150 ), 4, CV_AA, 0 );
        }
    }
    return;
}

int main( int argc, char *argv[] )
{
    XnStatus nRetVal = XN_STATUS_OK;
    EnumerationErrors errors;
    XnFloat confidence = CONFIDENCE_THRESHOLD;
    XnFloat smoothing = SMOOTHING;
    if ( 2 == argc ) {
        // confidence, no smoothing
        confidence = atof( argv[1] );
    } else if ( 3 == argc ) {
        // confidence, smoothing
        confidence = atof( argv[1] );
        smoothing = atof( argv[2] );
    }

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
    g_UserGenerator.GetSkeletonCap().SetSmoothing(smoothing);

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

    char *str_skeleton_name[] = {
        "HEAD", "NECK", "TORSO", "WAIST",
        "LEFT_COLLAR", "LEFT_SHOULDER", "LEFT_ELBOW", "LEFT_WRIST", "LEFT_HAND", "LEFT_FINGERTIP",
        "RIGHT_COLLAR", "RIGHT_SHOULDER", "RIGHT_ELBOW", "RIGHT_WRIST", "RIGHT_HAND", "RIGHT_FINGERTIP",
        "LEFT_HIP", "LEFT_KNEE", "LEFT_ANKLE", "LEFT_FOOT",
        "RIGHT_HIP", "RIGHT_KNEE", "RIGHT_ANKLE", "RIGHT_FOOT" };
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
            XnUInt16 nJoints = 24;
            XnSkeletonJoint pJoints[24];
            g_UserGenerator.GetSkeletonCap().EnumerateActiveJoints( pJoints, nJoints );
            joint_s skeletonPoint[MAX_NUM_SKELETONS] = { -1, 0 };
            for ( int s = 0; s < nJoints ; s++ )
            {
                g_UserGenerator.GetSkeletonCap().GetSkeletonJoint( aUsers[i], ( XnSkeletonJoint ) pJoints[s], skeletonJoint );
                {
                    XnPoint3D pt = skeletonJoint.position.position;
                    g_DepthGenerator.ConvertRealWorldToProjective( 1, &pt, &pt );
                    skeletonPoint[s].point = pt;
                    skeletonPoint[s].confidence = skeletonJoint.position.fConfidence;
#if DEBUG
                    printf( "user %d: %s at (%6.2f,%6.2f,%6.2f)\n", aUsers[i], str_skeleton_name[s], pt.X, pt.Y, pt.Z );
#endif
                    circle( img8bitDepth, Point2f( pt.X, pt.Y ), 10, Scalar( 200, 150, 150 ), CV_FILLED, CV_AA );
                    circle( imgSkeleton, Point2f( pt.X, pt.Y ), 5, Scalar( 150, 200, 150 ), CV_FILLED, CV_AA );
#if DEBUG
                    cv::putText( img8bitDepth, str_skeleton_name[s], Point( pt.X, pt.Y ), FONT_ITALIC, 0.4, Scalar( 0, 0, 0 ), 1, 7, false );
                    cv::putText( imgSkeleton, str_skeleton_name[s], Point( pt.X, pt.Y ), FONT_ITALIC, 0.4, Scalar( 100, 100, 100 ), 1, 7, false );
#endif
                }
            }

            addLine( imgSkeleton, skeletonPoint, XN_SKEL_HEAD, XN_SKEL_NECK, confidence );
            addLine( imgSkeleton, skeletonPoint, XN_SKEL_NECK, XN_SKEL_TORSO, confidence );
            addLine( imgSkeleton, skeletonPoint, XN_SKEL_TORSO, XN_SKEL_WAIST, confidence );

            addLine( imgSkeleton, skeletonPoint, XN_SKEL_TORSO, XN_SKEL_LEFT_COLLAR, confidence );
            addLine( imgSkeleton, skeletonPoint, XN_SKEL_LEFT_COLLAR, XN_SKEL_LEFT_SHOULDER, confidence );
            addLine( imgSkeleton, skeletonPoint, XN_SKEL_LEFT_SHOULDER, XN_SKEL_LEFT_ELBOW, confidence );
            addLine( imgSkeleton, skeletonPoint, XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_WRIST, confidence );
            addLine( imgSkeleton, skeletonPoint, XN_SKEL_LEFT_WRIST, XN_SKEL_LEFT_HAND, confidence );
            //addLine( imgSkeleton, skeletonPoint, XN_SKEL_LEFT_HAND, XN_SKEL_LEFT_FINGERTIP, confidence );

            addLine( imgSkeleton, skeletonPoint, XN_SKEL_TORSO, XN_SKEL_RIGHT_COLLAR, confidence );
            addLine( imgSkeleton, skeletonPoint, XN_SKEL_RIGHT_COLLAR, XN_SKEL_RIGHT_SHOULDER, confidence );
            addLine( imgSkeleton, skeletonPoint, XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW, confidence );
            addLine( imgSkeleton, skeletonPoint, XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_WRIST, confidence );
            addLine( imgSkeleton, skeletonPoint, XN_SKEL_RIGHT_WRIST, XN_SKEL_RIGHT_HAND, confidence );
            //addLine( imgSkeleton, skeletonPoint, XN_SKEL_RIGHT_HAND, XN_SKEL_RIGHT_FINGERTIP, confidence );

            addLine( imgSkeleton, skeletonPoint, XN_SKEL_WAIST, XN_SKEL_LEFT_HIP, confidence );
            addLine( imgSkeleton, skeletonPoint, XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE, confidence );
            addLine( imgSkeleton, skeletonPoint, XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_ANKLE, confidence );
            addLine( imgSkeleton, skeletonPoint, XN_SKEL_LEFT_ANKLE, XN_SKEL_LEFT_FOOT, confidence );

            addLine( imgSkeleton, skeletonPoint, XN_SKEL_WAIST, XN_SKEL_RIGHT_HIP, confidence );
            addLine( imgSkeleton, skeletonPoint, XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE, confidence );
            addLine( imgSkeleton, skeletonPoint, XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_ANKLE, confidence );
            addLine( imgSkeleton, skeletonPoint, XN_SKEL_RIGHT_ANKLE, XN_SKEL_RIGHT_FOOT, confidence );
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