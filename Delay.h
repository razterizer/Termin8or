//
//  Delay.h
//  Terminal Text Lib
//
//  Created by Rasmus Anthin on 2023-11-30.
//

#pragma once
#ifdef _WIN32
#include <thread>
#else
#include <unistd.h>
#endif


namespace Delay
{
  // Arguments:
  // int us : microseconds.
  // #FIXME: Choose one for all platforms if possible.
  void sleep(int us)
  {
#ifdef _WIN32
    std::this_thread::sleep_for(std::chrono::microseconds(us));
#else
    usleep(us);
#endif
  }
}
