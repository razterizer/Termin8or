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
#include <Core/FolderHelper.h>

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
  
  std::string_view path_to_exe;
  const bool use_wasd_arrow_keys = false;
  const Text::Color c_bg_color_default = Text::Color::Default;
  const Text::Color c_bg_color_title = Text::Color::Default;
  const Text::Color c_bg_color_instructions = Text::Color::Default;
  
  int delay = 50'000; // 100'000 (10 FPS) // 60'000 (16.67 FPS);
  int fps = 12; // 5
  
  YesNoButtons quit_confirm_button = YesNoButtons::No;
  
  std::vector<HiScoreItem> hiscore_list;
  int score = 0;
  HiScoreItem curr_score_item;
  int hiscore_caret_idx = 0;
  
  bool handle_hiscores(const HiScoreItem& curr_hsi)
  {
    const int c_max_num_hiscores = 20;
    const auto [exe_path, exe_file] = folder::split_file_path(std::string(path_to_exe));
    const std::string c_file_path = folder::join_file_path({ exe_path, "hiscores.txt" });
  
    // Read saved hiscores.
    std::vector<std::string> lines;
    if (std::filesystem::exists(c_file_path))
    {
      if (!TextIO::read_file(c_file_path, lines))
        return false;
    }
    
    // Import saved hiscores from vector of strings.
    hiscore_list.clear();
    for (const auto& hs_str : lines)
    {
      std::istringstream iss(hs_str);
      HiScoreItem hsi;
      iss >> hsi.name >> hsi.score;
      hiscore_list.emplace_back(hsi);
    }
    
    // Add current hiscore.
    hiscore_list.push_back(curr_hsi);
    
    // Sort hiscores.
    auto num_hiscores = static_cast<int>(hiscore_list.size());
    stlutils::sort(hiscore_list,
      [](const auto& hsi_A, const auto& hsi_B) { return hsi_A.score > hsi_B.score; });
    if (num_hiscores >= 1)
      hiscore_list = stlutils::subset(hiscore_list, 0, std::min(num_hiscores, c_max_num_hiscores) - 1);
    else
    {
      std::cerr << "ERROR: Unknown error while saving hiscores!" << std::endl;
      return false;
    }
    
    // Export hiscores to vector of strings.
    lines.clear();
    for (const auto& hsi : hiscore_list)
    {
      std::ostringstream oss;
      oss << str::trim_ret(hsi.name) << " " << hsi.score;
      lines.emplace_back(oss.str());
    }
    
    // Save hiscores.
    if (!TextIO::write_file(c_file_path, lines))
      return false;
    
    return true;
  }
  
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
  
  int& ref_score() { return score; }
  
  // Callbacks
  virtual void update() = 0;
  virtual void draw_title() = 0;
  virtual void draw_instructions() = 0;
  virtual void on_quit() {}
  virtual void on_exit_title() {}
  virtual void on_exit_instructions() {}
  virtual void on_enter_game_over() {}
  virtual void on_exit_game_over() {}
  virtual void on_enter_you_won() {}
  virtual void on_exit_you_won() {}
  virtual void on_enter_input_hiscore() {}
  virtual void on_exit_input_hiscore() {}
  virtual void on_enter_hiscores() {}
  
public:
  GameEngine(std::string_view exe_path,
             bool use_wasd_keys,
             const Text::Color bg_col_def,
             const Text::Color bg_col_title,
             const Text::Color bg_col_instr)
    : path_to_exe(exe_path)
    , use_wasd_arrow_keys(use_wasd_keys)
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
  
  void set_state_game_over() { show_game_over = true; }
  void set_state_you_won() { show_you_won = true; }
  
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
      
    if (show_quit_confirm && !show_hiscores)
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
      else if (show_game_over)
      {
        if (game_over_timer == 0)
          draw_game_over(sh);
        else
        {
          game_over_timer--;
          if (game_over_timer == 0)
            on_enter_game_over();
        }
        
        update();
          
        if (kpd.curr_key == ' ')
        {
          on_exit_game_over();
          show_game_over = false;
          show_input_hiscore = true;
          curr_score_item.reset(score);
          hiscore_caret_idx = 0;
          on_enter_input_hiscore();
        }
      }
      else if (show_you_won)
      {
        if (you_won_timer == 0)
          draw_you_won(sh);
        else
        {
          you_won_timer--;
          if (you_won_timer == 0)
            on_enter_you_won();
        }
        
        update();
        
        if (kpd.curr_key == ' ')
        {
          on_exit_you_won();
          show_you_won = false;
          show_input_hiscore = true;
          curr_score_item.reset(score);
          hiscore_caret_idx = 0;
          on_enter_input_hiscore();
        }
      }
      else if (show_input_hiscore)
      {
        if (draw_input_hiscore(sh, kpd, curr_score_item, hiscore_caret_idx, anim_ctr))
        {
          on_exit_input_hiscore();
          handle_hiscores(curr_score_item);
          show_input_hiscore = false;
          show_hiscores = true;
          on_enter_hiscores();
        }
      }
      else if (show_hiscores)
      {
        draw_hiscores(sh, hiscore_list);
        
        if (kpd.curr_key == ' ')
        {
          restore_cursor();
          on_quit();
          return false;
        }
      }
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
