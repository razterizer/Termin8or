//
//  Delay.h
//  Terminal Text Lib
//
//  Created by Rasmus Anthin on 2023-11-30.
//

#pragma once
#include <thread>
#include <functional>


namespace Delay
{
  // Arguments:
  // int us : microseconds.
  void sleep(int us)
  {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
  }

  void update_loop(int fps, std::function<bool(void)> update_func)
  {
    std::chrono::milliseconds frame_time(1000 / fps);
    auto last_time = std::chrono::steady_clock::now();

    while (true)
    {
      if (!update_func())
        return;

      // Check time elapsed since the last frame.
      auto current_time = std::chrono::steady_clock::now();
      auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_time);

      if (elapsed_time < frame_time)
      {
        // Sleep to limit the refresh rate.
        std::this_thread::sleep_for(frame_time - elapsed_time);
      }

      //refresh();
      //Delay::sleep(delay);

      last_time = current_time;
    }
  }
}
