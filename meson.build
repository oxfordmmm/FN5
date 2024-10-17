project('fn5', 'cpp',
  version : '2.0.7',
  default_options : ['cpp_std=c++2a', 'optimization=3'])

thread_dep = dependency('threads')

py = import('python').find_installation(pure: false)
pybind11_dep = dependency('pybind11')


incdir = include_directories('src/include')

py.extension_module('fn5',
        'src/fn5_python.cpp',
        'src/sample.cpp', 
        'src/comparisons.cpp', 
        'src/fn5_python.cpp',
        include_directories : incdir,
        dependencies : [thread_dep, pybind11_dep],
        install: true
      )
