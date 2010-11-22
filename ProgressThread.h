#ifndef PROGRESSTHREAD_H
#define PROGRESSTHREAD_H

#include <QThread>

#include "ImageGraphCutBase.h"

class CProgressThread : public QThread
{
Q_OBJECT
public:
  void run();
  void exit();

  ImageGraphCutBase* GraphCut;

signals:
  void StartProgressSignal();
  void StopProgressSignal();
};

#endif