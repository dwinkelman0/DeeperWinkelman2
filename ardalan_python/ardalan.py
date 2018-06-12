import imp

from os.path import realpath, dirname, join
CWD = dirname(realpath(__file__))

from sys import version_info
if version_info.major == 2:
	print("Loading ardalan for Python 2")
	ardalan = imp.load_dynamic('ardalan', CWD + '/libardalanpython27.so')
elif version_info.major == 3:
	print("Loading ardalan for Python 3")
	ardalan = imp.load_dynamic('ardalan', CWD + '/libardalanpython35.so')
else:
	print("Ardalan does not support Python %i.%i" % (version_info.major, version_info.minor))

del imp
del realpath, dirname, join
del CWD