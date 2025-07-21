//
//  Logging.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2025-07-11.
//

#pragma once
#include "Keyboard.h"
#include <Core/FolderHelper.h>
#include <Core/Rand.h>
#include <fstream>

enum class LogMode { None, Record, Replay };

namespace logging
{
  std::ofstream rec_file;
  std::ifstream rep_file;
  bool log_finished = false;
  
  void setup_logging(LogMode log_mode, const std::string& xcode_log_filepath, const std::string& log_filename, unsigned int& curr_rnd_seed)
  {
    switch (log_mode)
    {
      case LogMode::None:
        break;
      case LogMode::Record:
        folder::delete_file(log_filename);
        rec_file = std::ofstream { log_filename, std::ios::out | std::ios::trunc };
        rec_file << curr_rnd_seed << '\n';
        break;
      case LogMode::Replay:
        std::string log_filepath;
#ifndef _WIN32
        const char* xcode_env = std::getenv("RUNNING_FROM_XCODE");
        if (xcode_env != nullptr)
          log_filepath = xcode_log_filepath;
#endif
        if (log_filepath.empty())
          log_filepath = log_filename;
        else
          log_filepath = folder::join_file_path({ log_filepath, log_filename });
        rep_file = std::ifstream { log_filepath, std::ios::in };
        if (!rep_file.is_open())
        {
          std::cerr << "Error opening log file \"" + log_filepath + "\"!" << std::endl;
          exit(EXIT_FAILURE);
        }
        std::string line;
        if (std::getline(rep_file, line))
        {
          std::istringstream iss(line);
          iss >> curr_rnd_seed;
          rnd::srand(curr_rnd_seed);
        }
        //rep_file >> curr_rnd_seed;
        break;
    }
  }
  
  void update_log_stream(LogMode log_mode, keyboard::KeyPressDataPair& kpdp, keyboard::StreamKeyboard* keyboard, const int frame_count)
  {
    if (log_mode == LogMode::Replay)
    {
      std::string line;
      if (std::getline(rep_file, line))
      {
        std::istringstream iss(line);
        int log_frame_count = 0;
        iss >> log_frame_count;
        if (log_frame_count != frame_count)
        {
          std::cerr << "REPLAY ERROR : Expected frame number " << frame_count << " but received frame number " << log_frame_count << ". Exiting!" << std::endl;
          exit(EXIT_FAILURE);
        }
        std::string log_key_transient = "";
        std::string log_key_held = "";
        iss >> log_key_transient >> log_key_held;
        if (log_key_transient.size() == 1)
          kpdp.transient = log_key_transient[0];
        else if (log_key_transient == "Space")
          kpdp.transient = ' ';
        else if (log_key_transient.size() > 1)
          kpdp.transient = keyboard::string_to_special_key(log_key_transient);
          
        if (log_key_held.size() == 1)
          kpdp.held = log_key_held[0];
        else if (log_key_held == "Space")
          kpdp.held = ' ';
        else if (log_key_held.size() > 1)
          kpdp.held = keyboard::string_to_special_key(log_key_held);
      }
      else
        log_finished = true;
    }
    else if (keyboard != nullptr)
    {
      kpdp = keyboard->readKey();
      if (log_mode == LogMode::Record)
      {
        rec_file << std::to_string(frame_count) << ' ';
        
        auto special_key_transient = keyboard::get_special_key(kpdp.transient);
        if (special_key_transient != keyboard::SpecialKey::None)
          rec_file << keyboard::special_key_to_string(special_key_transient);
        else
        {
          char char_key = keyboard::get_char_key(kpdp.transient);
          if (char_key == ' ')
            rec_file << "Space";
          else if (33 <= char_key && char_key <= 126)
            rec_file << char_key;
          else
            rec_file << "--";
        }
        
        rec_file << ' ';
        
        auto special_key_held = keyboard::get_special_key(kpdp.held);
        if (special_key_held != keyboard::SpecialKey::None)
          rec_file << keyboard::special_key_to_string(special_key_held);
        else
        {
          char char_key = keyboard::get_char_key(kpdp.held);
          if (char_key == ' ')
            rec_file << "Space";
          else if (33 <= char_key && char_key <= 126)
            rec_file << char_key;
          else
            rec_file << "--";
        }
        
        rec_file << '\n';
        rec_file.flush();
      }
    }
  }
}
