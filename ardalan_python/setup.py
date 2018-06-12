from sys import version_info

major = version_info.major
minor = version_info.minor
print("Building with version %i.%i" % (major, minor))

################################################################################
# Documentation
module_name = "DeeperWinkelman2 Ardalan"
module_version = "1.0"
module_author = "Daniel Winkelman"
module_email = "dwinkelman3@gmail.com"
module_description = "Wrapper around the DeeperWinkelman2 chess engine board representation library"

################################################################################
# Include
# https://docs.python.org/2.5/dist/module-distutils.core.html
from distutils.core import setup, Extension
from os.path import realpath, dirname, join
from os import listdir
from shutil import rmtree

def FilesByExtension(directory, extension):
	return [join(directory, f) for f in listdir(directory) if f.endswith("." + extension)]

CWD = dirname(realpath(__file__))

################################################################################
# Delete old module file
try:
	rmtree(CWD + "/build")
except:
	None

################################################################################
# Gather object files
if major == 2:
	objects_apy = FilesByExtension(CWD + "/obj/Python2.7/", "o")
elif major == 3:
	objects_apy = FilesByExtension(CWD + "/obj/Python3.5/", "o")
objects_ardalan = FilesByExtension(CWD + "/../ardalan/obj/Release/", "o")
print("Gathered %i common, %i Python %i.%i objects to link" % (
	len(objects_ardalan), len(objects_apy), major, minor))

################################################################################
# Compile
module_apy = Extension(
	name = 'ardalan',
	include_dirs = [],
	sources = [],
	extra_objects = objects_ardalan + objects_apy,
	language = 'c++'
)

################################################################################
# Create the module

setup (
	name = module_name + " (Python %i.%i)" % (major, minor),
	version = module_version,
	description = module_description,
	author = module_author,
	author_email = module_email,
	ext_modules = [module_apy]
)