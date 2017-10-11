#include <iostream>
#include <XnCppWrapper.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <mutex>
#include <Ws2tcpip.h>

#define SERV_PORT 5567

#define VGA_WIDTH 640
#define VGA_HEIGHT 480
#define FID_SIZE 11
#define TOFWRDATA_SIZE 614400
#define RGBWRDATA_SIZE 921600

using namespace std;
using namespace xn;
using namespace cv;

Mat combineDep( VGA_HEIGHT * 2, VGA_WIDTH * 2, CV_16UC1 );
Mat combineRGB( VGA_HEIGHT * 2, VGA_WIDTH * 2, CV_8UC3 );
char *tcpDepth, *tcpRGB;
volatile unsigned int showCount;
volatile bool pflag;
Mat img8bitDepth;
Mat imgBGRColor;
mutex gMutex;


DWORD WINAPI showview( LPVOID ptr )
{
    //cout << "showview function enter" << endl;

    int keyEvent, eventCheck = '1';

    while ( true )
    {
        if ( pflag == true )
        {
            if ( eventCheck == '1' )
            {
                destroyWindow( "Color view" );
                imshow( "Depth view", img8bitDepth );
            }
            else if ( eventCheck == '2' )
            {
                destroyWindow( "Depth view" );
                imshow( "Color view", imgBGRColor );
            }

            keyEvent = waitKey ( 1 );

            if ( ( keyEvent == '1' ) || ( keyEvent == '2' ) )
            {
                eventCheck = keyEvent;
            }
        }
    }
}

void spilitbuf( char *data, unsigned int count )
{
    gMutex.lock();

    int posX = 0, posY = 0;

    switch ( count )
    {
        case 4:
            posX = 0;
            posY = 0;
            break;
        case 3:
            posX = 640;
            posY = 0;
            break;
        case 2:
            posX = 0;
            posY = 480;
            break;
        case 1:
            posX = 640;
            posY = 480;
            break;
        default:
            posX = 0;
            posY = 0;
            break;
    }

    memcpy( tcpDepth, data + FID_SIZE, TOFWRDATA_SIZE );
    Mat imgDepth( VGA_HEIGHT, VGA_WIDTH, CV_16UC1, ( void* )( tcpDepth ) );
    imgDepth.copyTo( combineDep( Rect( posX, posY, VGA_WIDTH, VGA_HEIGHT ) ) );
    combineDep.convertTo( img8bitDepth, CV_8U, 255.0 / 4096.0 );

    memcpy( tcpRGB, data + FID_SIZE + TOFWRDATA_SIZE + FID_SIZE, RGBWRDATA_SIZE );
    Mat imgColor( VGA_HEIGHT, VGA_WIDTH, CV_8UC3, ( void* )( tcpRGB ) );
    imgColor.copyTo( combineRGB( Rect( posX, posY, VGA_WIDTH, VGA_HEIGHT ) ) );
    cvtColor( combineRGB, imgBGRColor, CV_RGB2BGR );
    pflag = true;

    gMutex.unlock();
}

DWORD WINAPI readSocket( LPVOID fd )
{
    cout << "readSocket function enter" << endl;

    int readLen = TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE;

    char TOFserialStr[FID_SIZE] = {0};
    char RGBserialStr[FID_SIZE] = {0};
    int nbytes = 0;
    int readTotal = 0;
    char *data;
    char *dataAll;
    int showpos;

    showpos = showCount;
    showCount--;

    data = ( char* )malloc( ( TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE ) * sizeof( char ) );
    dataAll = ( char* )malloc( ( TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE ) * sizeof( char ) );

    while ( true )
    {
        int TOFserialNum = 0;
        int RGBserialNum = 0;

        if ( readLen >= nbytes )
        {
            nbytes = recv( *( int * )fd, data, ( TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE ), 0 );
        }
        else
        {
            nbytes = recv( *( int * )fd, data, readLen, 0 );
        }

        if ( nbytes == -1 )
        {
            cout << "read from client error !" << endl;
            break;
        }
        else if ( ( readTotal + nbytes ) > ( TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE ) )
        {
            //cout << "Data reading too much!!" << endl;
            memcpy( dataAll + readTotal, data, ( TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE - readTotal ) );
            {
                memcpy( TOFserialStr, dataAll, FID_SIZE );
                memcpy( RGBserialStr, dataAll + FID_SIZE + TOFWRDATA_SIZE, FID_SIZE );
                TOFserialNum = atoi( TOFserialStr );
                RGBserialNum = atoi( RGBserialStr );
                spilitbuf( dataAll, showpos );
                //cout << "TOF Frame ID = " << TOFserialNum << ",  RGB Frame ID = " << RGBserialNum << endl;

                memset( dataAll, 0, ( TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE ) );
            }
            //cout << "nbytes = " << nbytes << ", readTotal = " << readTotal << endl;
            memcpy( dataAll, data + ( TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE - readTotal ), nbytes - ( TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE - readTotal ) );
            readLen = TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE - ( nbytes - ( TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE - readTotal ) );
            readTotal = nbytes - ( TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE - readTotal );

        }
        else if ( nbytes != ( TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE ) )
        {
            memcpy( dataAll + readTotal, data, nbytes );
            readLen -= nbytes;
            readTotal += nbytes;
            //cout << "nbytes = " << nbytes << ", readTotal = " << readTotal << endl;
        }

        if ( ( nbytes == ( TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE ) ) || ( readTotal == ( TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE ) ) )
        {
            if ( nbytes == ( TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE ) )
            {
                memcpy( dataAll, data, ( TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE ) );
            }

            memcpy( TOFserialStr, dataAll, FID_SIZE );
            memcpy( RGBserialStr, dataAll + FID_SIZE + TOFWRDATA_SIZE, FID_SIZE );
            TOFserialNum = atoi( TOFserialStr );
            RGBserialNum = atoi( RGBserialStr );
            spilitbuf( dataAll, showpos );
            //cout << "TOF Frame ID = " << TOFserialNum << ",  RGB Frame ID = " << RGBserialNum << endl;

            nbytes = 0;
            readTotal = 0;
            readLen = TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE;
            memset( dataAll, 0, ( TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE ) );
            memset( data, 0, ( TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE ) );
        }
    }

    free( data );
    free( dataAll );
    free( tcpDepth );
    free( tcpRGB );

    return 0;
}

int main()
{
    SOCKET socket_fd = INVALID_SOCKET;      /* file description into transport */
    int length;     /* length of address structure      */
    int recfd[4];     /* file descriptor to accept        */
    struct sockaddr_in myaddr; /* address of this service */
    struct sockaddr_in client_addr; /* address of client    */
    WSADATA wsaData;
    HANDLE sThread;
    HANDLE rThread[4];
    DWORD tcpthread[4];
    DWORD showthread;
    int ret = 0, i;

    tcpDepth = ( char * )malloc( VGA_WIDTH * VGA_HEIGHT * 2 );
    tcpRGB = ( char * )malloc( VGA_WIDTH * VGA_HEIGHT * 3 );

    pflag = false;
    showCount = 4;

    cout << "Server program enter!!" << endl;

    ret = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
    if ( ret != NO_ERROR )
    {
        cout << "WSAStartup failed with error: " << ret << endl;
        return 1;
    }

    // 1. Get a socket into TCP/IP
    if ( ( socket_fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
        cout << "socket failed" << endl;
        return 1;
    }

    // 2. Set up our address
    memset( &myaddr, 0, sizeof( myaddr ) );
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl( INADDR_ANY );
    myaddr.sin_port = htons( SERV_PORT );

    // 3. Bind to the address to which the service will be offered
    ret = ::bind( socket_fd, ( struct sockaddr * )&myaddr, sizeof( myaddr ) );
    if ( ret < 0 )
    {
        cout << "bind failed" << endl;
        WSACleanup();
        return 1;
    }

    // 4. Set up the socket for listening, with a queue length of
    if ( listen( socket_fd, 20 ) < 0 )
    {
        cout << "listen failed" << endl;
        WSACleanup();
        return 1;
    }

    length = sizeof( client_addr );

    sThread = CreateThread( NULL, 0, showview, NULL, 0, &showthread );

    void *serfd;
    int threadCount = -1;

    cout << "Press any key to stop Server >>" << endl;

    // 5. Loop continuously, waiting for connection requests and performing the service
    fd_set oRSet;
    struct timeval stTimeout;
    stTimeout.tv_sec = 1;
    stTimeout.tv_usec = 0;

    while ( !xnOSWasKeyboardHit() )
    {
        FD_ZERO( &oRSet );
        FD_SET( socket_fd, &oRSet );
        ret = select( 1, &oRSet, NULL, NULL, &stTimeout );

        if ( FD_ISSET( socket_fd, &oRSet ) )
        {
            if ( ( recfd[threadCount + 1] = accept( socket_fd, ( struct sockaddr * )&client_addr, ( socklen_t* )&length ) ) >= 0 )
            {
                cout << "Client IP " << inet_ntoa( client_addr.sin_addr ) << " Connect!!" << endl;

                if ( threadCount < 4 )
                {
                    serfd = &recfd[threadCount + 1];
                    rThread[threadCount + 1] = CreateThread( NULL, 0, readSocket, serfd, 0, &tcpthread[threadCount + 1] );
                    threadCount++;
                }
            }
        }
    }

    for ( i = 0; i <= threadCount; i++ )
    {
        ret = CloseHandle( rThread[i] );
        if ( ret <= 0 )
        {
            cout << "Stop rThread fail, ret = " << ret << endl;
        }
    }

    ret = CloseHandle( sThread );
    if ( ret <= 0 )
    {
        cout << "Stop sThread fail, ret = " << ret << endl;
    }

    WSACleanup();
    cout << "Server program ending!!" << endl;

    return 0;
}