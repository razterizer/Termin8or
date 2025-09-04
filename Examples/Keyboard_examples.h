//
//  Keyboard_examples.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-11-01.
//

#pragma once
#include "../Keyboard.h"
#include <Core/Benchmark.h>
#include <Core/Delay.h>


namespace keyboard
{

  void example1()
  {
    using namespace t8;
  
    t8::input::KeyPressDataPair kpdp;
    //  D        FPS    I
    //   1 ms => 1000 => 150
    //  10 ms =>  100 => 20
    //  50 ms =>   20 => 4
    // 100 ms =>   10 => 3
    // D = 1000 / FPS.
    auto keyboard = std::make_unique<t8::input::StreamKeyboard>(4);
  
    for (int i = 0; i < 1500; ++i)
    {
      benchmark::tic();
      
      Delay::sleep(0'50'000);
      
      kpdp = keyboard->readKey();
      auto key = input::get_char_key(kpdp.transient);
      auto lo_key = str::to_lower(key);
      if (lo_key == 'q')
        break;
        
     auto time_ms = benchmark::toc();
     
     std::cout << get_key_description(kpdp.held) << " | " << get_key_description(kpdp.transient) << " : " << time_ms << " ms" << std::endl;
    }
  }

}
