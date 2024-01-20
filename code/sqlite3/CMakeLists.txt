cmake_minimum_required(VERSION 3.5)
project(omohsqlite)

file(GLOB_RECURSE SOURCES_SQLITE "./*.c")

add_library(omohsqlite STATIC ${SOURCES_SQLITE})

find_package (SQLite3)

  include_directories(${SQLITE3_INCLUDE_DIRS})
  target_link_libraries (omohsqlite ${SQLITE3_LIBRARIES})
