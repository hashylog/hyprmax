project('hyprmax', 'cpp',
  version: '1.0.0',
  default_options: ['buildtype=release'],
)

cpp_compiler = meson.get_compiler('cpp')
if cpp_compiler.has_argument('-std=c++23')
  add_global_arguments('-std=c++23', language: 'cpp')
elif cpp_compiler.has_argument('-std=c++2b')
  add_global_arguments('-std=c++2b', language: 'cpp')
else
  error('Could not configure current C++ compiler (' + cpp_compiler.get_id() + ' ' + cpp_compiler.version() + ') with required C++ standard (C++23)')
endif

globber = run_command('find', 'src', '-name', '*.cpp', check: true)
src = globber.stdout().strip().split('\n')

home_dir = run_command('sh', '-c', 'echo $HOME', check: true).stdout().strip()
plugin_dir = home_dir / '.config' / 'hypr' / 'plugins'

shared_module(meson.project_name(), src,
  dependencies: [
    dependency('hyprland'),
    dependency('pixman-1'),
    dependency('libdrm'),
    dependency('pangocairo'),
    dependency('libinput'),
    dependency('libudev'),
    dependency('wayland-server'),
    dependency('xkbcommon'),
  ],
  install: true,
  install_dir: plugin_dir,
)
