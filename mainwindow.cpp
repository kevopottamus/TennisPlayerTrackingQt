#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <string>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <opencv2/opencv.hpp>


using namespace cv;
using namespace std;
using namespace boost;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_load_video_action = boost::shared_ptr<QAction>(new QAction(tr("&Load"),this));
    m_save_result_action = boost::shared_ptr<QAction>(new QAction(tr("&Save"),this));
    m_quit_app_action = boost::shared_ptr<QAction>(new QAction(tr("&Quit"),this));
    /// connect the actions to slots
    connect(m_load_video_action.get(),SIGNAL(triggered()),SLOT(_on_load_video_action()));
    connect(m_save_result_action.get(),SIGNAL(triggered()),SLOT(_on_save_result_action()));
    connect(m_quit_app_action.get(),SIGNAL(triggered()),SLOT(_on_quit_app_action()));

    /// add &File menu
    m_file_menu = boost::shared_ptr<QMenu>(menuBar()->addMenu(tr("&File")));
    /// add actions to the file menu just created
    m_file_menu->addAction(m_load_video_action.get());
    m_file_menu->addAction(m_save_result_action.get());
    m_file_menu->addAction(m_quit_app_action.get());
}


void MainWindow::_on_load_video_action(){
    cout << "load video..." << endl;
    /// bring up a dialog allow user to choose the video file
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Open Image"),
                                                    QDir::currentPath(),
                                                    tr("Files (*.mp4)"));


    if (filename.isEmpty()) // Do nothing if filename is empty
       return;
    setWindowTitle(filename);

    /// we load the video and extract the first frame and display it on the main window

    VideoCapture cap(filename.toStdString());
    //
    if(!cap.isOpened()){
        QMessageBox::information(this, tr("Load Video"),
                                 tr("Cannot load %1.").arg(filename));
        return;
    }
    /// read the first frame
    Mat firstFrame;
    cap >> firstFrame;
    ///
    cv::Size imgSize = firstFrame.size();
    m_image = boost::shared_ptr<QImage>(new QImage(imgSize.width,imgSize.height,QImage::Format_RGB888));
    /// convert Mat to RGB channel
    Mat rgbFrame;
    cv::cvtColor(firstFrame,rgbFrame,CV_BGR2RGB);
    memcpy(m_image->scanLine(0),(unsigned char*)rgbFrame.data,imgSize.width * imgSize.height * rgbFrame.channels());

    /// try to load the image
//    if (!filename.isEmpty()) {
//          m_image = boost::shared_ptr<QImage>(new QImage(filename));
//          if (m_image->isNull()) {
//              QMessageBox::information(this, tr("Image Viewer"),
//                                       tr("Cannot load %1.").arg(filename));
//              return;
//          }
//          /// resize the window to the image size
//          resize(m_image->size());
//          update();
//      }
//    cv::Mat img = cv::imread(filename.toStdString());
//    if (img.empty())
//        return;

//    // Since OpenCV uses BGR order, we need to convert it to RGB
//    cv::cvtColor(img, img, CV_BGR2RGB);

//    // _image is created according to video dimensions
//    if (_image)
//    {
//        delete _image;
//    }
//    _image = new QImage(img.size().width, img.size().height, QImage::Format_RGB888);

//    // Copy cv::Mat to QImage
//    memcpy(_image->scanLine(0), (unsigned char*)img.data, _image->width() * _image->height() * img.channels());

//    // Set the filename as the window title
//    setWindowTitle(filename);

//    // Resize the window to fit video dimensions
//    resize(img.size().width, img.size().height);

//    // Mouse move events will occur only when a mouse button is pressed down,
//    // unless mouse tracking has been enabled:
//    QWidget::setMouseTracking(true);

    // Trigger paint event to redraw the window
    update();

}

void MainWindow::paintEvent(QPaintEvent *event){
    if(!m_image)
        return;
    QPainter painter(this);
    painter.drawImage(QPoint(0,0),*m_image);
    QWidget::paintEvent(event);
}

void MainWindow::_on_save_result_action(){
    cout << "save result..." << endl;
}


void MainWindow::_on_quit_app_action(){
    cout << "close the application..." << endl;
    emit close();
}

MainWindow::~MainWindow()
{
    delete ui;
}
