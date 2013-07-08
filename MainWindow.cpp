#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QKeyEvent>
#include <QStandardPaths>

#define TOOLBAR_HEIGHT 50

//----------------------------------------------------------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  qApp->installEventFilter(this);

  pictContainer = new QWidget();
  ui->verticalLayout->insertWidget(0, pictContainer);

  pictLabel = new QLabel();
  pictLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  pictLabel->setScaledContents(true);

  pictScroller = new QScrollArea(pictContainer);
  pictScroller->setWidget(pictLabel);

  QStringList args = qApp->arguments();
  QFileInfo qfi(args.at(1));
  LoadPath(qfi);
}

//----------------------------------------------------------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
  delete ui;
}

//----------------------------------------------------------------------------------------------------------------------------------
//LoadPath (file or directory) - used for application startup arguments, or for D&D
void MainWindow::LoadPath(QFileInfo &qfi)
{
  if(qfi.exists() == false) return;
  if(qfi.isDir())
  {
      currDirectory = qfi.absolutePath();
      ReadDirectory();
      OffsetIndex(1);
  }
  else if(qfi.isFile())
  {
      currDirectory = qfi.absoluteDir().absolutePath();
      ReadDirectory();
      QString filePath = qfi.absoluteFilePath();
      int idx = GetIndexOf(filePath);
      if(idx == -1)
        OffsetIndex(1);
      else
        OffsetIndex(idx+1);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
//ReadDirectory - populate currFileList, and reset currFileIndex to -1, for currDirectory path
void MainWindow::ReadDirectory()
{
  currFileList.clear();
  currFileIndex = -1;
  QString empty = "";
  SetPicture(empty);

  QDir qd(currDirectory);
  QStringList filters; filters << "*.jpg" << "*.png";
  currFileList = qd.entryInfoList(filters, QDir::Files | QDir::NoDot | QDir::NoDotDot | QDir::Readable, QDir::Name);
}

//----------------------------------------------------------------------------------------------------------------------------------
//GetIndexOf - find index of specified path in currFileList (on not found, -1 returned)
int MainWindow::GetIndexOf(QString &path)
{
  int max = currFileList.size();
  for(int i=0; i<max; i++)
    if(currFileList.at(i).absoluteFilePath().compare(path) == 0)
      return i;
  return -1;
}

//----------------------------------------------------------------------------------------------------------------------------------
//OffsetIndex - offset currFileIndex by offset amount (and load respective picture)
void MainWindow::OffsetIndex(int offset)
{
  int max = currFileList.size();
  if(max == 0) { currFileIndex = -1; QString empty=""; SetPicture(empty); return; }
  offset += currFileIndex;
  while(offset <   0) offset += max;
  while(offset >= max) offset -= max;
  currFileIndex = offset;
  QString currFile = currFileList.at(currFileIndex).absoluteFilePath();
  SetPicture(currFile);
}

//----------------------------------------------------------------------------------------------------------------------------------
//Toolbar buttons
void MainWindow::on_btnRotateLeft_clicked()  { Rotate(-90);     } // e
void MainWindow::on_btnRotateRight_clicked() { Rotate( 90);     } // r
void MainWindow::on_btnOpenPrev_clicked()    { OffsetIndex(-1); } // Left-Arrow
void MainWindow::on_btnOpenNext_clicked()    { OffsetIndex( 1); } // Right-Arrow
void MainWindow::on_btnZoomOut_clicked()     { Zoom(-1);        } // -/_
void MainWindow::on_btnZoomIn_clicked()      { Zoom( 1);        } // +/=
void MainWindow::on_btnGroup_clicked()       { AddToGroup();    } // (no shortcut)
void MainWindow::on_btnDelete_clicked()      { DeleteFile();    } // (no shortcut)

//----------------------------------------------------------------------------------------------------------------------------------
//Global keyboard handler
bool MainWindow::eventFilter(QObject *obj, QEvent *qe)
{
  if(qe->type() == QEvent::KeyRelease)
  {
    QKeyEvent *keyEvent = (QKeyEvent *)qe;
    switch(keyEvent->key())
    {
      case Qt::Key_Left : OffsetIndex(-1); return true;
      case Qt::Key_Right: OffsetIndex( 1); return true;
      case Qt::Key_E    : Rotate(-90);     return true;
      case Qt::Key_R    : Rotate( 90);     return true;
      case Qt::Key_Equal: Zoom( 1);        return true;
      case Qt::Key_Minus: Zoom(-1);        return true;
    }
  }
  return false;
}

//----------------------------------------------------------------------------------------------------------------------------------
//Window resize events
void MainWindow::resizeEvent(QResizeEvent *qre)
{
  if(currZoom < 1)
    CalcFitZoom();
  ResizePicture();
}

//----------------------------------------------------------------------------------------------------------------------------------
void MainWindow::Rotate(int degrees, bool save)
{
  if(currPixmap == NULL) return;

  QTransform qtf; qtf.rotate(degrees);
  QPixmap *oldPixmap = currPixmap;
  currPixmap = new QPixmap(oldPixmap->transformed(qtf));
  delete oldPixmap;
  pictLabel->setPixmap(*currPixmap);
  CalcFitZoom();
  if(currZoom > 1.0) currZoom = 1.0;
  ResizePicture();
  if(save == false) return;

  QFile qf(currFileList.at(currFileIndex).absoluteFilePath());
  if(qf.open(QIODevice::WriteOnly) == false)
    { QMessageBox::critical(this, "Rotate Re-Save Error", "Unable to save rotated image.", QMessageBox::Ok); return; }
  if(currPixmap->save(&qf) == false)
    { QMessageBox::critical(this, "Rotate Re-Save Error", "Unable to save rotated image.", QMessageBox::Ok); return; }
  qf.close();
}

//----------------------------------------------------------------------------------------------------------------------------------
void MainWindow::Zoom(int factor)
{
  currZoom += 0.0625 * (double)factor;
  ResizePicture();
}

//----------------------------------------------------------------------------------------------------------------------------------
void MainWindow::AddToGroup()
{
  //verify file selection
  if(currFileIndex == -1) return;
  if(currFileList.size() == 0) return;

  //verify upload group
  QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
  desktop += "/uploadGroup";
  QDir qd(desktop);
  if(!qd.exists())
  {
    if(qd.mkdir(desktop) == false)
    {
      QMessageBox::critical(this, "Upload Group Error", "Unable to create upload group folder.", QMessageBox::Ok);
      return;
    }
  }

  //copy source file
  QFileInfo qfi = currFileList.at(currFileIndex);
  QFile qf(qfi.absoluteFilePath());
  desktop += "/" + qfi.fileName();
  bool success = qf.copy(desktop);
  if(success == false)
  {
    QMessageBox::critical(this, "Upload Group Error", "Unable to copy file to upload group folder.", QMessageBox::Ok);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
void MainWindow::DeleteFile()
{
  if(currFileIndex == -1) return;
  if(currFileList.size() == 0) return;
  QFileInfo qfi = QFileInfo(currFileList.at(currFileIndex).absoluteFilePath());
  if(QMessageBox::question(this,
                           "Delete File",
                           "Really delete " + qfi.fileName() + "?",
                           QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;
  QFile qf(qfi.absoluteFilePath());
  bool result = qf.remove();
  if(result == false)
  {
    QMessageBox::information(this, "Delete File", "Unable to delete file.", QMessageBox::Ok);
    return;
  }
  if(currFileIndex > 0)
    OffsetIndex(-1);
  else
    OffsetIndex(1);
}

//----------------------------------------------------------------------------------------------------------------------------------
void MainWindow::SetPicture(QString &path)
{
  if(path.length() == 0)
  {
    setWindowTitle("QuickPic");
    if(currPixmap) delete currPixmap;
    currPixmap = new QPixmap(0,0);
    pictLabel->setPixmap(*currPixmap);
    return;
  }
  QFileInfo qfi = QFileInfo(path);
  setWindowTitle("QuickPic - " + qfi.fileName());
  if(currPixmap) delete currPixmap;
  currPixmap = new QPixmap;
  currPixmap->load(qfi.absoluteFilePath());
  pictLabel->setPixmap(*currPixmap);
  CalcFitZoom();
  if(currZoom > 1.0) currZoom = 1.0;
  ResizePicture();
}

//----------------------------------------------------------------------------------------------------------------------------------
void MainWindow::CalcFitZoom()
{
  double widthFactor  = (double)width()                   / (double)currPixmap->width();
  double heightFactor = (double)(height()-TOOLBAR_HEIGHT) / (double)currPixmap->height();
  currZoom = widthFactor; if(heightFactor < currZoom) currZoom = heightFactor;
}

//----------------------------------------------------------------------------------------------------------------------------------
void MainWindow::ResizePicture()
{
  int availableWidth  = width();
  int availableHeight = height() - TOOLBAR_HEIGHT;
  pictContainer->resize(availableWidth, availableHeight);

  int requestedWidth  = (int)((double)currPixmap->width()  * currZoom);
  int requestedHeight = (int)((double)currPixmap->height() * currZoom);

  int scrollX=0, scrollY=0, scrollWidth=availableWidth, scrollHeight=availableHeight;
  if(requestedWidth  < (availableWidth+2) ) { scrollX = (availableWidth -requestedWidth )>>1; scrollWidth  = requestedWidth+2;  }
  if(requestedHeight < (availableHeight+2)) { scrollY = (availableHeight-requestedHeight)>>1; scrollHeight = requestedHeight+2; }
  pictScroller->setGeometry(scrollX, scrollY, scrollWidth, scrollHeight);
  pictLabel->resize(requestedWidth, requestedHeight);
}
