#include "ProgressThread.h"

#include <iostream>
#include <cmath>

void CProgressThread::run()
{
  emit StartProgressSignal();

  this->GraphCut->PerformSegmentation();

  exit();
  emit StopProgressSignal();
}

void CProgressThread::exit()
{
  emit StopProgressSignal();
}
