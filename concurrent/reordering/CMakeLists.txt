add_executable(reordering)
if (WIN32)
  target_sources(reordering PRIVATE reodering_win.cpp)
elseif(LINUX)
  target_sources(reordering PRIVATE reordering_linux.cpp)
endif()
