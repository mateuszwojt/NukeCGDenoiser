import os
import nuke

# add DLLs to NUKE_PATH
plugin_path = os.path.dirname(os.path.realpath(__file__))
lib_dir_name = 'lib64' if os.name == 'posix' else 'lib'
nuke.pluginAddPath(plugin_path + os.path.sep + lib_dir_name)
nuke.pluginAddPath(plugin_path + os.path.sep + 'plugins')

# Add denoiser entry to the menu
nuke.menu('Nodes').addMenu('MW')
nuke.menu('Nodes').addCommand('MW/Denoiser', lambda: nuke.createNode('Denoiser'))

nuke.load('NukeCGDenoiser')