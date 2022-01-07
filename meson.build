project('libbencode', 'c',
	default_options : [
		'warning_level=3',
		'werror=false',
		'debug=false',
		'optimization=2',
		'c_std=c2x', 'build.c_std=c2x'
		],
	license: 'Apache License',
	version: '1.0.0',
)
project_description = 'A p2p messaging library.'
add_project_arguments('-Wpedantic', language : 'c')
add_project_arguments('-Wshadow', language : 'c')


project_headers = [
	'bencode.h'
]

project_source_files = [
	'bencode.c'
]

install_headers(project_headers)

project_target=library('bencode', project_source_files)
