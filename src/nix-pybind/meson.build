project('nix-pybind', 'cpp',
  version : files('.version'),
  default_options : [
    'cpp_std=c++2a',
    'warning_level=1',
    'errorlogs=true',
  ],
  meson_version : '>= 1.1',
  license : 'LGPL-2.1-or-later',
)

cxx = meson.get_compiler('cpp')

subdir('nix-meson-build-support/deps-lists')

# Find required dependencies
pybind_dep = dependency('pybind11', required: true)
python = import('python').find_installation()
pybind11_include = include_directories(run_command('pybind11-config', '--includes').stdout().split('-I')[-1].strip())


deps_private_maybe_subproject = [
  dependency('nix-util'),
  dependency('nix-store'),
  dependency('nix-expr'),
]

deps_public_maybe_subproject = [
  dependency('nix-util-c'),
  dependency('nix-store-c'),
  dependency('nix-expr-c'),
  dependency('nix-flake-c'),
]

subdir('nix-meson-build-support/subprojects')
subdir('nix-meson-build-support/common')
subdir('nix-meson-build-support/export-all-symbols')

configdata = configuration_data()

configdata.set_quoted('PACKAGE_VERSION', meson.project_version())


# Define sources for each binding module
sources = [
    'bindings/bindings.cpp',
    'bindings/libexpr_bindings.cpp',
]

# Include paths for Nix libraries
include_dirs = [include_directories('.')]

# Build the Python module
py_module = python.extension_module(
    'nixpy',
    sources,
    dependencies : deps_public + deps_private + deps_other,
    include_directories: include_dirs + pybind11_include,
    link_args: linker_export_flags,
    install : true,
)

# Install the module
install_subdir('bindings', install_dir: python.get_install_dir())
