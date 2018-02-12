#ifndef GIT_WRAPPER_H
#define GIT_WRAPPER_H

#include <iostream>
#include <memory>
#include <regex>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <wchar.h>
#include <errno.h>

#define MSYS2_BASE L"C:\\msys64"
#define OPT_BASE L"\\opt\\mingw64"

typedef struct version_s {
  int major;
  int minor;
  int revision;
  int win_ver;
  bool operator<(const version_s& rhs) const {
    return major == rhs.major ? minor == rhs.minor ? revision == rhs.revision ?
      win_ver < rhs.win_ver : revision < rhs.revision : minor < rhs.minor : major < rhs.major;
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

errno_t wgetenv_wrapper(const std::wstring& name, std::wstring& value);

#endif
