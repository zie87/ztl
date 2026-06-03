set shell := ["bash", "-eu", "-o", "pipefail", "-c"]


_default:
    @just --list

# full build & tests
build build_type='release':
    meson setup --reconfigure --buildtype {{build_type}} build/{{build_type}}
    meson compile -C build/{{build_type}}
    meson test -C build/{{build_type}}

# full coverage build & creation
coverage:
    meson setup --reconfigure -Db_coverage=true --buildtype debug build/gcc_cov
    meson compile -C build/gcc_cov
    meson test -C build/gcc_cov --print-errorlogs
    ninja coverage-html -C build/gcc_cov
    ninja coverage-xml -C build/gcc_cov

# full sanitizer build
sanitize:
    meson setup --reconfigure -Db_sanitize=address,undefined --buildtype debug build/gcc_san
    meson compile -C build/gcc_san
    meson test -C build/gcc_san --print-errorlogs
