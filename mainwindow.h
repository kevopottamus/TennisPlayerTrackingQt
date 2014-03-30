#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <boost/shared_ptr.hpp>
using namespace boost;


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    void paintEvent(QPaintEvent *event);
    ~MainWindow();

protected:
    shared_ptr<QAction> m_load_video_action;
    shared_ptr<QAction> m_save_result_action;
    shared_ptr<QAction> m_quit_app_action;
    shared_ptr<QMenu> m_file_menu;
    shared_ptr<QImage> m_image;
protected slots:
    void _on_load_video_action();
    void _on_save_result_action();
    void _on_quit_app_action();
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
