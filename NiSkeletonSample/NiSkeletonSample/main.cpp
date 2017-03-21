//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnCppWrapper.h>
#include <opencv2/core/core.hpp>
#include <tchar.h>

#include <XnOpenNI.h>
#include <XnCppWrapper.h>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define SAMPLE_XML_PATH "../../../../Data/SamplesConfig.xml"
#define SAMPLE_XML_PATH_LOCAL "SamplesConfig.xml"

//---------------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------------

xn::UserGenerator g_UserGenerator;
xn::DepthGenerator g_DepthGenerator;

XnBool g_bNeedPose = FALSE;
XnChar g_strPose[20] = "";

#define MAX_NUM_USERS 1
#define MAX_NUM_SKELETONS 15
//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

//Check config file exists
XnBool fileExists(const char *fn)
{
	XnBool exists;
	xnOSDoesFileExist(fn, &exists);
	return exists;
}

// Callback: New user was detected
void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& /*generator*/, XnUserID nId, void* /*pCookie*/)
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    printf("%d New User %d\n", epochTime, nId);
    // New user found
    if (g_bNeedPose)
    {
        g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
    }
    else
    {
        g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
    }
}

// Callback: An existing user was lost
void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& /*generator*/, XnUserID nId, void* /*pCookie*/)
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    printf("%d Lost user %d\n", epochTime, nId);
}

// Callback: Detected a pose
void XN_CALLBACK_TYPE UserPose_PoseDetected(xn::PoseDetectionCapability& /*capability*/, const XnChar* strPose, XnUserID nId, void* /*pCookie*/)
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    printf("%d Pose %s detected for user %d\n", epochTime, strPose, nId);
    g_UserGenerator.GetPoseDetectionCap().StopPoseDetection(nId);
    g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}
// Callback: Started calibration
void XN_CALLBACK_TYPE UserCalibration_CalibrationStart(xn::SkeletonCapability& /*capability*/, XnUserID nId, void* /*pCookie*/)
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    printf("%d Calibration started for user %d\n", epochTime, nId);
}
// Callback: check calibration success or failed
void XN_CALLBACK_TYPE UserCalibration_CalibrationComplete(xn::SkeletonCapability& /*capability*/, XnUserID nId, XnCalibrationStatus eStatus, void* /*pCookie*/)
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    if (eStatus == XN_CALIBRATION_STATUS_OK)
    {
        // Calibration succeeded
        printf("%d Calibration complete, start tracking user %d\n", epochTime, nId);
        g_UserGenerator.GetSkeletonCap().StartTracking(nId);
    }
    else
    {
        // Calibration failed
        printf("%d Calibration failed for user %d\n", epochTime, nId);
        if(eStatus == XN_CALIBRATION_STATUS_MANUAL_ABORT)
        {
            printf("Manual abort occured, stop attempting to calibrate!");
            return;
        }
        if (g_bNeedPose)
        {
            g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
        }
        else
        {
            g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
        }
    }
}



#define CHECK_RC(nRetVal, what)					    \
    if (nRetVal != XN_STATUS_OK)				    \
{								                    \
    printf("%s failed: %s\n", what, xnGetStatusString(nRetVal));    \
    return nRetVal;						    \
}
using namespace xn;
using namespace cv;
int main()
{
    XnStatus nRetVal = XN_STATUS_OK;
    EnumerationErrors errors;

    Context mContext;
    nRetVal = mContext.Init();

    if (nRetVal != XN_STATUS_OK)
    {
        printf("mContext init failed: %s\n", xnGetStatusString(nRetVal));
        return (nRetVal);
    }

    nRetVal = g_DepthGenerator.Create(mContext);
    CHECK_RC(nRetVal, "Find user generator");
    if (nRetVal != XN_STATUS_OK)
    {
        printf("Create g_DepthGenerator failed: %s\n", xnGetStatusString(nRetVal));
        return 1;
    }
    //Set depth generator output mode;
    XnMapOutputMode mapMode;
    mapMode.nXRes = 640;
    mapMode.nYRes = 480;
    mapMode.nFPS = 30;
    g_DepthGenerator.SetMapOutputMode(mapMode);

    nRetVal = g_UserGenerator.Create(mContext);
    CHECK_RC(nRetVal, "Find user generator");
    if (nRetVal != XN_STATUS_OK)
    {
        printf("Create UserGenerator failed: %s\n", xnGetStatusString(nRetVal));
        return 1;
    }

    XnCallbackHandle hUserCallbacks, hCalibrationStart, hCalibrationComplete, hPoseDetected;
    if (!g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
    {
        printf("Supplied user generator doesn't support skeleton\n");
        return 1;
    }
    nRetVal = g_UserGenerator.RegisterUserCallbacks(User_NewUser, User_LostUser, NULL, hUserCallbacks);
    CHECK_RC(nRetVal, "Register to user callbacks");
    nRetVal = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationStart(UserCalibration_CalibrationStart, NULL, hCalibrationStart);
    CHECK_RC(nRetVal, "Register to calibration start");
    nRetVal = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationComplete(UserCalibration_CalibrationComplete, NULL, hCalibrationComplete);
    CHECK_RC(nRetVal, "Register to calibration complete");

    if (g_UserGenerator.GetSkeletonCap().NeedPoseForCalibration())
    {
        g_bNeedPose = TRUE;
        if (!g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
        {
            printf("Pose required, but not supported\n");
            return 1;
        }
        nRetVal = g_UserGenerator.GetPoseDetectionCap().RegisterToPoseDetected(UserPose_PoseDetected, NULL, hPoseDetected);
        CHECK_RC(nRetVal, "Register to Pose Detected");
        g_UserGenerator.GetSkeletonCap().GetCalibrationPose(g_strPose);
    }

    g_UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

    nRetVal = mContext.StartGeneratingAll();
    CHECK_RC(nRetVal, "StartGenerating");

    XnUserID aUsers[MAX_NUM_USERS];
    XnUInt16 nUsers;
    XnSkeletonJointTransformation skeletonJoint;

    printf("Starting to run\n");
    if(g_bNeedPose)
    {
        printf("Assume calibration pose\n");
    }
    char fpsstr[7];
    double tStart = 0;

    int arr_skeleton_part[MAX_NUM_SKELETONS] = { XN_SKEL_HEAD,XN_SKEL_NECK, XN_SKEL_TORSO,
                                            XN_SKEL_LEFT_SHOULDER, XN_SKEL_RIGHT_SHOULDER, XN_SKEL_LEFT_ELBOW, XN_SKEL_RIGHT_ELBOW,
                                            XN_SKEL_LEFT_HAND, XN_SKEL_RIGHT_HAND, XN_SKEL_LEFT_HIP,
                                            XN_SKEL_RIGHT_HIP, XN_SKEL_LEFT_KNEE, XN_SKEL_RIGHT_KNEE,
                                            XN_SKEL_LEFT_FOOT, XN_SKEL_RIGHT_FOOT};
    char *str_skeleton_name[] = {"Head", "Neck", "Torso", "Left shoulder", "Right shoulder", "Left Elbow", "Right Elbow",
                   "Left hand", "Right Hand", "Left hip", "Right hip", "Left knee", "Right, knee",
                   "Left foot", "Right foot"}; 
	while (!xnOSWasKeyboardHit())
    {
        mContext.WaitOneUpdateAll(g_UserGenerator);
        tStart = (double)getTickCount();
        // print the skeleton information for the first user already tracking

        DepthMetaData depthData;
        g_DepthGenerator.GetMetaData( depthData );
        Mat imgDepth( depthData.FullYRes(), depthData.FullXRes(), CV_16UC1, ( void* )depthData.Data() );
        Mat img8bitDepth;
        imgDepth.convertTo( img8bitDepth, CV_8U, 255.0 / 5000 );
        sprintf_s(fpsstr, sizeof(fpsstr), "%.2f", 1.0 / (((double)getTickCount() - tStart) / getTickFrequency()));
        putText(img8bitDepth, string("FPS:") + fpsstr, Point(5, 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(200, 0, 0));

        nUsers = MAX_NUM_USERS;
        g_UserGenerator.GetUsers(aUsers, nUsers);
        for(XnUInt16 i = 0; i < nUsers; i++)
        {
            if(g_UserGenerator.GetSkeletonCap().IsTracking(aUsers[i]) == FALSE)
                continue;

            // Print Head  coordinate
            for( int s = 0; s < MAX_NUM_SKELETONS ; s++)
            {
                g_UserGenerator.GetSkeletonCap().GetSkeletonJoint(aUsers[i], (XnSkeletonJoint) arr_skeleton_part[s], skeletonJoint);
                if ( skeletonJoint.position.fConfidence ==  1.0f || skeletonJoint.orientation.fConfidence == 1.0f )
                {
                    XnPoint3D pt = skeletonJoint.position.position;
                    pt = skeletonJoint.position.position;
                    g_DepthGenerator.ConvertRealWorldToProjective(1, &pt, &pt);
                    printf("user %d: %s at (%6.2f,%6.2f,%6.2f)\n",aUsers[i], str_skeleton_name[s], pt.X, pt.Y, pt.Z);
                    circle(img8bitDepth, Point2f(pt.X, pt.Y) , 10, Scalar(200, 150, 150));
                }
            }
        }
        imshow("Depth View", img8bitDepth );
        waitKey(1);
    }
    g_DepthGenerator.Release();
    g_UserGenerator.Release();
    mContext.Release();
}