#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileInfo>
#include <QFileInfoList>
#include <QScrollArea>
#include <QLabel>
#include <QPixmap>

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
  void on_btnRotateLeft_clicked();
  void on_btnRotateRight_clicked();
  void on_btnOpenPrev_clicked();
  void on_btnOpenNext_clicked();
  void on_btnZoomOut_clicked();
  void on_btnZoomIn_clicked();
  void on_btnGroup_clicked();
  void on_btnDelete_clicked();

protected:
  void resizeEvent(QResizeEvent *qre);
  bool eventFilter(QObject *obj, QEvent *qe);

private:
  void LoadPath(QFileInfo &qfi);
  void ReadDirectory();
  int  GetIndexOf(QString &path);
  void OffsetIndex(int offset);
  void Rotate(int degrees, bool save=true);
  void Zoom(int factor);
  void AddToGroup();
  void DeleteFile();
  void SetPicture(QString &path);
  void CalcFitZoom();
  void ResizePicture();

  Ui::MainWindow *ui;
  QString         currDirectory;
  QFileInfoList   currFileList;
  int             currFileIndex;
  QWidget        *pictContainer = NULL;
  QScrollArea    *pictScroller  = NULL;
  QLabel         *pictLabel     = NULL;
  QPixmap        *currPixmap    = NULL;
  double          currZoom      = 0.0;
};

#endif // MAINWINDOW_H
