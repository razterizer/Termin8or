#pragma once
#include "SpriteHandler.h"
#include "Keyboard.h"
#include <Core/TextIO.h>
#include <cmath>
#ifdef _WIN32
#define NOMINMAX // Should fix the std::min()/max() and std::numeric_limits<T>::min()/max() compilation problems.
#include <windows.h>
#endif

// Game Over
int game_over_timer = 10;
int you_won_timer = 10;
// Wavey Text
float wave_x0 = 0.f;
float wave_f = 0.4f;
float wave_a = 5.f;
float wave_step = 0.1f;
// Misc
const float pix_ar = 1.9f;//1.5f; // height/width of "pixel".
const float pix_ar_sq = pix_ar*pix_ar;
const float pix_ar2 = 1.5f;
const float pix_ar2_sq = pix_ar2*pix_ar;


void clear_screen()
{
#ifdef _WIN32
  // Too slow and not necessary.
  //HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
  //COORD coord = { 0, 0 };
  //DWORD count;
  //CONSOLE_SCREEN_BUFFER_INFO csbi;
  //
  //GetConsoleScreenBufferInfo(hStdOut, &csbi);
  //FillConsoleOutputCharacterA(hStdOut, ' ', csbi.dwSize.X * csbi.dwSize.Y, coord, &count);
  //SetConsoleCursorPosition(hStdOut, coord);
#else
  printf("\x1b[2J");
#endif
}

void return_cursor()
{
#ifdef _WIN32
  HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
  COORD coord = { 0, 0 };
  SetConsoleCursorPosition(hStdOut, coord);
#else
  //printf("\x1b[H");
  std::cout << "\033[0;0H"; // Works on Ubuntu! #NOTE: must be std::cout instead of printf() here!
#endif
}

void restore_cursor()
{
#ifndef _WIN32
  printf("\x1b[2J");
#endif
}

void gotorc(int r, int c)
{
#ifdef _WIN32
  HANDLE hStdOut;
  COORD coord;
  hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
  coord.X = c;
  coord.Y = r;
  SetConsoleCursorPosition(hStdOut, coord);
#else
  printf("%c[%d;%df", 0x1B, r, c);
#endif
}

template<int NR, int NC>
void draw_frame(SpriteHandler<NR, NC>& sh, Text::Color fg_color)
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

// http://www.network-science.de/ascii/
// http://patorjk.com/software/taag/#p=display&f=Graffiti&t=Game%20Over Graffiti
//  ________    _____      _____  ___________
// /  _____/   /  _  \    /     \ \_   _____/
///   \  ___  /  /_\  \  /  \ /  \ |    __)_
//\    \_\  \/    |    \/    Y    \|        \
// \______  /\____|__  /\____|__  /_______  /
//        \/         \/         \/        \/
//____________   _________________________
//\_____  \   \ /   /\_   _____/\______   \
// /   |   \   Y   /  |    __)_  |       _/
///    |    \     /   |        \ |    |   \
//\_______  /\___/   /_______  / |____|_  /
//        \/                 \/         \/
template<int NR, int NC>
void draw_game_over(SpriteHandler<NR, NC>& sh)
{
  auto wave_func = [](float c, int i)
  {
    float x = static_cast<float>(i)/12.f;
    auto x1 = c + wave_a*std::sin(wave_f*math::c_2pi*(x + wave_x0));

    return std::round(x1);
  };

  int c = 18;
  sh.write_buffer("  ________    _____      _____  ___________", 7,  wave_func(c, 0), Text::Color::DarkRed, Text::Color::White);
  sh.write_buffer(" /  _____/   /  _  \\    /     \\ \\_   _____/", 8,  wave_func(c, 1), Text::Color::DarkRed, Text::Color::Yellow);
  sh.write_buffer("/   \\  ___  /  /_\\  \\  /  \\ /  \\ |    __)_ ",9,  wave_func(c, 2), Text::Color::DarkRed, Text::Color::Yellow);
  sh.write_buffer("\\    \\_\\  \\/    |    \\/    Y    \\|        \\", 10, wave_func(c, 3), Text::Color::DarkRed, Text::Color::DarkYellow);
  sh.write_buffer(" \\______  /\\____|__  /\\____|__  /_______  /", 11, wave_func(c, 4), Text::Color::DarkRed, Text::Color::DarkYellow);
  sh.write_buffer("        \\/         \\/         \\/        \\/ ", 12, wave_func(c, 5), Text::Color::DarkRed, Text::Color::DarkYellow);
  sh.write_buffer("____________   _________________________   ", 13, wave_func(c, 6), Text::Color::DarkRed, Text::Color::DarkYellow);
  sh.write_buffer("\\_____  \\   \\ /   /\\_   _____/\\______   \\  ", 14, wave_func(c, 7), Text::Color::DarkRed, Text::Color::DarkYellow);
  sh.write_buffer(" /   |   \\   Y   /  |    __)_  |       _/  ", 15, wave_func(c, 8), Text::Color::DarkRed, Text::Color::DarkYellow);
  sh.write_buffer("/    |    \\     /   |        \\ |    |   \\  ", 16, wave_func(c, 9), Text::Color::DarkRed, Text::Color::Yellow);
  sh.write_buffer("\\_______  /\\___/   /_______  / |____|_  /  ", 17, wave_func(c, 10), Text::Color::DarkRed, Text::Color::Yellow);
  sh.write_buffer("        \\/                 \\/         \\/   ", 18, wave_func(c, 11), Text::Color::DarkRed, Text::Color::White);

  wave_x0 += wave_step;
  if (std::abs(wave_x0 - 100.f) < 1e-4f)
    wave_x0 = 0.f;
}

//_____.___.               __      __            ._.
//\__  |   | ____  __ __  /  \    /  \____   ____| |
// /   |   |/  _ \|  |  \ \   \/\/   /  _ \ /    \ |
// \____   (  <_> )  |  /  \        (  <_> )   |  \|
// / ______|\____/|____/    \__/\  / \____/|___|  /_
// \/                            \/             \/\/
template<int NR, int NC>
void draw_you_won(SpriteHandler<NR, NC>& sh)
{
  wave_f = 1.5f;//0.4f;
  wave_a = 1.f;//5.f;
  wave_step = 0.07f; //0.1f;

  auto wave_func = [](float c, int i)
  {
    float x = static_cast<float>(i)/12.f;
    auto x1 = c + wave_a*std::sin(wave_f*math::c_2pi*(x + wave_x0));

    return std::round(x1);
  };

  int c = 15;
  sh.write_buffer("_____.___.               __      __            ._.", 10, wave_func(c, 0), Text::Color::DarkBlue, Text::Color::Cyan);
  sh.write_buffer("\\__  |   | ____  __ __  /  \\    /  \\____   ____| |", 11, wave_func(c, 1), Text::Color::DarkBlue, Text::Color::Cyan);
  sh.write_buffer(" /   |   |/  _ \\|  |  \\ \\   \\/\\/   /  _ \\ /    \\ |", 12, wave_func(c, 2), Text::Color::DarkBlue, Text::Color::Cyan);
  sh.write_buffer(" \\____   (  <_> )  |  /  \\        (  <_> )   |  \\|", 13, wave_func(c, 3), Text::Color::DarkBlue, Text::Color::DarkCyan);
  sh.write_buffer(" / ______|\\____/|____/    \\__/\\  / \\____/|___|  /_", 14, wave_func(c, 4), Text::Color::DarkBlue, Text::Color::DarkCyan);
  sh.write_buffer(" \\/                            \\/             \\/\\/", 15, wave_func(c, 5), Text::Color::DarkBlue, Text::Color::DarkCyan);

  wave_x0 += wave_step;
  if (std::abs(wave_x0 - 100.f) < 1e-4f)
    wave_x0 = 0.f;
}

template<int NR, int NC>
void draw_paused(SpriteHandler<NR, NC>& sh, int anim_ctr)
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
  sh.write_buffer(msg, 15, 36, Text::Color::White, Text::Color::DarkCyan);
}

enum class YesNoButtons { No = 0, Yes = 1 };
template<int NR, int NC>
void draw_confirm_quit(SpriteHandler<NR, NC>& sh, YesNoButtons button)
{
  const auto nr = static_cast<int>(NR);
  const auto nc = static_cast<int>(NC);
  std::string msg = "Are you sure you want to quit?";
  auto msg_len = static_cast<int>(msg.length());
  sh.write_buffer(msg, nr/2 - 2, (nc - msg_len)/2, Text::Color::Black, Text::Color::DarkCyan);
  // "[Yes]      [No]"
  std::string yes = "[Yes]";
  std::string no = "[No]";
  const auto yes_len = static_cast<int>(yes.length());
  const auto no_len = static_cast<int>(no.length());
  Text::Color bg_color_yes = (button == YesNoButtons::Yes) ?
    Text::Color::Cyan : Text::Color::DarkCyan;
  Text::Color bg_color_no = (button == YesNoButtons::No) ?
    Text::Color::Cyan : Text::Color::DarkCyan;
  sh.write_buffer(yes, nr/2 + 1, (nc - 6)/2 - yes_len, Text::Color::Black, bg_color_yes);
  sh.write_buffer(no, nr/2 + 1, (nc - 6)/2 + no_len, Text::Color::Black, bg_color_no);
  msg = "Press arrow keys to select choice and then [Enter] key to confirm.";
  msg_len = static_cast<int>(msg.length());
  sh.write_buffer(msg, nr/2 + 5, (nc - msg_len)/2, Text::Color::White, Text::Color::DarkCyan);
}

static const int c_hiscore_name_len = 8;

struct HiScoreItem
{
  std::string name;
  int score = 0;
  
  void reset(int new_score)
  {
    name = str::rep_char(' ', c_hiscore_name_len);
    score = new_score;
  }
};

template<int NR, int NC>
bool draw_input_hiscore(SpriteHandler<NR, NC>& sh,
                        const keyboard::KeyPressData& kpd,
                        HiScoreItem& hsi, int& caret_idx)
{
  const auto nr = static_cast<int>(NR);
  const auto nc = static_cast<int>(NC);
  const int num_max_name_chars = 5;
  const int r = nr/2 - 2;
  std::string msg = "Enter your name: ";
  auto msg_len = static_cast<int>(msg.length());
  const int c_base = (nc - msg_len)/2;
  const int c_prompt = c_base + msg_len;
  
  sh.write_buffer(msg, r, c_base, Text::Color::Green, Text::Color::Black);
  
  if (str::is_letter(kpd.curr_key) || kpd.curr_key == ' ')
  {
    if (hsi.name.length() >= caret_idx + 1)
      hsi.name[caret_idx] = str::to_upper(kpd.curr_key);
  }
  
  sh.write_buffer(stlutils::try_get(hsi.name, caret_idx, ' '), r, c_prompt + caret_idx, Text::Color::Green, Text::Color::DarkGreen);
  sh.write_buffer(hsi.name, r, c_prompt, Text::Color::Green, Text::Color::Black);
  
  msg = "Press arrow keys to change between characters,";
  msg_len = static_cast<int>(msg.length());
  sh.write_buffer(msg, nr/2 + 5, (nc - msg_len)/2, Text::Color::DarkGreen, Text::Color::Black);
  msg = "then press the [Enter] key to confirm.";
  msg_len = static_cast<int>(msg.length());
  sh.write_buffer(msg, nr/2 + 6, (nc - msg_len)/2, Text::Color::DarkGreen, Text::Color::Black);
  
  if (kpd.curr_special_key == keyboard::SpecialKey::Left)
  {
    caret_idx--;
    if (caret_idx < 0)
      caret_idx = 0;
  }
  else if (kpd.curr_special_key == keyboard::SpecialKey::Right)
  {
    caret_idx++;
    if (caret_idx >= num_max_name_chars)
      caret_idx = num_max_name_chars - 1;
  }
  
  return kpd.curr_special_key == keyboard::SpecialKey::Enter;
}

template<int NR, int NC>
void draw_hiscores(SpriteHandler<NR, NC>& sh, const std::vector<HiScoreItem>& hiscore_list)
{
  const auto nr = static_cast<int>(NR);
  const auto nc = static_cast<int>(NC);
  std::string msg = "-=* HIGH SCORES *=-";
  auto msg_len = static_cast<int>(msg.length());
  int r_title = 3;
  int c_title = (nc - msg_len)/2;
  sh.write_buffer(msg, r_title, c_title, Text::Color::Green, Text::Color::Black);

  int r = r_title + 2;
  const int c_score_len = 10; // 8
  const int c_padding = 2;
  const int c_score = nc/2 - c_score_len - c_padding;
  const int c_name = nc/2 + c_padding;
  for (const auto& hsi : hiscore_list)
  {
    if (r + 3 >= nr)
      break;
    msg = std::to_string(hsi.score);
    msg = str::adjust_str(msg, str::Adjustment::Right, c_score_len, 0, '0');
    sh.write_buffer(msg, r, c_score, Text::Color::Green, Text::Color::Black);
    
    msg = str::trim_ret(hsi.name);
    msg += str::rep_char('.', c_hiscore_name_len - static_cast<int>(msg.length()));
    sh.write_buffer(msg, r, c_name, Text::Color::Green, Text::Color::Black);
    
    r++;
  }
  
  msg = "Press space-bar to quit...";
  msg_len = static_cast<int>(msg.length());
  sh.write_buffer(msg, nr - 2, (nc - msg_len)/2, Text::Color::DarkGreen, Text::Color::Black);
}
