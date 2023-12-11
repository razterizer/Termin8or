//
//  Delay.h
//  Terminal Text Lib
//
//  Created by Rasmus Anthin on 2023-11-30.
//

#pragma once
#include <thread>


namespace Delay
{
  // Arguments:
  // int us : microseconds.
  void sleep(int us)
  {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
  }
}
