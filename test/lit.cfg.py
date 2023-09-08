import lit.formats

config.name = "Goop Tests"
config.test_format = lit.formats.ShTest(True)

config.suffixes = ['.go', '.c']

config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = os.path.join(config.goop_bin_root, 'test')

config.substitutions += [
        ('%goop-tok', os.path.join(config.goop_bin_root, 'goop-tok')),
        ('%goop-ast', os.path.join(config.goop_bin_root, 'goop-ast'))
]
