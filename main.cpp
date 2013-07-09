#include "MainWindow.h"
#include <QApplication>
#include <QFileOpenEvent>
#include <stdio.h>

#ifdef __APPLE__
class OpenerApp : QApplication
{
public:
  OpenerApp(int argc, char **argv) : QApplication(argc, argv){}
  int exec() { return QApplication::exec(); }
protected:
  virtual bool event(QEvent *e)
  {
    if(loaded == false) if(e->type() == QEvent::FileOpen)
    {
      loaded = true;
      QString path = ((QFileOpenEvent *)e)->file();
      w = new MainWindow(&path);
      w->GeometryLoad();
      w->show();
      return true;
    }
    return QApplication::event(e);
  }
private:
  bool loaded = false;
  MainWindow *w;
};
#endif

int main(int argc, char *argv[])
{
#ifndef __APPLE__
  if(argc < 2) return -1;
  QApplication a(argc, argv);
  MainWindow w;
  w.GeometryLoad();
  w.show();
  return a.exec();
#else
  OpenerApp a(argc, argv);
  return a.exec();
#endif
}
