#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "opencv2/highgui.hpp"
#include "opencv2/ximgproc.hpp"
#include "opencv2/opencv.hpp"
#include "fill.h"
using namespace std;
using namespace cv;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->scrollImage->installEventFilter(this);
    ui->scrollImage->setMouseTracking(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}
//显示图片
void MainWindow::DisplayMat(Mat image)
{
    Mat rgb;
    QImage img;
    imageLabel = new QLabel();
    if(image.channels() == 3)
    {
        cvtColor(image,rgb,CV_BGR2RGB);
        img = QImage((const unsigned char*)(rgb.data),
                     rgb.cols,rgb.rows,rgb.cols*rgb.channels(),//rgb.cols*rgb.channels()可以替换为image.step
                     QImage::Format_RGB888);
    }
    else
    {
        img = QImage((const unsigned char*)(image.data),
                     image.cols,image.rows,rgb.cols*image.channels(),
                     QImage::Format_RGB888);
    }
    imageLabel->setPixmap(QPixmap::fromImage(img));//setPixelmap(QPixmap::fromImage(img));
    //imageLabel->resize(QSize(img.width(),img.height()));//resize(ui->label->pixmap()->size());
    //ui->scrollImage->setAlignment(Qt::AlignCenter);
    ui->scrollImage->setWidget(imageLabel);
    ui->scrollImage->setMaximumHeight(450);
    ui->scrollImage->setMaximumWidth(450);
    //ui->scrollImage->widgetResizable();
    ui->scrollImage->resize(img.width(),img.height());
    //int curXmax=ui->scrollImage->horizontalScrollBar()->maximum();
    //int curYmax=ui->scrollImage->verticalScrollBar()->maximum();
    ui->scrollImage->horizontalScrollBar()->setValue((int)x_offset);

    ui->scrollImage->verticalScrollBar()->setValue((int)y_offset);

    //ui->scrollImage->resize(QSize(img.width(),img.height()));
}
//读取图片
void MainWindow::on_actionOpen_Image_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,
                           tr("Open Image"),".",tr("Image File (*.jpg *.png *.bmp *.tif)"));
    QTextCodec *code = QTextCodec::codecForName("gb18030");
    std::string name = code->fromUnicode(filename).data();
    image = imread(name);
    fx=1.0;
    fy=1.0;
    DisplayMat(image);
}



//slic算法
void MainWindow::on_btnSLIC_clicked()
{
    slicImg=image.clone();
    QString t=ui->textSize->toPlainText();//取出文本框的值
    QString tr=ui->textRuler->toPlainText();//取出文本框的值
    int size=t.toInt();
    int ruler=tr.toFloat();
    Ptr<cv::ximgproc::SuperpixelSLIC> slic = cv::ximgproc::createSuperpixelSLIC(slicImg,101,size,ruler);//创建一个对象

    slic->iterate(20);//迭代次数，默认为10
    slic->enforceLabelConnectivity();
    slic->getLabelContourMask(mask,false);//获取超像素的边界
    slic->getLabels(labels);//获取labels
    sp_number = slic->getNumberOfSuperpixels();//获取超像素的数量
    //imshow("mask",labels);
    //qDebug()<<labels.size<<endl;
    //单通道变三通道
    vector<Mat> channels;
    for (int i=0;i<3;i++)
    {
        channels.push_back(mask);
    }
    merge(channels,mask_3);
    //slicImg.setTo(Scalar(255, 255, 255), mask);//将mask放到图像上
    Mat imgshow;
    alpha=ui->textAlpha->toPlainText().toFloat();
    addWeighted(slicImg,alpha,mask_3,1-alpha,0.0,imgshow);//重叠显示
    imgSave=imgshow;
    //cv::resize(mask,mask,Size(mask.cols+2,mask.rows+2));
    DisplayMat(imgshow);
    fx=1.0;
    fy=1.0;

}
//保存图片
void MainWindow::on_actionSave_Image_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this,tr("Save Image"),"",tr("Images (*.png *.bmp *.jpg *.tif)")); //选择路径
    string savePath = filename.toStdString();
    if(slicImg.empty()){
        imwrite( savePath,image);
    }
    else
        imwrite( savePath,imgSave);
}

//放缩功能
void MainWindow::on_resizeSlider_valueChanged(int value)
{
    Mat r;//缩放后的图片
    if(!image.empty()){
        if(imgSave.empty()){
            Mat rSlic =image.clone();
            fx=value>0?(double)value+1:1.0/(-(double)value);
            fy=fx;
            //saveXmax=ui->scrollImage->horizontalScrollBar()->maximum();
            //saveYmax=ui->scrollImage->verticalScrollBar()->maximum();
            cv::resize(rSlic,r,Size(0,0),fx,fy);
            DisplayMat(r);
            //ui->scrollImage
        }
        else{
            Mat rSlic =imgSave.clone();
            fx=value>0?(double)value+1:1.0/(-(double)(value-1));
            fy=fx;
            //saveXmax=ui->scrollImage->horizontalScrollBar()->maximum();
            //saveYmax=ui->scrollImage->verticalScrollBar()->maximum();
            cv::resize(rSlic,r,Size(0,0),fx,fy);
            DisplayMat(r);
        }
    }
}

//交互事件处理
bool MainWindow::eventFilter(QObject *obj, QEvent *event){
    if(!mask_3.empty()&&obj==ui->scrollImage){
        //绘制事件
        if(event->type()==QEvent::MouseButtonPress){
        QMouseEvent *mouseEvent=static_cast<QMouseEvent *>(event);
        QPoint qrealPoint=mouseEvent->globalPos();
        qrealPoint= ui->scrollImage->mapFromGlobal(qrealPoint);
        x_offset=ui->scrollImage->horizontalScrollBar()->value();//取bar偏移量
        y_offset=ui->scrollImage->verticalScrollBar()->value();//取bar偏移量
        QPoint imagePoint(qrealPoint.x()+x_offset,qrealPoint.y()+y_offset);
        Point2i seedPoint(imagePoint.x()/fx, imagePoint.y()/fy);
        Rect ccomp;
        //QMouseEvent *mouseEvent2=static_cast<QMouseEvent *>(event);
        if((mask_3.at<Vec3b>(seedPoint.y,seedPoint.x)[0]!=255
                || mask_3.at<Vec3b>(seedPoint.y,seedPoint.x)[1]!=255
                || mask_3.at<Vec3b>(seedPoint.y,seedPoint.x)[2]!=255)//判断是否点在边界上
                && mouseEvent->buttons()&Qt::LeftButton){
            cv::floodFill(mask_3,seedPoint,color,&ccomp,Scalar(20,20,20),Scalar(20,20,20),4);//填充算法
            Mat imgshow;
            addWeighted(slicImg,alpha,mask_3,1-alpha,0.0,imgshow);
            imgSave=imgshow;
            //slicImg.setTo(Scalar(255, 255,255,0.5), mask);
            cv::resize(imgshow,imgshow,Size(0,0),fx,fy);
            DisplayMat(imgshow);
            qDebug()<<imagePoint.x()<<" "<<imagePoint.y()<<endl;
        }
    }

        else if(event->type()==QEvent::KeyPress){
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            //分割结果与原图的快捷键space
            if(keyEvent->key()==Qt::Key_Space){
                if(isShow){
                    Mat imgshow=image.clone();
                    cv::resize(imgshow,imgshow,Size(0,0),fx,fy);
                    DisplayMat(imgshow);
                    isShow=false;
                }
                else
                {
                    Mat imgshow;
                    addWeighted(slicImg,alpha,mask_3,1-alpha,0.0,imgshow);
                    cv::resize(imgshow,imgshow,Size(0,0),fx,fy);
                    DisplayMat(imgshow);
                    isShow=true;
                }
            }
            //删除标注快捷键D
            else if(keyEvent->key()==Qt::Key_D){
                if(isDelete){
                    color.val[0]=0;
                    color.val[1]=0;
                    color.val[2]=0;
                    isDelete=false;
                }
                else
                {
                    color=saveColor;
                    isDelete=true;
                }

            }
        }
        /*if(event->type()==QEvent::MouseButtonPress){
        QMouseEvent *mouseEvent=static_cast<QMouseEvent *>(event);
        QPoint qrealPoint=mouseEvent->globalPos();
        qrealPoint= ui->scrollImage->mapFromGlobal(qrealPoint);
        x_offset=ui->scrollImage->horizontalScrollBar()->value();
        y_offset=ui->scrollImage->verticalScrollBar()->value();
        QPoint imagePoint(qrealPoint.x()+x_offset,qrealPoint.y()+y_offset);
        Point2i seedPoint(imagePoint.x(), imagePoint.y());
        Rect ccomp;
        if(mouseEvent->buttons()&Qt::RightButton){
                    cv::floodFill(mask_3,seedPoint,CvScalar(0,0,0),&ccomp,Scalar(20,20,20),Scalar(20,20,20),4);
                    Mat imgshow;
                    addWeighted(slicImg,alpha,mask_3,1-alpha,0.0,imgshow);
                    //slicImg.setTo(Scalar(255, 255,255,0.5), mask);
                    DisplayMat(imgshow);
                    qDebug()<<imagePoint.x()<<" "<<imagePoint.y()<<endl;
                }
        }*/
}
}
//保存mask
void MainWindow::on_actionSave_Segmentation_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this,tr("Save Image"),"",tr("Images (*.png *.bmp *.jpg *.tif)")); //选择路径
    string savePath = filename.toStdString();
    if(mask_3.empty()){
        //imwrite( savePath,image);
    }
    else
        imwrite( savePath,mask_3);
}

//加载mask
void MainWindow::on_actionLoad_Segmentation_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,
                           tr("Open Image"),".",tr("Image File (*.jpg *.png *.bmp *.tif)"));
    QTextCodec *code = QTextCodec::codecForName("gb18030");
    std::string name = code->fromUnicode(filename).data();
    mask_3 = imread(name);
    Mat imgshow;
    slicImg=image.clone();
    alpha=ui->textAlpha->toPlainText().toFloat();
    addWeighted(slicImg,alpha,mask_3,1-alpha,0.0,imgshow);
    imgSave=imgshow;
    DisplayMat(imgshow);
    fx=1.0;
    fy=1.0;
}
//颜色选取按钮
void MainWindow::on_btnColor_clicked()
{
    QColorDialog cd;
    QColor c=cd.getRgba();
    color.val[0]=c.blue();
    color.val[1]=c.green();
    color.val[2]=c.red();
    saveColor=color;
}
