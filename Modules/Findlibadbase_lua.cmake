MESSAGE(STATUS "Using bundled Findlibadbase_lua.cmake...")
  FIND_PATH(
      LIBADBASE_LUA_INCLUDE_DIR
      Lua.hpp
      /usr/local/include/adbase
     /usr/include/
  )

FIND_LIBRARY(
  LIBADBASE_LUA_LIBRARIES NAMES libadbase_lua.a
  PATHS /usr/lib/ /usr/local/lib/ /usr/lib64 /usr/local/lib64
  )
