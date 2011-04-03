
template <typename TImage>
void ProgressThread<TImage>::run()
{
  // When the thread is started, emit the signal to start the marquee progress bar
  emit StartProgressSignal();

  this->GraphCut->PerformSegmentation();

  // When the function is finished, end the thread
  exit();
}

template <typename TImage>
void ProgressThread<TImage>::exit()
{
  // When the thread is stopped, emit the signal to stop the marquee progress bar
  emit StopProgressSignal();
}
