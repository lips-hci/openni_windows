#include <XnOpenNI.h>
#include <XnCppWrapper.h>
#include <iostream>
#include <winsock2.h>
#include <Ws2tcpip.h>

#if defined(RES_VGA)
#define IMG_WIDTH 640
#define IMG_HEIGHT 480
#else
#define IMG_WIDTH 320
#define IMG_HEIGHT 240
#endif
#define IMG_FPS 30
#define SERVER_PORT 5566
#define CLIENT_PORT 5567
#define PKT_SIZE 51200
#define FID_SIZE 11

using namespace std;
using namespace xn;

int main( int argc, char* argv[] )
{
    // 1. Prepare OpenNI context and depth generator
    Context mContext;
    mContext.Init();

    XnMapOutputMode mapMode;
    mapMode.nXRes = IMG_WIDTH;
    mapMode.nYRes = IMG_HEIGHT;
    mapMode.nFPS = IMG_FPS;

    DepthGenerator mDepthGen;
    mDepthGen.Create( mContext );
    mDepthGen.SetMapOutputMode( mapMode );

    int iResult;
    WSADATA wsaData;

    SOCKET socket_fd = INVALID_SOCKET;
    struct sockaddr_in client_addr;

    int length;
    char *data1;
    char *data2;
    char *data3;
#if defined(RES_VGA)
    char *data4;
    char *data5;
    char *data6;
    char *data7;
    char *data8;
    char *data9;
    char *data10;
    char *data11;
    char *data12;
#endif
    char frame_id[FID_SIZE] = {0};
   
    data1 = (char*)malloc((PKT_SIZE + FID_SIZE)*sizeof(char));
    data2 = (char*)malloc((PKT_SIZE + FID_SIZE)*sizeof(char));
    data3 = (char*)malloc((PKT_SIZE + FID_SIZE)*sizeof(char));
#if defined(RES_VGA)
    data4 = (char*)malloc((PKT_SIZE + FID_SIZE)*sizeof(char));
    data5 = (char*)malloc((PKT_SIZE + FID_SIZE)*sizeof(char));
    data6 = (char*)malloc((PKT_SIZE + FID_SIZE)*sizeof(char));
    data7 = (char*)malloc((PKT_SIZE + FID_SIZE)*sizeof(char));
    data8 = (char*)malloc((PKT_SIZE + FID_SIZE)*sizeof(char));
    data9 = (char*)malloc((PKT_SIZE + FID_SIZE)*sizeof(char));
    data10 = (char*)malloc((PKT_SIZE + FID_SIZE)*sizeof(char));
    data11 = (char*)malloc((PKT_SIZE + FID_SIZE)*sizeof(char));
    data12 = (char*)malloc((PKT_SIZE + FID_SIZE)*sizeof(char));
#endif

    // 2. Prepare UDP socket
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        cout << "WSAStartup failed with error: " << iResult << endl;
        return 1;
    }
    socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_fd == INVALID_SOCKET) {
        cout << "socket failed with error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // 3. Set up address
    memset(&client_addr, 0, sizeof(client_addr));

    mContext.StartGeneratingAll();

    DepthMetaData mDepthMD;

    while ( !xnOSWasKeyboardHit() )
    {
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons(CLIENT_PORT);
        client_addr.sin_addr.s_addr = inet_addr(argv[1]);

        cout << "Start running, press any key to stop" << endl;
        while ( !xnOSWasKeyboardHit() )
        {
            mContext.WaitOneUpdateAll(mDepthGen);
            mDepthGen.GetMetaData(mDepthMD);
            sprintf(frame_id, "%d", mDepthMD.FrameID());
            *data1 = 'A';
            *data2 = 'B';
            *data3 = 'C';
#if defined(RES_VGA)
            *data4 = 'D';
            *data5 = 'E';
            *data6 = 'F';
            *data7 = 'G';
            *data8 = 'H';
            *data9 = 'I';
            *data10 = 'J';
            *data11 = 'K';
            *data12 = 'L';
#endif
            memcpy(data1 + 1, frame_id, FID_SIZE);
            memcpy(data1 + FID_SIZE, mDepthMD.Data(), PKT_SIZE);
            memcpy(data2 + 1, frame_id, FID_SIZE);
            memcpy(data2 + FID_SIZE, mDepthMD.Data() + PKT_SIZE/2, PKT_SIZE);
            memcpy(data3 + 1, frame_id, FID_SIZE);
            memcpy(data3 + FID_SIZE, mDepthMD.Data() + PKT_SIZE, PKT_SIZE);
#if defined(RES_VGA)
            memcpy(data4 + 1, frame_id, FID_SIZE);
            memcpy(data4 + FID_SIZE, mDepthMD.Data() + PKT_SIZE + PKT_SIZE/2, PKT_SIZE);
            memcpy(data5 + 1, frame_id, FID_SIZE);
            memcpy(data5 + FID_SIZE, mDepthMD.Data() + 2 * PKT_SIZE, PKT_SIZE);
            memcpy(data6 + 1, frame_id, FID_SIZE);
            memcpy(data6 + FID_SIZE, mDepthMD.Data() + 2 * PKT_SIZE + PKT_SIZE/2, PKT_SIZE);
            memcpy(data7 + 1, frame_id, FID_SIZE);
            memcpy(data7 + FID_SIZE, mDepthMD.Data() + 3 * PKT_SIZE, PKT_SIZE);
            memcpy(data8 + 1, frame_id, FID_SIZE);
            memcpy(data8 + FID_SIZE, mDepthMD.Data() + 3 * PKT_SIZE + PKT_SIZE/2, PKT_SIZE);
            memcpy(data9 + 1, frame_id, FID_SIZE);
            memcpy(data9 + FID_SIZE, mDepthMD.Data() + 4 * PKT_SIZE, PKT_SIZE);
            memcpy(data10 + 1, frame_id, FID_SIZE);
            memcpy(data10 + FID_SIZE , mDepthMD.Data() + 4 * PKT_SIZE + PKT_SIZE/2, PKT_SIZE);
            memcpy(data11 + 1, frame_id, FID_SIZE);
            memcpy(data11 + FID_SIZE, mDepthMD.Data() + 5 * PKT_SIZE, PKT_SIZE);
            memcpy(data12 + 1, frame_id, FID_SIZE);
            memcpy(data12 + FID_SIZE, mDepthMD.Data() + 5 * PKT_SIZE + PKT_SIZE/2, PKT_SIZE);
#endif
            length = sizeof(client_addr);
            sendto( socket_fd, data1, PKT_SIZE + FID_SIZE, 0, (struct sockaddr*)&client_addr, length);
            sendto( socket_fd, data2, PKT_SIZE + FID_SIZE, 0, (struct sockaddr*)&client_addr, length);
            sendto( socket_fd, data3, PKT_SIZE + FID_SIZE, 0, (struct sockaddr*)&client_addr, length);
#if defined(RES_VGA)
            sendto( socket_fd, data4, PKT_SIZE + FID_SIZE, 0, (struct sockaddr*)&client_addr, length);
            sendto( socket_fd, data5, PKT_SIZE + FID_SIZE, 0, (struct sockaddr*)&client_addr, length);
            sendto( socket_fd, data6, PKT_SIZE + FID_SIZE, 0, (struct sockaddr*)&client_addr, length);
            sendto( socket_fd, data7, PKT_SIZE + FID_SIZE, 0, (struct sockaddr*)&client_addr, length);
            sendto( socket_fd, data8, PKT_SIZE + FID_SIZE, 0, (struct sockaddr*)&client_addr, length);
            sendto( socket_fd, data9, PKT_SIZE + FID_SIZE, 0, (struct sockaddr*)&client_addr, length);
            sendto( socket_fd, data10, PKT_SIZE + FID_SIZE, 0, (struct sockaddr*)&client_addr, length);
            sendto( socket_fd, data11, PKT_SIZE + FID_SIZE, 0, (struct sockaddr*)&client_addr, length);
            sendto( socket_fd, data12, PKT_SIZE + FID_SIZE, 0, (struct sockaddr*)&client_addr, length);
#endif
        }
    }
    iResult = closesocket(socket_fd);
    if (iResult == SOCKET_ERROR) {
        cout << "closesocket failed with error: " << WSAGetLastError() << endl;
    }
    WSACleanup();

    mContext.StopGeneratingAll();

    mContext.Release();

    free(data1);
    free(data2);
    free(data3);
#if defined(RES_VGA)
    free(data4);
    free(data5);
    free(data6);
    free(data7);
    free(data8);
    free(data9);
    free(data10);
    free(data11);
    free(data12);
#endif
    return 0;
}

