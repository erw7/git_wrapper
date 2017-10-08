#include "git_wrapper.h"

int wmain(int argc, wchar_t* argv[])
{
  errno_t err;
  std::wstring msys2_base;
  std::wstring opt_base;

  err = wgetenv_wrapper(L"MSYS2_BASE", msys2_base);
  if (err != 0 || msys2_base.empty()) {
    msys2_base = MSYS2_BASE;
  }

  err = wgetenv_wrapper(L"OPT_BASE", opt_base);
  if (err != 0 || opt_base.empty()) {
    opt_base = msys2_base + OPT_BASE;
  }

  std::vector<std::unique_ptr<git_path_t>> vec;
  std::wregex wre(L".*\\git-(\\d+)\\.(\\d+)\\.(\\d+)");
  std::wsmatch wmatch;
  const fs::wpath wpath(opt_base);
  boost::system::error_code b_err;
  const bool result = fs::exists(wpath, b_err);
  if (result && !b_err) {
    for (const auto& wp :
        boost::make_iterator_range(fs::directory_iterator(wpath), {})) {
      if (fs::is_directory(wp)
          && std::regex_match(wp.path().wstring(), wmatch, wre)) {
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
  boost::sort(vec,
      [](const std::unique_ptr<git_path_t> &lhs,
        const std::unique_ptr<git_path_t> &rhs)
      -> bool {return *lhs < *rhs;});
  auto last = vec.cend();
  std::wstring git_dir = (last - 1)->get()->path;

  std::wstring env_path;
  err = wgetenv_wrapper(L"PATH", env_path);
  if (err != 0 && env_path.empty()) {
    std::cerr << "Failed get PATH environment variable" << std::endl;
    return 1;
  }

  std::wstring git_dir_bin(git_dir + L"\\bin");
  std::wstring msys2_bin(msys2_base + L"\\usr\\bin");
  std::wstring mingw64_bin(msys2_base + L"\\mingw64\\bin");
  if (env_path.find(git_dir_bin) == std::string::npos) {
    env_path = git_dir_bin + L";" + env_path;
  }
  if (env_path.find(msys2_bin) == std::string::npos) {
    env_path = msys2_bin + L";" + env_path;
  }
  if (env_path.find(mingw64_bin) == std::string::npos) {
    env_path = mingw64_bin + L";" + env_path;
  }
  if (_wputenv_s(L"PATH", env_path.c_str())) {
    std::cerr << "Failed put PATH environment variable" << std::endl;
    return 1;
  }

  const fs::wpath prg(*(argv++));
  std::wstring prg_name, script_name;
  std::vector<std::wstring> args;
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

  args.push_back(prg_name);

  if (script_name.length()) {
    args.push_back(script_name);
  }

  auto range = boost::make_iterator_range_n(argv, argc - 1);
  for (auto arg : range) {
    std::wregex wre(L"(.*=)(.* .*)");
    std::wsmatch wmatch;
    std::wstring wsarg(arg);
    if (std::regex_match(wsarg, wmatch, wre)) {
      std::wstring quoted_arg(wmatch.str(1) + L"\"" + wmatch.str(2) + L"\"");
      args.push_back(quoted_arg.c_str());
    } else {
      args.push_back(arg);
    }
  }

  const wchar_t* nargs[args.size() + 1];
  size_t i = 0;
  for(const auto& ref: args) {
    nargs[i] = ref.c_str();
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

errno_t wgetenv_wrapper(const std::wstring& name, std::wstring& value)
{
  errno_t ret;
  size_t len;

  value = L"";
  ret = _wgetenv_s(&len, NULL, 0, name.c_str());
  if (ret || len == 0) {
    return ret;
  }
  std::unique_ptr<wchar_t[]> upvalue(new wchar_t[len]);
  ret = _wgetenv_s(&len, upvalue.get(), len, name.c_str());
  if (ret || len == 0) {
    return ret;
  }
  value = upvalue.get();
  return ret;
}
