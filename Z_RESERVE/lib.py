import os
import subprocess
import sys
from typing import Self

def which(names:list)->list:
	e = subprocess.run(["which"]+names, capture_output=True)
	return e.stdout.decode().split("\n").pop()


def exec(line):
	pass

# Change directory and return the full path to which it changed to.
# Returns None if failed.
def cd(dir:str)->str:
	if not os.path.exists(dir) or os.path.isfile(dir):
		return None
	else:
		cwd = dir
		return dir

def newer_than(f1path:str, f2path)->str:
	if (os.stat(f1path).st_mtime > os.stat(f2path).st_mtime):
		return f1path
	else:
		return f2path

def find_by_name(basedir, wildcards) -> list:
	pass

class ThreadLocalData:
	pass

'''
The shift operators nest within each other. For example:

Cat("File.txt") >> Grep("Hello") >> LineCount >> output

This translates to:

Cat("File.txt").__shr__(Grep("Hello").__shr__(LineCount.__shr__(output)))

Shift operators make command compositions very easy to read.

The environment can be set for each command using brackets at any point.
It uses keyword args. Self-referencing should be marked using a $ prefix.

GCC[PATH="$PATH:"]("-c main.c")

It is also possible to run multiple instances of a command.

CC[CCPATH]("-c -O2", dup=glob(list, MODROOT+"/*.c"))

= THREAD SAFETY =

Pipe strings require dependent operations so naturally they cannot be
multithreaded.

As a result, execution of program

'''
class Tool:
	shell_cmd = None

	#
	# Called by the __rshift__ of another program for redirection.
	# Returns a string representing the standard output
	#
	def pass_input(self, stdin:str)->str:
		pass

	# Bracket operator sets the environment of the command.
	# It overrides global settings.
	def __getitem__(self, x):
		pass

	def __rshift__(self, dest):
		# Perform the action associated with the current command
		if dest is Tool:
			# The destination is another tool.

			pass
		else:
			#
			# The destination is a variable to which all the
			# Output gets copied. At this point the command
			# String is executed
			#
			pass
		pass

	def __call__(self, *args, **kwargs):
		# All kwargs are prefixed as requested by the constructor
		# A keyword called "dup" means to duplicate the command
		# and add an extra string into each iteration that also
		# substitutes
		pass

	def __init__(self, name, arg_prefix):
		self.shell_cmd = name

Cat = Tool("cat")

Grep = Tool("grep")

x=None
Cat("readme.md") >> x
# Exit status?
print(x)
