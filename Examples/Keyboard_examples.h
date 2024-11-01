//
//  Keyboard_examples.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-11-01.
//

#pragma once
#include "../Keyboard.h"
#include <Core/Benchmark.h>


namespace keyboard
{

  void example1()
  {
  
    KeyPressData kpd;
    auto keyboard = std::make_unique<StreamKeyboard>();
  
    for (int i = 0; i < 1000; ++i)
    {
      benchmark::tic();
      
      kpd = keyboard->readKey();
      auto key = keyboard::get_char_key(kpd);
      auto lo_key = str::to_lower(key);
      if (lo_key == 'q')
        break;
        
     auto time_ms = benchmark::toc();
     
     std::cout << get_key_description(kpd) << " : " << time_ms << " ms" << std::endl;
    }
  }

}
