#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <winsock2.h>
#include <Ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>

#if defined(COMPRESS)
#include "zlib.h"
#endif

#define SERV_PORT 5567

#define IMG_WIDTH 640
#define IMG_HEIGHT 480
#define FID_SIZE 11

#if defined(COMPRESS)
#define RDDATA_SIZE 210000
#define PAYLOAD_SIZE 7
#else
#define TOFWRDATA_SIZE 614400
#define RGBWRDATA_SIZE 921600
#endif

using namespace std;
using namespace cv;

void showview(char *data) {
    Mat imgDepth( IMG_HEIGHT, IMG_WIDTH, CV_16UC1, ( void* )(data));
    Mat img8bitDepth;
    imgDepth.convertTo( img8bitDepth, CV_8U, 255.0 / 4096.0);
    imshow( "Depth view", img8bitDepth);

    Mat imgColor( IMG_HEIGHT, IMG_WIDTH, CV_8UC3, ( void* )(data + TOFWRDATA_SIZE + FID_SIZE));
    Mat imgBGRColor;
    cvtColor( imgColor, imgBGRColor, CV_RGB2BGR );
    imshow( "Color view", imgBGRColor);

    waitKey( 1 );
}

int main( int argc, char* argv[] )
{
	int iResult;
	SOCKET fd = INVALID_SOCKET;
    struct sockaddr_in servaddr; /* the server's full addr */
	WSADATA wsaData;
    char *data;
    char *dataAll;

#if defined(COMPRESS)
    data = (char*)malloc((RDDATA_SIZE) * sizeof(char));
    dataAll = (char*)malloc((RDDATA_SIZE) * sizeof(char));
#else
    data = (char*)malloc((TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE) * sizeof(char));
    dataAll = (char*)malloc((TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE) * sizeof(char));
#endif

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        cout << "WSAStartup failed with error: " << iResult << endl;
        return 1;
    }

    // 1. Get a socket into TCP/IP
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout << "socket failed with error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // 2. Fill in the server's address.
	memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
	servaddr.sin_addr.s_addr = inet_addr(argv[1]);

    // 3. Connect to the server.
    if (connect(fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        cout << "connect failed!" << endl;
        WSACleanup();
        return 1;
    }

#if defined(COMPRESS)
    Byte *uzData;
    char payload_size[PAYLOAD_SIZE] = {0};
    int err = 0;
    uzData = (Byte*)malloc((IMG_WIDTH * IMG_HEIGHT * 2) * sizeof(Byte));
#else
    int readLen = TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE;
#endif

    char TOFserialStr[FID_SIZE] = {0};
    char RGBserialStr[FID_SIZE] = {0};
    int nbytes = 0;
    int readTotal = 0;

    while (true) {
#if defined(COMPRESS)
        uLong uzDataLen = (uLong)(IMG_WIDTH * IMG_HEIGHT * 2);
        uLong len = 0;
        memset(uzData, 0, IMG_WIDTH * IMG_HEIGHT * 2);
#endif
        int TOFserialNum = 0;
        int RGBserialNum = 0;

#if defined(COMPRESS)
        nbytes = read(fd, data, RDDATA_SIZE);
#else

        if (readLen >= nbytes)
			nbytes = recv((int)fd, data, (TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE), 0);
        else
			nbytes = recv((int)fd, data, readLen, 0);

#endif

        if (nbytes == -1) {
            cout << "read from server error !" << endl;
            return 1;
#if defined(COMPRESS)
        } else if (nbytes != RDDATA_SIZE) {
            memcpy(dataAll + readTotal, data, nbytes);
            readTotal = nbytes + readTotal;
#else
        } else if ((readTotal + nbytes) > (TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE)) {
            cout << "Data reading too much!!" << endl;
            memcpy(dataAll + readTotal, data, (TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE - readTotal));
            {
                memcpy(TOFserialStr, dataAll, FID_SIZE);
                memcpy(RGBserialStr, dataAll + FID_SIZE + TOFWRDATA_SIZE, FID_SIZE);
                TOFserialNum = atoi(TOFserialStr);
                RGBserialNum = atoi(RGBserialStr);
                cout << "TOF Frame ID = " << TOFserialNum << ",  RGB Frame ID = " << RGBserialNum << endl;
                showview(dataAll + FID_SIZE);
                memset(dataAll, 0, (TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE));
            }
            //cout << "nbytes = " << nbytes << ", readTotal = " << readTotal << endl;
            memcpy(dataAll, data + (TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE - readTotal), nbytes - (TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE - readTotal));
            readLen = TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE - (nbytes - (TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE - readTotal));
            readTotal = nbytes - (TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE - readTotal);

        } else if (nbytes != (TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE)) {
            memcpy(dataAll + readTotal, data, nbytes);
            readLen -= nbytes;
            readTotal += nbytes;
            //cout << "nbytes = " << nbytes << ", readTotal = " << readTotal << endl;
#endif
        }

#if defined(COMPRESS)
        if ((nbytes == RDDATA_SIZE) || (readTotal == RDDATA_SIZE)) {
            if (readTotal == RDDATA_SIZE) {
                memset(data, 0 , RDDATA_SIZE);
                memcpy(data, dataAll, RDDATA_SIZE);
            }
#else
        if ((nbytes == (TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE)) || (readTotal == (TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE))) {
            if (readTotal == (TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE))
                memcpy(data, dataAll, (TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE));
#endif

            memcpy(TOFserialStr, data, FID_SIZE);
            memcpy(RGBserialStr, dataAll + FID_SIZE + TOFWRDATA_SIZE, FID_SIZE);
            TOFserialNum = atoi(TOFserialStr);
            RGBserialNum = atoi(RGBserialStr);

            cout << "TOF Frame ID = " << TOFserialNum << ",  RGB Frame ID = " << RGBserialNum << endl;

#if defined(COMPRESS)
            memcpy(payload_size, data + FID_SIZE, PAYLOAD_SIZE - 1);
            len = (uLong)atoi(payload_size);
            err = uncompress(uzData, &uzDataLen, (Bytef*)(data + FID_SIZE + PAYLOAD_SIZE), len);

            cout << "frame_id = " << serialNum << ", payload_size = " << len << ", uzDataLen = " << uzDataLen << endl;

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
            } else {
                showview(uzData);
            }

            nbytes = 0;
            readTotal = 0;
            memset(dataAll, 0, RDDATA_SIZE);
#else
            showview(data + FID_SIZE);

            nbytes = 0;
            readTotal = 0;
            readLen = TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE;
            memset(dataAll, 0, (TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE));
            memset(data, 0, (TOFWRDATA_SIZE + FID_SIZE + RGBWRDATA_SIZE + FID_SIZE));
#endif
        }
    } 

#if defined(COMPRESS)
    free(uzData);
#endif
    free(data);
    free(dataAll);
    WSACleanup();
    return 0;
}