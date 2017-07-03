// NiGetSerinalNumber.cpp : 定義主控台應用程式的進入點。
//

#include "stdafx.h"
#include "NiGetSerinalNumber.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAX_STR_SIZE 128

using namespace std;
using namespace xn;

// 僅有的一個應用程式物件

CWinApp theApp;

using namespace std;

void showSerialNumber() {
    Context mContext;
    mContext.Init();

    Device mDevice;
    XnStatus status;
    status = mDevice.Create(mContext);
    if ( XN_STATUS_OK == status ) {
        DeviceIdentificationCapability mDeviceIdentCap = mDevice.GetIdentificationCap();
        XnChar strBuff[MAX_STR_SIZE] = {0};
        XnUInt32 strBuffSize = MAX_STR_SIZE;

        //cout << "GetName : " << mDeviceIdentCap.GetName() << endl;
        status = mDeviceIdentCap.GetSerialNumber( strBuff, strBuffSize );
        if ( XN_STATUS_OK == status ) {
            //cout << "GetSerialNumber : " << strBuff << endl;
            CString cstr(strBuff);
            MessageBox( nullptr, cstr, TEXT( "Serial Number" ), MB_OK );
        }
    } else {
        cout << "mDevice crate not ok" << endl;
    }
    mDevice.Release();
    mContext.Release();
    return;
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// 初始化 MFC 並於失敗時列印錯誤
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: 配合您的需要變更錯誤碼
			_tprintf(_T("嚴重錯誤: MFC 初始化失敗\n"));
			nRetCode = 1;
		}
		else
		{
			showSerialNumber();
		}
	}
	else
	{
		// TODO: 配合您的需要變更錯誤碼
		_tprintf(_T("嚴重錯誤: GetModuleHandle 失敗\n"));
		nRetCode = 1;
	}

	return nRetCode;
}
