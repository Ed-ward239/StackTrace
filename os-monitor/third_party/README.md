# third_party

StackTrace v2 pulls its dependencies automatically at configure time via
CMake's `FetchContent`, so this directory stays empty in the repo — there are
no vendored sources to commit and nothing to `git submodule update`.

`cmake -B build` downloads and builds, pinned to these versions:

| Dependency | Version | Purpose |
|------------|---------|---------|
| [FTXUI](https://github.com/ArthurSonzogni/FTXUI) | v5.0.0 | Declarative terminal UI |
| [nlohmann/json](https://github.com/nlohmann/json) | v3.11.3 | `config.json` parsing |
| [tinyxml2](https://github.com/leethomason/tinyxml2) | 10.0.0 | RSS/Atom parsing |

`libcurl` is found via the system package manager (`find_package(CURL)`), not
fetched. If it is missing, the build still succeeds and the stock/news panels
fall back to placeholder data.

To vendor a dependency instead (e.g. for offline builds), drop its source here
and swap the matching `FetchContent_Declare` in `../CMakeLists.txt` for an
`add_subdirectory(third_party/<dep>)`.
