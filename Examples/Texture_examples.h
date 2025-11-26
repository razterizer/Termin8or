//
//  Texture_examples.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2025-11-24.
//

#pragma once
#include "screen/ScreenCommands.h"
#include "screen/ScreenUtils.h"
#include "drawing/Drawing.h"


namespace texture
{
  template<int NR, int NC>
  using ScreenHandler = t8::ScreenHandler<NR, NC>;
  using StreamKeyboard = t8::StreamKeyboard;
  using KeyPressDataPair = t8::KeyPressDataPair;
  using Color16 = t8::Color16;

  void example1()
  {
    ScreenHandler<20, 40> sh;
    KeyPressDataPair kpdp;
    auto keyboard = std::make_unique<StreamKeyboard>();
    
    // //////////////////////////////
    
    t8::Texture texture;
    texture.load("colors.tx");
    
    // ///////////////////////////////////////////////////////////
    //                        LET's GO !                        //
    // ///////////////////////////////////////////////////////////
        
    t8::begin_screen();
    for (;;)
    {
      t8::return_cursor();
      sh.clear();
      draw_box_textured(sh,
                        -1, -1, texture.size.r + 2, texture.size.c + 2,
                        t8x::SolarDirection::Zenith,
                        texture);
    
      sh.print_screen_buffer(Color16::Black);
      Delay::sleep(0'400'000);
      
      kpdp = keyboard->readKey();
      auto key = t8::get_char_key(kpdp.transient);
      auto lo_key = str::to_lower(key);
      if (lo_key == 'q')
        goto quit;
    }
    
quit:
    t8::end_screen(sh);
  }

}

