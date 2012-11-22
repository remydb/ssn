#!/usr/bin/python

import sys
import re
import syslog
import subprocess
import Crypto.PublicKey.RSA as RSA
import os

class Proc:

	def __init__(self):
		self.file = sys.argv[1]
		self.user = sys.argv[2]
		self.date = sys.argv[3]

	def sign(self):
		key = RSA.generate(2048,os.urandom)
		K = '1234'
		enc = key.encrypt(self.user + " " + self.date,K)
		print enc
		sig = key.sign(self.user + " " + self.date,K)
		# print sig
		encsig = key.encrypt(sig[0],K)
		print encsig
		versig = key.verify(self.user + " " + self.date,sig)
		print versig


	def jpg():
		subprocess.call(["./steghide", "-blabla"])

if __name__ == "__main__":
	x = Proc()
	x.sign()


	# try:
	# 	func = getattr(x, re.search('[^.]*$', x.file))
	# except:
	# 	syslog.syslog(LOG_ERR, "[Python-Proc] Specified file extension not supported, terminating")
	# 	quit()

	# if(callable(func)):
	# 	funct()


















	# def ext(self):
	# 	extension = re.search('[^.]*$', self.file)
	# 	if extension.lower() == 'jpg':
	# 		self.procjpg()

	# 	elif extension.lower() == 'pdf':
	# 		#Do something

	# 	elif extension.lower() == 'mp3':
	# 		#Do something

	# 	else:
	# 		print "Unknown extension for source file, terminating now"