My personal ncurses startpage.

## Usage

Create `settings.h` with your settings (Reddit username + shortcuts)

Compile with one of:

  - `clang main.c -Wall -Wextra -pedantic -lcurses -lpthread -o startpage -O3 -march=native`
  - `gcc main.c -Wall -Wextra -pedantic -lcurses -lpthread -o startpage -O2 -march=native`

Clang is preferred, as gcc will complain about struct initialization in `settings.h`.