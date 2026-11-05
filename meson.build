project('pokeplatinum', ['c', 'cpp', 'nasm'],
    version: '1.0',
    meson_version: '>=1.12.0',
    default_options : [
        'buildtype=plain',
        'warning_level=0'
    ]
)

fs = import('fs')

############################################################
###                    INCLUDE PATHS                     ###
############################################################
public_includes = include_directories('include', 'asm')
toplevel_includes = include_directories('.')


############################################################
###                    COMPILER FLAGS                    ###
############################################################
shared_args = [
    '-proc', 'arm946e',
    '-enum', 'int',
    '-Cpp_exceptions', 'off',
    '-gccext,on',
    '-msgstyle', 'gcc',
    '-ipa', 'file',
    '-interworking',
    '-inline', 'on,noauto',
    '-char', 'signed',
    '-nosyspath',
    '-stdinc',
    '-sym', 'on',
    '-DPOKEPLATINUM_GENERATED_ENUM',
    '-D_MSL_RESTRICT=',
]

if get_option('revision') == '0'
    shared_args += [
        '-DPOKEPLATINUM_REVISION=0'
    ]
else
    shared_args += [
        '-DPOKEPLATINUM_REVISION=1'
    ]
endif

if get_option('gdb_debugging')
    shared_args += [
        '-O1,p',
#        '-inline', 'off' todo: figure out why this makes the rom crash after the copyright screen
    ]
else
    shared_args += '-O4,p'
endif

c_args = shared_args + ['-lang', 'c99']
cpp_args = shared_args + ['-RTTI', 'off']

add_global_arguments(c_args,
    language: 'c',
    native: false
)

add_global_arguments(cpp_args,
    language: 'cpp',
    native: false
)

pokeplatinum_args = [
    '-DPM_KEEP_ASSERTS',
    '-DGAME_VERSION=VERSION_PLATINUM',
    '-DGAME_LANGUAGE=LANGUAGE_ENGLISH'
]

if get_option('gdb_debugging')
    pokeplatinum_args += '-DGDB_DEBUGGING'
endif

if get_option('logging_enabled')
    pokeplatinum_args += [
        '-DLOGGING_ENABLED',
    ]
endif

asm_args = [
    '-proc', 'arm5TE',
    '-16',
    '-gccinc'
]

link_args = [
    '-w', 'off',
    '-proc', 'arm946e',
    '-nopic',
    '-nopid',
    '-interworking',
    '-map', 'closure,unused',
    '-symtab', 'sort',
    '-msgstyle' ,'gcc',
    '-m', '_start',
    '-force_active', 'DGTi_hash2_arm4_small',
    '-sym', 'on'
]

############################################################
###                     DEPENDENCIES                     ###
############################################################
nitrosdk_dep = dependency('NitroSDK',
    default_options: {
        'mwrap_sdk': '2.0/sp1p2',
        'sp1p3_conditionals': true
    }
)

nitrosystem_dep = dependency('NitroSystem',
    default_options: {
        'mwrap_sdk': '2.0/sp2',
    }
)

nitrowifi_dep = dependency('NitroWiFi',
    default_options: {
        'mwrap_sdk': '2.0/sp1p2',
        'sp1p3_conditionals': true
    }
)

nitrodwc_dep = dependency('NitroDWC',
    default_options: {
        'mwrap_sdk': '2.0/sp2p2',
        'link_ppwlobby': true
    }
)

libvct_dep = dependency('libvct')
libsyscall_dep = dependency('libsyscall')

############################################################
###                       CONSTS                         ###
############################################################
subdir('generated')


############################################################
###                        TOOLS                         ###
############################################################
subdir('tools')


############################################################
###                      SPEC FILES                      ###
############################################################
subdir('platinum.us')


############################################################
###                  INTERNAL LIBRARIES                  ###
############################################################
subdir('lib')


############################################################
###                     FILE SYSTEM                      ###
############################################################
subdir('res')

# Compact BST, evolution-family and classification tables. Keep data versioning
# tied to the actual species data instead of loading personal.narc per candidate.
run_species_python = find_program('python3', native: true)
run_species_data = custom_target('run_species_data.h',
    output: 'run_species_data.h',
    input: [species_txt, files('tools/datagen/run_species_classes.json', 'tools/datagen/run_species.py')],
    depend_files: species_data_files,
    command: [run_species_python, files('tools/datagen/run_species.py'),
        '--root', meson.project_source_root(), '--output', '@OUTPUT@'],
)

run_world_data = custom_target('run_world_data.h',
    output: 'run_world_data.h',
    input: files('tools/datagen/run_world.py', 'tools/datagen/run_zones.json',
        'include/data/map_headers.h', 'generated/map_headers.txt', 'generated/trainers.txt',
        'res/text/location_names.json'),
    command: [run_species_python, files('tools/datagen/run_world.py'),
        '--root', meson.project_source_root(), '--output', '@OUTPUT@', '--depfile', '@DEPFILE@'],
    depfile: 'run_world_data.d',
)

############################################################
###                     ARM9 BINARY                      ###
############################################################
subdir('src')

trainer_ai_o = custom_target(
    'trainer_ai_script.o',
    output: 'trainer_ai_script.o',
    input: files('src/battle/trainer_ai/script.s'),
    command: [
        arm_none_eabi_gcc_exe,
        '-I', meson.global_source_root() / 'include',
        '-I', meson.global_source_root() / 'asm',
        '-I', meson.global_build_root(),
        '-x', 'assembler-with-cpp',
        '-MD', '-MF', '@DEPFILE@',
        '-c', '-o', '@OUTPUT@',
        '@INPUT@',
    ],
    depfile: '@BASENAME@.d',
)

libmslc = fs.copyfile('tools/metroskrew/lib/metroskrew/sdk/ds/2.0/sp2/msl/MSL_C/MSL_ARM/Lib/MSL_C_NITRO_Ai_LE.a')
libmslcpp = fs.copyfile('tools/metroskrew/lib/metroskrew/sdk/ds/2.0/sp2/msl/MSL_C++/MSL_ARM/Lib/MSL_CPP_NITRO_Ai_LE.a')

main = executable('main',
    sources: [
        pokeplatinum_c,
        c_consts_generators,
        naix_headers,
        h_headers,
        embed_headers,
        run_species_data,
        run_world_data,
    ],
    objects: trainer_ai_o,
    c_args: [
        pokeplatinum_args,
        '-thumb'
    ],
    c_pch: 'include/pch/global_pch.h',
    include_directories: [
        public_includes,
        libcrypto_public_includes,
        libgds_public_includes,
        libspl_public_includes,
        ppwlobby_public_includes,
    ],
    dependencies: [
        nitrosdk_dep,
        nitrosystem_dep,
        nitrowifi_dep,
        nitrodwc_dep,
        libvct_dep,
        libsyscall_dep,
    ],
    link_with: [
        libcrypto_ov97,
        libcrypto_static,
        libgds,
        libspl,
        libmslc,
        libmslcpp,
        libdwcenc,
        libdwcppwlobby,
        libdwcilobby
    ],
    link_args: [
        link_args,
        'platinum.us/main.lcf'
    ],
    link_depends: main_lcf,
    native: false
)


############################################################
###                    ARM7 BINARIES                     ###
############################################################
ichneumon_sub = subproject('NitroSDK'
    ).get_variable('ichneumon_sub')


############################################################
###                        DS ROM                        ###
############################################################
pokeplatinum_nds = custom_target('pokeplatinum.us.nds',
    output: 'pokeplatinum.us.nds',
    input: [
        main,
        rom_header_template,
        icon_png,
        ichneumon_sub,
        nitrofs_files,
    ],
    command : [
        # ENTRY P110: ysmguard.py runs the nitrorom command below, then fills the R4 / YSMenu guard table in the packed ROM
        ysmguard_py,
        '@OUTPUT0@',
        '--',
        nitrorom_exe,
        'pack',
        '-DHEADER_TEMPLATE=@INPUT1@', # rom_header_template
        '-DICON_PNG=@INPUT2@',        # icon_png
        '-DARM7_NEF=@INPUT3@',        # ichneumon_sub
        '--output=@OUTPUT0@',         # pokeplatinum.us.nds
        rom_ini,
        filesys_csv,
    ],
    build_by_default: true
)

############################################################
###                   GDB HELPERS                        ###
############################################################
nef_fixer = custom_target('debug.nef',
    output: [
        'debug.nef'
    ],
    input: [
        main
    ],
    command : [
        nef_fixer_py, '@INPUT@', '@OUTPUT@'
    ]
)

ovly_mapper = custom_target('overlay.map',
    output: [
        'overlay.map'
    ],
    input: [
        main_lsf
    ],
    command : [
        overlay_mapper_py, '@INPUT@', '@OUTPUT@'
    ]
)


############################################################
###                        TESTS                         ###
############################################################
sha1sum = find_program(['sha1sum', 'shasum'], native: true)

test('SBIN Checksums - Shared',
    sha1sum,
    args: ['-c', '--quiet', sbins_sha1]
)

test('SBIN Checksums - For Revision',
    sha1sum,
    args: ['-c', '--quiet', sbins_for_rev_sha1]
)

test('SBIN Checksums - ARM7',
    sha1sum,
    args: ['-c', '--quiet', sbins_arm7_sha1]
)

test('Filesystem Checksums',
    sha1sum,
    args: ['-c', '--quiet', filesys_sha1]
)

test('ROM Checksum',
    sha1sum,
    args: ['-c', '--quiet', rom_sha1]
)


############################################################
###                       POSTCONF                       ###
############################################################
meson.add_postconf_script(postconf_py)
