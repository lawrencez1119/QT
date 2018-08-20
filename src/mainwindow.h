#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QImage>
#include <QLabel>
#include <QTextCodec>
#include <QMouseEvent>
#include <QMessageBox>
#include <QPaintEvent>
#include <QEvent>
#include <QObject>
#include <vector>
#include <qdebug.h>
#include <QScrollBar>
#include <QColor>
#include <QColorDialog>
#include <opencv2/opencv.hpp>
using namespace cv;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionOpen_Image_triggered();//读取文件
    void DisplayMat(Mat img);//在qlabel上显示图像
    void on_btnSLIC_clicked();

    void on_actionSave_Image_triggered();

    void on_resizeSlider_valueChanged(int value);

    bool eventFilter(QObject *obj, QEvent *event);

    void on_actionSave_Segmentation_triggered();

    void on_actionLoad_Segmentation_triggered();

    void on_btnColor_clicked();

private:
    Ui::MainWindow *ui;
    QLabel *imageLabel;
    Mat image;//原图
    Mat slicImg;//slic算法输入图
    Mat mask;//单通道mask
    Mat mask_3;//三通道mask
    Mat labels;//slic算法标签
    Mat imgSave;//代保存的带mask的图
    float alpha;//不透明度
    int sp_number;//silc算法超像素个数
    float x_offset;//滚动框x方向偏移
    float y_offset;//滚动框y方向偏移
    CvScalar color;//当前颜色
    CvScalar saveColor;//存储最近使用的颜色
    bool isDelete=true;//控制删除标签
    bool isShow=true;//控制显示快捷键
    double fx=1.0;//放缩倍数
    double fy=1.0;//放缩倍数
    //int saveXmax;//存储滚动框x最近的最大值
    //int saveYmax;//存储滚动框y最近的最大值
};

#endif // MAINWINDOW_H
