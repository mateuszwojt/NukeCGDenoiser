plugin_path = os.path.dirname(os.path.realpath(__file__))

nuke.menu('Nodes').addMenu('MW')
nuke.menu('Nodes').addCommand('MW/Denoiser', lambda: nuke.createNode('Denoiser'))

nuke.load('denoiser')