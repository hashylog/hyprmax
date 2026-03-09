project('hyprmax', 'cpp',
  version: '1.0.0',
  default_options: [
    'buildtype=release',
    'cpp_std=c++23',
  ],
)

hyprland_dep = dependency('hyprland')

shared_module(meson.project_name(), files('src/main.cpp'),
  dependencies: [hyprland_dep],
  install: false,
)
