# SPDX-License-Identifier: BSD-2-Clause
# Copyright 2021 Haggai Eran
#

project('libconntrack-cm', 'cpp', default_options : 
	[
		'cpp_std=gnu++17',
	])

sources = [
    'src/cm_connection_tracker.cpp',
	'src/cnp.cpp',
	'src/main.cpp',
	'src/parser.cpp',
]

cc = meson.get_compiler('cpp')

dpdk = dependency('libdpdk')
boost = dependency('boost')


add_project_arguments('-fvisibility=hidden', language: 'cpp')
add_project_arguments(cc.get_supported_arguments([
	'-Wshadow=local',
	'-Wconversion',
]), language: 'cpp')

linker_script = 'src/libconntrack-cm.map'

libconntrack_cm = library('conntrack-cm',
	sources: sources,
	dependencies: [dpdk, boost],
	include_directories: ['include'],
	link_args: ['-Wl,--version-script,@0@/@1@'.format(meson.current_source_dir(), linker_script)],
	link_depends: linker_script)

install_headers('include/libconntrack-cm.h')

## unit tests

gtest_proj = subproject('gtest')
gtest_dep = gtest_proj.get_variable('gtest_dep')
gtest_main_dep = gtest_proj.get_variable('gtest_main_dep')

tests_src = [
  'tests/test_cnp.cpp',
]
e = executable(
	'gtest-all',
  	tests_src,
  	dependencies: [
    	gtest_dep,
		gtest_main_dep,
		dpdk,
  	],
	link_with: libconntrack_cm,
	include_directories: ['include'],
)
test('gtest tests', e)

