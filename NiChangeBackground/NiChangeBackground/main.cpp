#include <stdio.h>
#include <iostream>

#include <XnOpenNI.h>
#include <XnCppWrapper.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace xn;
using namespace cv;

#define DEBUG 1
cv::Mat colorBG;
xn::Device mDevice;
vector<std::string> filenames;
int files = 0;

int handleEvent ( char keyEvent )
{
    int value = 0;
    if ( keyEvent == -1 || keyEvent == 255 ) return value;
#ifdef DEBUG
    cout<<"Receive key event: "<<keyEvent<<endl;
#endif
    switch (keyEvent)
    {
        case 27:            //esc key
            value = -1;
            break;
        case ' ':
            if (files == ( filenames.size() - 1) )
            {
                files = 0;
            }
            else
            {
                files++;
            }
            break;
        default:
            break;
    }
    return value;
}

// Callback: New user was detected
void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& /*generator*/, XnUserID nId, void* /*pCookie*/)
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    printf("%d New User %d\n", epochTime, nId);
}

// Callback: An existing user was lost
void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& /*generator*/, XnUserID nId, void* /*pCookie*/)
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    printf("%d Lost user %d\n", epochTime, nId);
}


int main( int argc, char* argv[] )
{
    Context mContext;
    DepthGenerator mDepthGen;
    ImageGenerator mImageGen;
    UserGenerator mUserGen;

    mContext.Init();
    mDepthGen.Create( mContext );

    XnMapOutputMode mapMode;
    mapMode.nXRes = 640;
    mapMode.nYRes = 480;
    mapMode.nFPS = 30;
    mDepthGen.SetMapOutputMode( mapMode );

    mImageGen.Create( mContext );

    mUserGen.Create( mContext );
    XnCallbackHandle hUserCallbacks;
    mUserGen.RegisterUserCallbacks(User_NewUser, User_LostUser, NULL, hUserCallbacks);
    mDepthGen.GetAlternativeViewPointCap().SetViewPoint( mImageGen );
    mContext.StartGeneratingAll();
    filenames.push_back ("bg\\1.jpg");
    filenames.push_back ("bg\\2.jpg");

    while (true)
    {
        colorBG = cv::imread( filenames[files].c_str() );
        cv::resize( colorBG, colorBG, Size(640, 480 ));
        mContext.WaitOneUpdateAll( mUserGen );
        xn::DepthMetaData depthData;
        xn::SceneMetaData sceneMD;
        mDepthGen.GetMetaData( depthData );
        Mat imgDepth( depthData.FullYRes(), depthData.FullXRes(), CV_16UC1, ( void* ) depthData.Data() );
        Mat img8bitDepth;
        imgDepth.convertTo( img8bitDepth, CV_8UC3, 255.0 / 5000 );

        mUserGen.GetUserPixels(0, sceneMD);

        xn::ImageMetaData imageData;
        mImageGen.GetMetaData( imageData );
        Mat imgColor ( imageData.FullYRes(), imageData.FullXRes(), CV_8UC3, ( void* )imageData.Data() );
        Mat imgBGRColor;
        cvtColor( imgColor, imgBGRColor, CV_RGB2BGR );
        for( int i = 0; i < imgBGRColor.rows; i++ )
        {
            for( int j = 0; j < imgBGRColor.cols; j++ )
            {
                if ( sceneMD(j, i) > 0 )
                {
                    Vec3b color = imgBGRColor.at<cv::Vec3b>( i, j );
                    colorBG.at<cv::Vec3b>( i, j ) = color;
                }
            }
        }
        imshow( "Depth view", img8bitDepth );
        imshow( "Color view", colorBG );
        if ( handleEvent( waitKey( 1 ) )  == -1 ) break;
    }
    mImageGen.Release();
    mDepthGen.Release();
    mContext.Release();
    return 0;
}