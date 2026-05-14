//
//  TextureFileCommon.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-05-14.
//

#pragma once
#include <Core/FolderHelper.h>
#include <Core/StringHelper.h>

namespace t8
{
  
  // Retrieves file extennsion in lower case.
  inline std::string get_file_ext(const std::string& file_path)
  {
    return str::to_lower(folder::split_filename_ext(file_path).second);
  }
  
}
