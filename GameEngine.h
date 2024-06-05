//
//  GameEngine.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2023-12-30.
//

#pragma once
#include "Keyboard.h"
#include "Screen.h"
#include <Core/Delay.h>
#include <Core/Rand.h>
#include <Core/Math.h>

template<int NR = 30, int NC = 80>
class GameEngine
{
  bool paused = false;
  bool show_title = true;
  bool show_instructions = false;
  bool show_quit_confirm = false;
  bool show_game_over = false;
  bool show_you_won = false;
  bool show_input_hiscore = false;
  bool show_hiscores = false;
  
  const bool use_wasd_arrow_keys = false;
  const Text::Color c_bg_color_default = Text::Color::Default;
  const Text::Color c_bg_color_title = Text::Color::Default;
  const Text::Color c_bg_color_instructions = Text::Color::Default;
  
  int delay = 50'000; // 100'000 (10 FPS) // 60'000 (16.67 FPS);
  int fps = 12; // 5
  
  YesNoButtons quit_confirm_button = YesNoButtons::No;
  
protected:
  float dt = static_cast<float>(delay) / 1e6f;
  float time = 0.f;
  
  Text t;
  SpriteHandler<NR, NC> sh;
  
  Text::Color bg_color = Text::Color::Default;
  
  int anim_ctr = 0;
  
  keyboard::KeyPressData kpd;
  
  void set_fps(float fps_val) { fps = fps_val; }
  void set_delay_us(float delay_us)
  {
    delay = delay_us;
    dt = static_cast<float>(delay) / 1e6f;
  }
  
  // Callbacks
  virtual void update() = 0;
  virtual void on_quit() {}
  virtual void draw_title() = 0;
  virtual void draw_instructions() = 0;
  virtual void on_exit_title() {}
  virtual void on_exit_instructions() {}
  
public:
  GameEngine(bool use_wasd_keys,
             const Text::Color bg_col_def,
             const Text::Color bg_col_title,
             const Text::Color bg_col_instr)
    : use_wasd_arrow_keys(use_wasd_keys)
    , c_bg_color_default(bg_col_def)
    , c_bg_color_title(bg_col_title)
    , c_bg_color_instructions(bg_col_instr)
  {}
  
  virtual ~GameEngine() = default;

  void init()
  {
    keyboard::enableRawMode();
    
    clear_screen(); return_cursor();
    
    //nodelay(stdscr, TRUE);
    
    rnd::srand_time();
  }
  
  virtual void generate_data() = 0;
  
  void run()
  {
    // RT-Loop
    clear_screen();
    auto update_func = std::bind(&GameEngine::engine_update, this);
    Delay::update_loop(fps, update_func);
  }
  
  int get_fps() const { return fps; }
  int get_delay_us() const { return delay; }
  
private:
  bool engine_update()
  {
    return_cursor();
    sh.clear();
    
    kpd = keyboard::register_keypresses(use_wasd_arrow_keys);
    if (kpd.quit)
    {
      math::toggle(show_quit_confirm);
      quit_confirm_button = YesNoButtons::No;
    }
    else if (kpd.pause)
      math::toggle(paused);
      
    if (show_quit_confirm)
    {
      draw_confirm_quit(sh, quit_confirm_button);
      if (kpd.curr_special_key == keyboard::SpecialKey::Left)
        quit_confirm_button = YesNoButtons::Yes;
      else if (kpd.curr_special_key == keyboard::SpecialKey::Right)
        quit_confirm_button = YesNoButtons::No;
      
      if (kpd.curr_special_key == keyboard::SpecialKey::Enter)
      {
        if (quit_confirm_button == YesNoButtons::Yes)
        {
          restore_cursor();
          on_quit();
          return false;
        }
        else
          show_quit_confirm = false;
      }
    }
    else
    {
      bg_color = c_bg_color_default;
      if (show_title)
      {
        bg_color = c_bg_color_title;
        draw_title();
        if (kpd.curr_key == ' ')
        {
          on_exit_title();
          show_title = false;
          show_instructions = true;
        }
      }
      else if (show_instructions)
      {
        bg_color = c_bg_color_instructions;
        draw_instructions();
        if (kpd.curr_key == ' ')
        {
          on_exit_instructions();
          show_instructions = false;
        }
      }
      else if (paused)
        draw_paused(sh, anim_ctr);
      else
        update();
    }
      
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
