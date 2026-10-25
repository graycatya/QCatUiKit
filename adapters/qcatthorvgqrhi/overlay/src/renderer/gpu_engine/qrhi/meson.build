# Explicit opt-in: 'all' keeps its existing meaning and does not require Qt.
qrhi_qt = dependency('qt6', modules: ['Core', 'Gui'], private_headers: true, version: '>=6.7', method: 'config-tool')
override_options += ['cpp_std=c++17']
qsb = find_program('qsb', 'qsb6')
python = import('python').find_installation()
shader_headers = []
foreach stage : ['vert', 'frag']
    baked = custom_target('qrhi_' + stage,
        input: 'solid.' + stage,
        output: 'solid.' + stage + '.qsb',
        command: [qsb, '--glsl', '150,300es', '--hlsl', '50', '--msl', '12', '-o', '@OUTPUT@', '@INPUT@'])
    shader_headers += custom_target('qrhi_' + stage + '_header',
        input: baked,
        output: 'qcatthorvgqrhi' + stage + '_p.h',
        command: [python, files('embed_shader.py'), '@INPUT@', '@OUTPUT@', 'qcatthorvgqrhi' + stage])
endforeach
engine_dep += declare_dependency(
    include_directories: include_directories('.'),
    dependencies: qrhi_qt,
    sources: ['qcatthorvgqrhirenderer.cpp', 'qcatthorvgqrhicanvas.cpp', 'qcatthorvgqrhimaterial.cpp', 'qcatthorvgqrhieffects.cpp', 'qcatthorvgqrhicompositor.cpp', shader_headers])
install_headers('../../../../inc/qcatthorvgqrhicanvas.h', subdir: 'thorvg-' + vmaj)
