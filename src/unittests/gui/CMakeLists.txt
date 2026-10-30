# SPDX-FileCopyrightText: (C) 2025 - 2026 Deskflow Developers
# SPDX-License-Identifier: MIT

create_test(
  NAME ClipboardStatusLabelTests
  DEPENDS gui
  SOURCE ClipboardStatusLabelTests.cpp
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/src/lib/gui"
)

add_subdirectory(config)
add_subdirectory(core)

create_test(
  NAME LoggerTests
  DEPENDS gui
  SOURCE LoggerTests.cpp
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/src/lib/gui"
)
