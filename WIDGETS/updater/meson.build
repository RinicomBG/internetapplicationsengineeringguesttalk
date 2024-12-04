project('updater', 'c', 'cpp')

c_args = []

c_args += '-Wno-unknown-pragmas'
c_args += '-D__COMPILE_FOR_LINUX'

add_project_arguments(c_args, language: [ 'c', 'cpp' ])

fcgi_dep = dependency('fcgi')
cc = meson.get_compiler('c')
math_dep = cc.find_library('m', required : false)

updater_sources = files(
  'src/timefn.c',
  'src/utilityfn.c',
  'src/filesystem.c'
)
updater_include_dirs = include_directories('src')
updater_lib = static_library('updater', updater_sources, include_directories : updater_include_dirs, install : false)
updater_dep = declare_dependency(include_directories : updater_include_dirs, link_with : [ updater_lib ])

executable('updater', 'appsrc/updater.c', dependencies : [ updater_dep, math_dep ])
executable('updater_json', 'appsrc/updater_json.c', dependencies : [ updater_dep, math_dep ])
executable('updater_cgi', 'appsrc/updater_cgi.c', dependencies : [ updater_dep, math_dep ])
executable('updater_fcgi', 'appsrc/updater_fcgi.c', dependencies : [ updater_dep, math_dep, fcgi_dep ])

