# README #

This README is for the user who

* Want to develop application with LIPS 3D depth camera.
* Already has a LIPS camera module on hand.
* The target platform is Windows
* Develop application with OpenNI 1.5

If you don't have one and are interesting with it, please contact with us by e-mail [info@lips-hci.com](mailto:info@lips-hci.com)

### System Requirements ###

* Windows 7 or later

### Install LIPS 3D depth camera SDK ###

* Download SDK from [LIPS SDK] (http://www.lips-hci.com/products/sdk/)

* Install wizard will notify to install OpenNI if OpenNI is not installed well.

### Download LIPS Sample Code ###

#### Install GIT ####

* Please refer [Git-SCM](https://git-scm.com/) to install Git client for Windows platform.

#### Clone sample code ####
Clone the project in a git-bash console. (or GIT GUI client depends on you)

```
git clone https://github.com/lips-hci/openni_windows.git LIPS_Sample
```

#### Build Samples ####
##### NiRecorder #####

* Launch Visual Studio IDE by clicking NiRecorder\NiRecorder.sln.
* The example is created with Visual Studio 2012. You might be hinted that Visual Studio would like to update some files to adapt the new tool set if your Visual Studio version is higher than Visual Studio 2012.

##### NiSimpleViewer #####

* Install OpenCV 2.4.11 (or later). (Refer [OpenCV.org] (http://opencv.org/downloads.html) ). Assume you downloaded and extracted to C:\opencv
* Launch Visual Studio IDE by clicking NiSimpleViewer\NiSimpleViewer.sln.
* The example is created with Visual Studio 2012. You might be hinted that Visual Studio would like to update some files to adapt the new tool set if your Visual Studio version is higher than Visual Studio 2012.

#### Build your own application ####
You can base the samples we provided to develop your own application. What you need to know to create a new project are:

* After the new project was created, you should add OpenNI headers and library path in project configuration property. That is,

1. Right click the project in Solution Explorer and choose Properties

2. Expand "VC++ Directories", add "C:\Program Files\OpenNI\Include" to "Include Directories" and add "C:\Program Files\OpenNI\Lib64" to "Library Directories"

3. If you are developing a 32-bit application in a 64-bit machine, the paths might modified to "C:\Program Files (x86)\OpenNI\Include" and "C:\Program Files (x86)\OpenNI\Lib"

4. If you are developing a 32-bit application in a native 32-bit machine, the paths might modified to "C:\Program Files\OpenNI\Include" and "C:\Program Files\OpenNI\Lib"

5. Then, expand "Linker" in "Configuration Properties", add "openNI64.lib" to "Input->Additional Dependencies". (Or "openNI.lib" if you are developing a 32-bit application"
