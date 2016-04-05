TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle

SOURCES += main.cpp \
    grabcutapp.cpp \

INCLUDEPATH += C://opencv//build_mingw//install//include

LIBS += C:\opencv\build_mingw\lib\libopencv_calib3d2411d.dll.a
LIBS += C:\opencv\build_mingw\lib\libopencv_contrib2411d.dll.a
LIBS += C:\opencv\build_mingw\lib\libopencv_core2411d.dll.a
LIBS += C:\opencv\build_mingw\lib\libopencv_features2d2411d.dll.a
LIBS += C:\opencv\build_mingw\lib\libopencv_flann2411d.dll.a
LIBS += C:\opencv\build_mingw\lib\libopencv_highgui2411d.dll.a
LIBS += C:\opencv\build_mingw\lib\libopencv_imgproc2411d.dll.a
LIBS += C:\opencv\build_mingw\lib\libopencv_objdetect2411d.dll.a
LIBS += C:\opencv\build_mingw\lib\libopencv_video2411d.dll.a

HEADERS += \
    grabcutapp.h \
