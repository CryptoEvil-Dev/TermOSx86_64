project('TermOS', 'c', 'rust', version: '0.1.0')

# Проверяем наличие компиляторов
nasm = find_program('nasm', required: true)

# 1. Сборка Primary Bootloader (16-бит ASM)
primary_boot = custom_target('primary_boot',
    input: 'boot/primary.asm',
    output: 'primary.bin',
    command: [nasm, '-f', 'bin', '@INPUT@', '-o', '@OUTPUT@'],
    build_by_default: true
    )


# 2. Сборка Secondary Bootloader (32-бит Rust, bare-metal)
# secondary_boot = custom_target('secondary_boot',
#     input: 'boot/secondary.rs',
#     output: 'secondary.bin',
#     command: [
#         'rustc', 
#         '--crate-type=staticlib', 
#         '--target=i686-unknown-linux-gnu', 
#         '-C', 'panic=abort', 
#         '-C', 'relocation-model=static', 
#         '-C', 'link-arg=-nostdlib',
#         '-O', 
#         '@INPUT@', 
#         '-o', '@OUTPUT@'
#     ],
#     build_by_default: true
# )

# 3. Сборка ядра
