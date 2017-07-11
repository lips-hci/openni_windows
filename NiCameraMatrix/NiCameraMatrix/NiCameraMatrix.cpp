#include "stdafx.h"
#include "NiCameraMatrix.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAX_STR_SIZE 128

CWinApp theApp;

using namespace std;
using namespace xn;

void showCameraMatrix() {
    Context mContext;
    mContext.Init();

    DepthGenerator mDepthGen;
    ImageGenerator mImageGen;
    XnStatus status;
    XnDouble fx, fy, cx, cy;
    XnChar strDepBuff[MAX_STR_SIZE] = {0};
    XnChar strImgBuff[MAX_STR_SIZE] = {0};
    status = mDepthGen.Create(mContext);
    if ( XN_STATUS_OK == status ) {
        mDepthGen.GetRealProperty("fx", fx);
        mDepthGen.GetRealProperty("fy", fy);
        mDepthGen.GetRealProperty("cx", cx);
        mDepthGen.GetRealProperty("cy", cy);
        sprintf_s(strDepBuff, "fx=%f, fy=%f, cx=%f, cy=%f\n", fx, fy, cx, cy);

    } else {
        cout << "mDepthGen create fails" << endl;
    }
    status = mImageGen.Create(mContext);
    if ( XN_STATUS_OK == status ) {
        mImageGen.GetRealProperty("fx", fx);
        mImageGen.GetRealProperty("fy", fy);
        mImageGen.GetRealProperty("cx", cx);
        mImageGen.GetRealProperty("cy", cy);
        sprintf_s(strImgBuff, "fx=%f, fy=%f, cx=%f, cy=%f\n", fx, fy, cx, cy);
    } else {
        cout << "mImageGen create fails" << endl;
    }
    CString cstr = CString("[Depth camera]\n") + CString(strDepBuff) + CString("\n[Image camera]\n") + CString(strImgBuff);
    MessageBox( nullptr, cstr, TEXT( "Camera matrix" ), MB_OK );
    mDepthGen.Release();
    mImageGen.Release();
    mContext.Release();
    return;
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			_tprintf(_T("Fatal error: MFC initialization fails\n"));
			nRetCode = 1;
		}
		else
		{
			showCameraMatrix();
		}
	}
	else
	{
		_tprintf(_T("Fatal error: GetModuleHandle fails\n"));
		nRetCode = 1;
	}

	return nRetCode;
}
