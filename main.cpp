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

int wmain(int argc, wchar_t* argv[])
{
  errno_t err;
  size_t len;

  err = _wgetenv_s(&len, NULL, 0, L"MSYS2_BASE");
  if (err == 0 && len) {
    std::unique_ptr<wchar_t[]> wcp(new wchar_t[len]);
    err = _wgetenv_s(&len, wcp.get(), len, L"MSYS2_BASE");
    if (err == 0 && len) {
      msys2_base = wcp.get();
      opt_base = msys2_base + L"\\opt\\mingw64";
    }
  }

  std::vector<std::unique_ptr<git_path_t>> vec;
  std::wregex wre(L".*\\git-(\\d+)\\.(\\d+)\\.(\\d+)");
  std::wsmatch wmatch;
  const fs::wpath wpath(opt_base);
  boost::system::error_code b_err;
  const bool result = fs::exists(wpath, b_err);
  if (result && !b_err) {
    BOOST_FOREACH(const fs::wpath& wp,
        std::make_pair(fs::directory_iterator(wpath),
          fs::directory_iterator())) {
      if (fs::is_directory(wp)
          && std::regex_match(wp.wstring(), wmatch, wre)) {
        std::unique_ptr<git_path_t> gp(new git_path_t);
        gp->path = wmatch.str(0);
        gp->ver.major = std::stoi(wmatch.str(1));
        gp->ver.minor = std::stoi(wmatch.str(2));
        gp->ver.revision = std::stoi(wmatch.str(3));
        vec.push_back(std::move(gp));
      }
    }
    if (vec.empty()) {
      std::cerr << "Git directory not found" << std::endl;
      return 1;
    }
  } else {
    std::wcerr << L"Not exists " + opt_base << std::endl;
    return 1;
  }
  std::sort(vec.begin(), vec.end(),
      [](const std::unique_ptr<git_path_t> &lhs,
        const std::unique_ptr<git_path_t> &rhs)
      -> bool {return *lhs < *rhs;});
  auto last = vec.cend();
  std::wstring git_dir = (last - 1)->get()->path;

  std::wstring env_path;
  err = _wgetenv_s(&len, NULL, 0, L"PATH");
  if (err == 0 && len) {
    std::unique_ptr<wchar_t[]> wcp(new wchar_t[len]);
    err = _wgetenv_s(&len, wcp.get(), len, L"PATH");
    if (err == 0 && len) {
      env_path = wcp.get();
    } else {
      std::cerr << "Failed get PATH environment variable" << std::endl;
      return 1;
    }
  }else {
    std::cerr << "Failed get PATH environment variable" << std::endl;
    return 1;
  }

  env_path = git_dir + L"\\bin;" + msys2_base + L"\\usr\\bin;" + env_path;
  if (_wputenv_s(L"PATH", env_path.c_str())) {
    std::cerr << "Failed put PATH environment variable" << std::endl;
    return 1;
  }

  const fs::wpath prg(*(argv++));
  std::wstring prg_name, script_name;
  std::vector<std::unique_ptr<wchar_t[]>> args;
  prg_name = prg.filename().c_str();
  if (prg_name == L"git.exe" || prg_name == L"git") {
    prg_name = git_dir + L"\\bin\\" + prg.filename().c_str();
  } else {
    if (prg_name == L"gitk.exe" || prg_name == L"gitk") {
      script_name = git_dir + L"\\bin\\gitk";
    } else if (prg_name == L"git-gui.exe" || prg_name == L"git-gui") {
      script_name = git_dir + L"\\libexec\\git-core\\git-gui";
    } else {
      std::cerr << "The program was called with an incorrect name" << std::endl;
      return 1;
    }
    prg_name = msys2_base + L"\\mingw64\\bin\\wish.exe";
  }

  std::unique_ptr<wchar_t[]> pprg_name(new wchar_t[prg_name.length() + 1]);
  wcscpy(pprg_name.get(), prg_name.c_str());
  args.push_back(std::move(pprg_name));
  std::unique_ptr<wchar_t[]> pscript_name;

  if (script_name.length()) {
    pscript_name.reset(new wchar_t[script_name.length() + 1]);
    wcscpy(pscript_name.get(), script_name.c_str());
    args.push_back(std::move(pscript_name));
  }

  while(*argv) {
    std::unique_ptr<wchar_t[]> parg(new wchar_t[wcslen(*argv) + 1]);
    wcscpy(parg.get(), *argv);
    args.push_back(std::move(parg));
    ++argv;
  }

  wchar_t* nargs[args.size() + 1];
  size_t i = 0;
  for(auto it = args.begin(); it != args.end(); ++it) {
    nargs[i] = it->get();
    ++i;
  }
  nargs[i] = NULL;

  if (script_name.length()) {
    _wexecv(prg_name.c_str(), nargs);
    return 1;
  } else {
    intptr_t ret = _wspawnv(P_WAIT, prg_name.c_str(), nargs);
    return ret;
  }
}
