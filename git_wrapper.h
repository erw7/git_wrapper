#ifndef GIT_WRAPPER_H
#define GIT_WRAPPER_H

#include <iostream>
#include <memory>
#include <regex>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <wchar.h>
#include <errno.h>

std::wstring msys2_base = L"C:\\msys64";
std::wstring opt_base = msys2_base + L"\\opt\\mingw64";

typedef struct version_s {
  int major;
  int minor;
  int revision;
  bool operator<(const version_s& rhs) const {
    return major == rhs.major ? minor == rhs.minor ?
      revision < rhs.revision : minor < rhs.minor : major < rhs.major;
  }
} version_t;

typedef struct git_path_s {
  std::wstring path;
  version_t ver;
  bool operator<(const git_path_s& rhs) const {
    return ver < rhs.ver;
  }
} git_path_t;

namespace fs = boost::filesystem;

#endif