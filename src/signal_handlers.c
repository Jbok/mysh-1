#include "signal_handlers.h"
#include <signal.h>

void catch_sigint(int signalNo)
{
  // TODO: File this!
  signal(signalNo,SIG_IGN);  //SIG_DFL : Do default active
}			     //SIG_IGN : Ignore Signal


void catch_sigtstp(int signalNo)
{
  // TODO: File this!
  signal(signalNo,SIG_IGN);
}
