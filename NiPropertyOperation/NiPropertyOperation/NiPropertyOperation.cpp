#include "stdafx.h"

#include <string>
#include <iostream>
#include <XnOpenNI.h>
#include <XnCppWrapper.h>

using namespace std;
using namespace xn;

int _tmain(int argc, _TCHAR* argv[])
{
    Context mContext;
    mContext.Init();

    Device mDevice;
    XnStatus status;
    string name;
    XnUInt64 value;
    status = mDevice.Create(mContext);
    if ( XN_STATUS_OK == status ) {
        cout << "GetIntProperty --> name : ";
        cin >> name;
        cout << endl;
        status = mDevice.GetIntProperty( name.c_str(), value );
        if ( XN_STATUS_OK == status ) {
            cout << "IntProperty ( " << name << ") = " << value;
        } else {
            cout << "status = " << xnGetStatusString(status) << ", fails" << endl;
        }
        cout << "SetIntProperty --> name : ";
        cin >> name;
        cout << endl << "SetIntProperty --> value : ";
        cin >> value;
        status = mDevice.SetIntProperty( name.c_str(), value );
        if ( XN_STATUS_OK == status ) {
            cout << "SetIntProperty success" << endl;
        } else {
            cout << "status = " << xnGetStatusString(status) << ", fails" << endl;
        }
        cout << "GetIntProperty --> name : ";
        cin >> name;
        cout << endl;
        status = mDevice.GetIntProperty( name.c_str(), value );
        if ( XN_STATUS_OK == status ) {
            cout << "IntProperty ( " << name << " ) = " << value << endl;
        } else {
            cout << "status = " << xnGetStatusString(status) << ", fails" << endl;
        }
        system("pause");
    } else {
        cout << "Create depth generator fails!" << endl;
    }
    mDevice.Release();
    mContext.Release();
    return 0;
}

