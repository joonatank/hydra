""" A signal/slot implementation

File:		signal.py
Author:		Thiago Marcos P. Santos
Author:		Christopher S. Case
Author:		David H. Bronke
Created:	August 28, 2008
Updated:	December 12, 2011
License:	MIT

"""
#LICENSED CODE BEGINS HERE------------------------------------------------------
import inspect
from weakref import WeakSet, WeakKeyDictionary


class Signal(object):
	def __init__(self):
		self._functions = WeakSet()
		self._methods = WeakKeyDictionary()

	def __call__(self, *args, **kargs):
		# Call handler functions
		for func in self._functions:
			func(*args, **kargs)

		# Call handler methods
		for obj, funcs in list(self._methods.items()):
			for func in funcs:
				func(obj, *args, **kargs)

	def connect(self, slot):
		if inspect.ismethod(slot):
			if slot.__self__ not in self._methods:
				self._methods[slot.__self__] = set()

			self._methods[slot.__self__].add(slot.__func__)

		else:
			self._functions.add(slot)

	def disconnect(self, slot):
		if inspect.ismethod(slot):
			if slot.__self__ in self._methods:
				self._methods[slot.__self__].remove(slot.__func__)
		else:
			if slot in self._functions:
				self._functions.remove(slot)

	def clear(self):
		self._functions.clear()
		self._methods.clear()
#LICENSED CODE ENDS HERE--------------------------------------------------------
