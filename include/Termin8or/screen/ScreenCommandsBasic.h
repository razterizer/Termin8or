//
//  ScreenCommandsBasic.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2025-09-09.
//

#pragma once
#include <stdio.h>
#include <iostream>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX // Should fix the std::min()/max() and std::numeric_limits<T>::min()/max() compilation problems.
#endif
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif


namespace t8
{

  // Clear screen and send cursor to home position.
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
  
  // Send cursor to home position.
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
  
  // Clear screen and send cursor to home position.
  void restore_cursor()
  {
#ifndef _WIN32
    printf("\x1b[2J");
#endif
  }
  
  void hide_cursor()
  {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE; // Hide the cursor
    SetConsoleCursorInfo(hConsole, &cursorInfo);
#else
    std::cout << "\x1B[?25l";
#endif
  }
  
  void show_cursor()
  {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = TRUE; // Show the cursor
    SetConsoleCursorInfo(hConsole, &cursorInfo);
#else
    std::cout << "\x1B[?25h";
#endif
  }
  
  void gotorc(int r, int c)
  {
#ifdef _WIN32
    // Windows uses 0-indexed rows and cols.
    HANDLE hStdOut;
    COORD coord;
    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    coord.X = c;
    coord.Y = r;
    SetConsoleCursorPosition(hStdOut, coord);
#else
    // ANSI uses 1-indexed rows and cols.
    printf("%c[%d;%dH", 0x1B, r + 1, c + 1);
#endif
  }
  
  std::string get_gotorc_str(int r, int c)
  {
#ifdef _WIN32
    // Windows doesn't use ANSI escape codes for cursor movement.
    return "";
#else
    // ANSI escape code for cursor positioning (1-indexed)
    return "\033[" + std::to_string(r + 1) + ";" + std::to_string(c + 1) + "H";
#endif
  }
  
  std::pair<int, int> get_terminal_window_size()
  {
    int rows = 0;
    int cols = 0;
#ifdef _WIN32
    // Windows-specific code.
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
    {
      cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
      rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
    else
      rows = cols = -1; // If we fail to get the size.
#else
    // POSIX (Linux/macOS) specific code.
    struct winsize size;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0)
    {
      rows = size.ws_row;
      cols = size.ws_col;
    }
    else
      rows = cols = -1; // If we fail to get the size.
#endif
    return { rows, cols };
  }
  
  void resize_terminal_window(int nr, int nc)
  {
#ifdef _WIN32
    // Windows-specific code to resize the console window.
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE)
    {
      std::cerr << "Error: Unable to get console handle." << std::endl;
      return;
    }
    
    // Get current console buffer and window size.
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
    {
      std::cerr << "Error: Unable to get console screen buffer info." << std::endl;
      return;
    }
    
    // Adjust window size to prevent shrinking the buffer before the window.
    SMALL_RECT windowSize = { 0, 0, static_cast<SHORT>(nc - 1), static_cast<SHORT>(nr - 1) };
    
    // If new size is larger than buffer, set buffer size first.
    COORD bufferSize = {
      static_cast<SHORT>(std::max(static_cast<SHORT>(nc), csbi.dwSize.X)),
      static_cast<SHORT>(std::max(static_cast<SHORT>(nr), csbi.dwSize.Y)) };
    
    if (!SetConsoleScreenBufferSize(hConsole, bufferSize))
    {
      std::cerr << "Error: Unable to set console screen buffer size." << std::endl;
      return;
    }
    
    // Now set the window size.
    if (!SetConsoleWindowInfo(hConsole, TRUE, &windowSize))
    {
      std::cerr << "Error: Unable to resize console window." << std::endl;
      return;
    }
    
    system("cls"); // Clear the console screen (Windows specific).
#else
    // POSIX (Linux/macOS) code to resize terminal.
    std::cout << "\033[8;" << nr << ";" << nc << "t"; // Resize terminal with ANSI escape sequence.
#endif
  }

}
