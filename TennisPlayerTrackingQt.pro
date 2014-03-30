#-------------------------------------------------
#
# Project created by QtCreator 2014-03-29T15:23:15
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TennisPlayerTrackingQt
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    BackgroundRemover.cpp \
    MotionDetector.cpp \
    ParticleFilter.cpp \
    SVMPlayerDetector.cpp \
    Utility.cpp

HEADERS  += mainwindow.h \
    BackgroundRemover.h \
    MotionDetector.h \
    ParticleFilter.h \
    SVMPlayerDetector.h \
    Utility.h

FORMS    += mainwindow.ui

INCLUDEPATH += C:\local\boost_1_55_0 \
            C:\Progra~1\opencv\build\include


LIBS += -LC:\Progra~1\opencv\build\x86\vc10\lib \
    -lopencv_calib3d245d \
    -lopencv_contrib245d \
    -lopencv_core245d \
    -lopencv_features2d245d \
    -lopencv_flann245d \
    -lopencv_gpu245d \
    -lopencv_haartraining_engined\
    -lopencv_highgui245d \
    -lopencv_imgproc245d \
    -lopencv_legacy245d \
    -lopencv_ml245d \
    -lopencv_nonfree245d \
    -lopencv_objdetect245d \
    -lopencv_photo245d \
    -lopencv_stitching245d \
    -lopencv_superres245d \
    -lopencv_ts245d \
    -lopencv_video245d \
    -lopencv_videostab245d

