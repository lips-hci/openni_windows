#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <winsock2.h>
#include <Ws2tcpip.h>
#define COMPRESS
#if defined(COMPRESS)
#include "zlib.h"
#endif

#if defined(RES_VGA)
#define IMG_WIDTH 640
#define IMG_HEIGHT 480
#else
#define IMG_WIDTH 320
#define IMG_HEIGHT 240
#endif
#define SERVER_PORT 5566
#define CLIENT_PORT 5567
#define MAX_DATA 1024
#define PKT_SIZE 51200
#define MAX_COUNT 20
#define FID_SIZE 11
#if defined(COMPRESS)
#define PAYLOAD_SIZE 6
#endif

using namespace std;
using namespace cv;

int main( int argc, char* argv[] )
{
    int iResult;
    int server_size = 0;
    int client_size = 0;
    WSADATA wsaData;
    char *data1;
#if !defined(COMPRESS)
    data1 = (char*)malloc((PKT_SIZE+FID_SIZE)*sizeof(char));
#else
    data1 = (char*)malloc((PKT_SIZE+FID_SIZE+PAYLOAD_SIZE)*sizeof(char));
#endif
    char *data_all;
    data_all = (char*)malloc(IMG_WIDTH*IMG_HEIGHT*2*sizeof(char));
    SOCKET socket_fd = INVALID_SOCKET;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    if ( argc < 2 ) {
        cout << "No parameter" << endl;
        return 1;
    }

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

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_size = sizeof(server_addr);

    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(CLIENT_PORT);
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    client_size = sizeof(client_addr);

    iResult = bind(socket_fd, (struct sockaddr *)&client_addr, client_size);
    if (iResult != 0) {
        cout << "bind failed with error " << WSAGetLastError() << endl;
        return 1;
    }

    char serial[11] = {0};
    char now_serial[11] = {0};
#if defined(COMPRESS)
    char payload_size[PAYLOAD_SIZE] = {0};
    uLong len = 0;
#endif
    int counter = 0;
    int recv_size = 0;
    while ( true )
    {
#if !defined(COMPRESS)
        recv_size = recvfrom(socket_fd, data1, PKT_SIZE + FID_SIZE, 0, (struct sockaddr*)&server_addr, (socklen_t *)&server_size);
#else
        recv_size = recvfrom(socket_fd, data1, PKT_SIZE + FID_SIZE + PAYLOAD_SIZE, 0, (struct sockaddr*)&server_addr, (socklen_t *)&server_size);
#endif
        if ( 0 == recv_size) {
            cout << "recv_size == 0" << endl;
        } else if (SOCKET_ERROR == recv_size) {
            cout << "Error: " << WSAGetLastError() << endl;
        }
        unsigned int offset = *data1 - 'A';
        memcpy(serial, data1 + 1, FID_SIZE-1);
        counter++;
        cout << "offset : " << offset << ", serial : " << serial << ", now_serial : " << now_serial << ", counter : " << counter << endl;
        if (!strcmp(now_serial, "")) {
            strcpy(now_serial, serial);
        } else if (strcmp(now_serial, serial)) {
            strcpy(now_serial, serial);
#if !defined(COMPRESS)
#if defined(RES_VGA)
            if (counter > 9) {
#else
            if (counter > 2) {
#endif
#else
            {
                int err;
                Byte *uzData;
                uzData = (Byte*)calloc(IMG_WIDTH*IMG_HEIGHT*2, 1);
                uLong uzDataLen = (uLong)(IMG_WIDTH*IMG_HEIGHT*2);
                err = uncompress(uzData, &uzDataLen, (Bytef*)(data_all), len);
                if (err != Z_OK) {
                    switch (err) {
                        case Z_ERRNO:
                            cout << "uncompress error: Z_ERROR" << endl;
                            break;
                        case Z_STREAM_ERROR:
                            cout << "uncompress error: Z_STREAM_ERROR" << endl;
                            break;
                        case Z_DATA_ERROR:
                            cout << "uncompress error: Z_DATA_ERROR" << endl;
                            break;
                        case Z_MEM_ERROR:
                            cout << "uncompress error: Z_MEM_ERROR" << endl;
                            break;
                        case Z_BUF_ERROR:
                            cout << "uncompress error: Z_BUF_ERROR" << endl;
                            break;
                        case Z_VERSION_ERROR:
                            cout << "uncompress error: Z_VERSION_ERROR" << endl;
                            break;
                        default:
                            cout << "uncompress error:" << err << endl;
                            break;
                    }
                    cout << "uzDataLen: " << uzDataLen << ", len: " << len << endl;
                }
#endif
#if !defined(COMPRESS)
                Mat imgDepth( IMG_HEIGHT, IMG_WIDTH, CV_16UC1, ( void* )data_all );
#else
                Mat imgDepth( IMG_HEIGHT, IMG_WIDTH, CV_16UC1, ( void* )uzData );
#endif
                Mat img8bitDepth;
                imgDepth.convertTo( img8bitDepth, CV_8U, 255.0 / 5000 );
                imshow( "Depth view", img8bitDepth );
                waitKey( 1 );
#if defined(compress)
                free(uzData);
#endif
            }
            counter = 0;
#if defined(COMPRESS)
            len = 0;
#endif
        }
#if !defined(COMPRESS)
        memcpy(data_all + offset * PKT_SIZE , data1 + FID_SIZE, PKT_SIZE);
#else
        memcpy(payload_size, data1 + FID_SIZE, PAYLOAD_SIZE - 1);
        cout << "payload_size : " << payload_size << "(" << atoi(payload_size) << ")" << endl;
        memcpy(data_all + len, data1 + FID_SIZE + PAYLOAD_SIZE, atoi(payload_size));
        len += atoi(payload_size);
#endif
    }
    iResult = closesocket(socket_fd);
    if (iResult == SOCKET_ERROR) {
        cout << "closesocket failed with error: " << WSAGetLastError() << endl;
    }
    WSACleanup();

    free(data1);
    free(data_all);
    return 0;
}
