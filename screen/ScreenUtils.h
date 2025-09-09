#pragma once
#include "ScreenHandler.h"
#include "Styles.h"
#include "../input/Keyboard.h"
//#include <Core/TextIO.h>
#include <cmath>
#include <ranges>

namespace t8
{

  template<int NR, int NC>
  void draw_frame(ScreenHandler<NR, NC>& sh, Color fg_color)
  {
    const int nc_inset = sh.num_cols_inset();
    const int nr_inset = sh.num_rows_inset();
    sh.write_buffer("+" + str::rep_char('-', nc_inset) + "+", 0, 0, fg_color);
    for (int r = 1; r <= nr_inset; ++r)
    {
      sh.write_buffer("|", r, 0, fg_color);
      sh.write_buffer("|", r, nc_inset+1, fg_color);
    }
    sh.write_buffer("+" + str::rep_char('-', nc_inset) + "+", nr_inset+1, 0, fg_color);
  }
  
}

namespace t8x
{

  template<int NR, int NC>
  using ScreenHandler = t8::ScreenHandler<NR, NC>;
  using Color = t8::Color;
  using Style = t8::Style;
  using HiliteFGStyle = t8::HiliteFGStyle;
  using ButtonStyle = t8::ButtonStyle;
  using PromptStyle = t8::PromptStyle;
  using KeyPressData = t8::KeyPressData;
  using SpecialKey = t8::SpecialKey;


  // Game Over / You Won
  int game_over_timer = 10;
  int you_won_timer = 10;
  double timestamp_game_over = 0.f;
  double timestamp_you_won = 0.f;
  const double c_min_time_game_over = 3.f;
  const double c_min_time_you_won = 3.f;
  // Wavey Text
  float wave_x0 = 0.f;
  float wave_f = 0.4f;
  float wave_a = 5.f;
  // Misc
  const float pix_ar = 1.9f;//1.5f; // height/width of "pixel".
  const float pix_ar_sq = pix_ar*pix_ar;
  const float pix_ar2 = 1.5f;
  const float pix_ar2_sq = pix_ar2*pix_ar;
  
  // http://www.network-science.de/ascii/
  // http://patorjk.com/software/taag/#p=display&f=Graffiti&t=Game%20Over Graffiti
  // #   ________    _____      _____  ___________ #
  // #  /  _____/   /  _  \    /     \ \_   _____/ #
  // # /   \  ___  /  /_\  \  /  \ /  \ |    __)_  #
  // # \    \_\  \/    |    \/    Y    \|        \ #
  // #  \______  /\____|__  /\____|__  /_______  / #
  // #         \/         \/         \/        \/  #
  // # ____________   _________________________    #
  // # \_____  \   \ /   /\_   _____/\______   \   #
  // #  /   |   \   Y   /  |    __)_  |       _/   #
  // # /    |    \     /   |        \ |    |   \   #
  // # \_______  /\___/   /_______  / |____|_  /   #
  // #         \/                 \/         \/    #
  template<int NR, int NC>
  void draw_game_over(ScreenHandler<NR, NC>& sh, float wave_step = 0.1f,
                      const Style& line_0_style = { Color::DarkRed, Color::White },
                      const Style& line_1_style = { Color::DarkRed, Color::Yellow },
                      const Style& line_2_style = { Color::DarkRed, Color::DarkYellow },
                      const Style& line_3_style = { Color::DarkRed, Color::Yellow },
                      const Style& line_4_style = { Color::DarkRed, Color::White })
  {
    auto wave_func = [](int c, int i)
    {
      auto x = static_cast<float>(i)/12.f;
      auto x1 = static_cast<float>(c) + wave_a*std::sin(wave_f*math::c_2pi*(x + wave_x0));
      
      return math::roundI(x1);
    };
    
    int c = NC/2 - 22; // 18 = 80/2 - 22
    int r0 = NR/2 - 8; // 7 = 30/2 - 8
    sh.write_buffer("  ________    _____      _____  ___________",        r0 + 0,  wave_func(c, 0), line_0_style);
    sh.write_buffer(" /  _____/   /  _  \\    /     \\ \\_   _____/",     r0 + 1,  wave_func(c, 1), line_1_style);
    sh.write_buffer("/   \\  ___  /  /_\\  \\  /  \\ /  \\ |    __)_ ",   r0 + 2,  wave_func(c, 2), line_1_style);
    sh.write_buffer("\\    \\_\\  \\/    |    \\/    Y    \\|        \\", r0 + 3,  wave_func(c, 3), line_2_style);
    sh.write_buffer(" \\______  /\\____|__  /\\____|__  /_______  /",     r0 + 4,  wave_func(c, 4), line_2_style);
    sh.write_buffer("        \\/         \\/         \\/        \\/ ",    r0 + 5,  wave_func(c, 5), line_2_style);
    sh.write_buffer("____________   _________________________   ",        r0 + 6,  wave_func(c, 6), line_2_style);
    sh.write_buffer("\\_____  \\   \\ /   /\\_   _____/\\______   \\  ",  r0 + 7,  wave_func(c, 7), line_2_style);
    sh.write_buffer(" /   |   \\   Y   /  |    __)_  |       _/  ",       r0 + 8,  wave_func(c, 8), line_2_style);
    sh.write_buffer("/    |    \\     /   |        \\ |    |   \\  ",     r0 + 9,  wave_func(c, 9), line_3_style);
    sh.write_buffer("\\_______  /\\___/   /_______  / |____|_  /  ",      r0 + 10, wave_func(c, 10), line_3_style);
    sh.write_buffer("        \\/                 \\/         \\/   ",     r0 + 11, wave_func(c, 11), line_4_style);
    
    wave_x0 += wave_step;
    if (std::abs(wave_x0 - 100.f) < 1e-4f)
      wave_x0 = 0.f;
  }
  
  // # _____.___.               __      __            ._. #
  // # \__  |   | ____  __ __  /  \    /  \____   ____| | #
  // #  /   |   |/  _ \|  |  \ \   \/\/   /  _ \ /    \ | #
  // #  \____   (  <_> )  |  /  \        (  <_> )   |  \| #
  // #  / ______|\____/|____/    \__/\  / \____/|___|  /_ #
  // #  \/                            \/             \/\/ #
  template<int NR, int NC>
  void draw_you_won(ScreenHandler<NR, NC>& sh, float wave_step = 0.07f,
                    const Style& line_0_style = { Color::DarkBlue, Color::Cyan },
                    const Style& line_1_style = { Color::DarkBlue, Color::DarkCyan })
  {
    wave_f = 1.5f;//0.4f;
    wave_a = 1.f;//5.f;
    
    auto wave_func = [](int c, int i)
    {
      auto x = static_cast<float>(i)/12.f;
      auto x1 = static_cast<int>(c) + wave_a*std::sin(wave_f*math::c_2pi*(x + wave_x0));
      
      return math::roundI(x1);
    };
    
    int c = NC/2 - 25; // 18 = 80/2 - 25
    int r0 = NR/2 - 5; // 10 = 30/2 - 5
    sh.write_buffer("_____.___.               __      __            ._.",        r0 + 0, wave_func(c, 0), line_0_style);
    sh.write_buffer("\\__  |   | ____  __ __  /  \\    /  \\____   ____| |",     r0 + 1, wave_func(c, 1), line_0_style);
    sh.write_buffer(" /   |   |/  _ \\|  |  \\ \\   \\/\\/   /  _ \\ /    \\ |", r0 + 2, wave_func(c, 2), line_0_style);
    sh.write_buffer(" \\____   (  <_> )  |  /  \\        (  <_> )   |  \\|",     r0 + 3, wave_func(c, 3), line_1_style);
    sh.write_buffer(" / ______|\\____/|____/    \\__/\\  / \\____/|___|  /_",    r0 + 4, wave_func(c, 4), line_1_style);
    sh.write_buffer(" \\/                            \\/             \\/\\/",    r0 + 5, wave_func(c, 5), line_1_style);
    
    wave_x0 += wave_step;
    if (std::abs(wave_x0 - 100.f) < 1e-4f)
      wave_x0 = 0.f;
  }
  
  template<int NR, int NC>
  void draw_paused(ScreenHandler<NR, NC>& sh, int anim_ctr, const Style& info_style)
  {
    int anim = anim_ctr % 10;
    std::string msg;
    switch (anim)
    {
      case 6: msg = "PAUSED"; break;
      case 5: msg = "P USED"; break;
      case 4: msg = "P USE "; break;
      case 3: msg = "P U E "; break;
      case 2: msg = "  U E "; break;
      case 1: msg = "  U   "; break;
      case 0: msg = "      "; break;
      default: msg = "PAUSED"; break;
    }
    int r = NR/2;
    int c = (NC - 6)/2; // 36
    sh.write_buffer(msg, r, c, info_style);
  }
  
  enum class YesNoButtons { No = 0, Yes = 1 };
  template<int NR, int NC>
  void draw_confirm(ScreenHandler<NR, NC>& sh,
                    const std::vector<std::string>& titles,
                    YesNoButtons button,
                    const Style& title_style,
                    const ButtonStyle& button_style,
                    const Style& info_style)
  {
    const auto nr = static_cast<int>(NR);
    const auto nc = static_cast<int>(NC);
    int title_row_offs = 2;
    for (const auto& msg : titles | std::views::reverse)
    {
      auto msg_len = static_cast<int>(msg.length());
      sh.write_buffer(msg, nr/2 - title_row_offs, (nc - msg_len)/2, title_style);
      title_row_offs += 2;
    }
    // "[Yes]      [No]"
    std::string yes = "[Yes]";
    std::string no = "[No]";
    const auto yes_len = static_cast<int>(yes.length());
    const auto no_len = static_cast<int>(no.length());
    Color bg_color_yes = (button == YesNoButtons::Yes) ?
    button_style.bg_color_selected : button_style.bg_color;
    Color bg_color_no = (button == YesNoButtons::No) ?
    button_style.bg_color_selected : button_style.bg_color;
    sh.write_buffer(yes, nr/2 + 1, (nc - 6)/2 - yes_len, button_style.fg_color, bg_color_yes);
    sh.write_buffer(no, nr/2 + 1, (nc - 6)/2 + no_len, button_style.fg_color, bg_color_no);
    std::string msg = "Press arrow keys to select choice and then [Enter] key to confirm.";
    auto msg_len = static_cast<int>(msg.length());
    sh.write_buffer(msg, nr/2 + 5, (nc - msg_len)/2, info_style);
  }
  
  static const int c_hiscore_name_max_len = 8;
  
  struct HiScoreItem
  {
    std::string name;
    int score = 0;
    bool current_game = false;
    
    void init(int new_score)
    {
      name = str::rep_char(' ', c_hiscore_name_max_len);
      score = new_score;
      current_game = true;
    }
  };
  
  template<int NR, int NC>
  bool draw_input_hiscore(ScreenHandler<NR, NC>& sh,
                          const KeyPressData& kpd,
                          HiScoreItem& hsi, int& caret_idx, int anim_ctr,
                          const Style& title_style,
                          const PromptStyle& prompt_style,
                          const Style& info_style)
  {
    const auto nr = static_cast<int>(NR);
    const auto nc = static_cast<int>(NC);
    const int r = nr/2 - 2;
    std::string msg = "Enter your name: ";
    auto msg_len = static_cast<int>(msg.length());
    const int c_base = (nc - msg_len)/2;
    const int c_prompt = c_base + msg_len;
    
    sh.write_buffer(msg, r, c_base, title_style);
    
    auto key = get_char_key(kpd);
    auto special_key = get_special_key(kpd);
    
    if (str::is_letter(key) || key == ' ')
    {
      if (static_cast<int>(hsi.name.length()) >= caret_idx + 1)
        hsi.name[caret_idx] = str::to_upper(key);
    }
    
    auto bg_color_caret = (anim_ctr/2) % 2 == 0 ? prompt_style.bg_color_cursor : prompt_style.bg_color;
    sh.write_buffer(hsi.name.substr(0, caret_idx), r, c_prompt, prompt_style);
    sh.write_buffer(hsi.name.substr(caret_idx, 1), r, c_prompt + caret_idx, prompt_style.fg_color, bg_color_caret);
    sh.write_buffer(hsi.name.substr(caret_idx + 1), r, c_prompt + caret_idx + 1, prompt_style);
    
    msg = "Press arrow keys to change between characters,";
    msg_len = static_cast<int>(msg.length());
    sh.write_buffer(msg, nr/2 + 5, (nc - msg_len)/2, info_style);
    msg = "then press the [Enter] key to confirm.";
    msg_len = static_cast<int>(msg.length());
    sh.write_buffer(msg, nr/2 + 6, (nc - msg_len)/2, info_style);
    
    if (special_key == SpecialKey::Left)
    {
      caret_idx--;
      if (caret_idx < 0)
        caret_idx = 0;
    }
    else if (special_key == SpecialKey::Right)
    {
      caret_idx++;
      if (caret_idx >= c_hiscore_name_max_len)
        caret_idx = c_hiscore_name_max_len - 1;
    }
    
    return special_key == SpecialKey::Enter;
  }
  
  template<int NR, int NC>
  void draw_hiscores(ScreenHandler<NR, NC>& sh, const std::vector<HiScoreItem>& hiscore_list,
                     const Style& title_style,
                     const HiliteFGStyle& nr_style,
                     const HiliteFGStyle& score_style,
                     const HiliteFGStyle& name_style,
                     const Style& info_style)
  {
    const auto nr = static_cast<int>(NR);
    const auto nc = static_cast<int>(NC);
    std::string msg = "-=* HIGH SCORES *=-";
    auto msg_len = static_cast<int>(msg.length());
    int r_title = 3;
    int c_title = (nc - msg_len)/2;
    sh.write_buffer(msg, r_title, c_title, title_style);
    
    int r = r_title + 2;
    const int c_score_len = 10; // 8
    const int c_padding = 2;
    const int c_score = 2 + nc/2 - c_score_len - c_padding;
    const int c_name = 2 + nc/2 + c_padding;
    int hs_nr = 1;
    for (const auto& hsi : hiscore_list)
    {
      if (r + 3 >= nr)
        break;
      
      auto nr_fg_color = hsi.current_game ? nr_style.fg_color_hilite : nr_style.fg_color;
      auto score_fg_color = hsi.current_game ? score_style.fg_color_hilite : score_style.fg_color;
      auto name_fg_color = hsi.current_game ? name_style.fg_color_hilite : name_style.fg_color;
      
      msg = std::to_string(hs_nr++);
      msg = str::adjust_str(msg, str::Adjustment::Right, 2, 0, '0');
      msg += ". ";
      sh.write_buffer(msg, r, c_score - 4,
                      nr_fg_color, nr_style.bg_color);
      
      msg = std::to_string(hsi.score);
      msg = str::adjust_str(msg, str::Adjustment::Right, c_score_len, 0, '0');
      sh.write_buffer(msg, r, c_score,
                      score_fg_color, score_style.bg_color);
      
      sh.write_buffer(str::rep_char(' ', c_padding), r, c_score + c_score_len,
                      score_fg_color, score_style.bg_color);
      sh.write_buffer(str::rep_char(' ', c_padding), r, c_score + c_score_len + c_padding,
                      name_fg_color, name_style.bg_color);
      
      msg = str::trim_ret(hsi.name);
      msg += str::rep_char('.', c_hiscore_name_max_len - static_cast<int>(msg.length()));
      sh.write_buffer(msg, r, c_name,
                      name_fg_color, name_style.bg_color);
      
      r++;
    }
    
    msg = "Press space-bar to quit...";
    msg_len = static_cast<int>(msg.length());
    sh.write_buffer(msg, nr - 2, (nc - msg_len)/2, info_style);
  }
  
}
