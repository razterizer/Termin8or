//
//  GameEngine.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2023-12-30.
//

#pragma once
#include "Keyboard.h"
#include "Screen.h"
#include "Core/Delay.h"
#include "Core/Rand.h"

template<int NR = 30, int NC = 80>
class GameEngine
{
protected:
  bool paused = false;
  int delay = 50'000; // 100'000 (10 FPS) // 60'000 (16.67 FPS);
  int fps = 12; // 5
  float dt = static_cast<float>(delay) / 1e6f;
  float time = 0.f;
  
  Text t;
  SpriteHandler<NR, NC> sh;
  
  Text::Color bg_color = Text::Color::Default;
  
  int anim_ctr = 0;
  int key_ctr = 0;
  
  virtual bool update() = 0;
  
public:
  void init()
  {
    enableRawMode();
    
    clear_screen(); return_cursor();
    
    //nodelay(stdscr, TRUE);
    
    rnd::srand_time();
  }
  
  virtual void generate_data() = 0;
  
  void run()
  {
    // RT-Loop
    auto update_func = std::bind(&GameEngine::engine_update, this);
    Delay::update_loop(fps, update_func);
  }
  
  int get_fps() const { return fps; }
  int get_delay_us() const { return delay; }
  
private:
  bool engine_update()
  {
    clear_screen();
    return_cursor();
    sh.clear();
    
    if (!update())
      return false;
      
    sh.print_screen_buffer(t, bg_color);
    //sh.print_screen_buffer_chars();
    //sh.print_screen_buffer_fg_colors();
    //sh.print_screen_buffer_bg_colors();
    
  ///
    
    anim_ctr++;
    
    time += dt;
    
    return true;
  }
};
