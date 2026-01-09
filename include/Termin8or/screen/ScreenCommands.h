//
//  ScreenCommands.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2025-09-09.
//

#pragma once
#include "ScreenCommandsBasic.h"
#include "Styles.h"
#include "ScreenHandler.h"
#include <Core/System.h>

namespace t8
{
  
#ifdef _WIN32
  inline WORD savedAttributes = 0;
#endif
  inline Style orig_style = { Color16::White, Color16::Black };
  
  
  // Function to save current console fg and bg colors.
  void save_terminal_colors()
  {
    if (sys::is_windows_cmd())
    {
#ifdef _WIN32
      HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
      
      CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
      if (GetConsoleScreenBufferInfo(hConsole, &consoleInfo))
      {
        savedAttributes = consoleInfo.wAttributes;
        int bg_color = static_cast<int>(savedAttributes & 0xF0) >> 4;
        orig_style.bg_color = get_color16_win(bg_color);
        
        int fg_color = static_cast<int>(savedAttributes & 0x0F);
        orig_style.fg_color = get_color16_win(fg_color);
      }
      else
        std::cerr << "Error: Unable to get console screen buffer info." << std::endl;
#endif
    }
  }
  
  // Function to restore the saved console colors.
  Style restore_terminal_colors()
  {
    if (sys::is_windows_cmd())
    {
#ifdef _WIN32
      HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
      
      if (!SetConsoleTextAttribute(hConsole, savedAttributes))
        std::cerr << "Error: Unable to restore console text attributes." << std::endl;
#endif
    }
    return orig_style;
  }
  
  inline void disable_stdio_sync()
  {
    static bool once [[maybe_unused]] = []()
    {
      std::ios_base::sync_with_stdio(false);
      return true;
    }();
  }
  
  void begin_screen()
  {
    disable_stdio_sync();
    save_terminal_colors();
    if (sys::is_windows())
      ::term::init_windows_console();
    clear_screen();
    return_cursor();
    hide_cursor();
  }
  
  template<int NR, int NC, typename CharT>
  void end_screen(ScreenHandler<NR, NC, CharT>& sh)
  {
    auto orig_colors [[maybe_unused]] = restore_terminal_colors();
#ifndef __APPLE__
    sh.clear();
    sh.replace_fg_color(orig_colors.fg_color);
    sh.replace_bg_color(orig_colors.bg_color);
    sh.print_screen_buffer(orig_colors.bg_color);
#endif
    restore_cursor();
    show_cursor();
  }
  
}
