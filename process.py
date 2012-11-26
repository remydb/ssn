#!/usr/bin/python

import sys
import re
import syslog
import subprocess
import Crypto.PublicKey.RSA as RSA
import os
import Image
import stepic

class Proc:

	def __init__(self):
		self.file = sys.argv[1]
		self.user = sys.argv[2]
		self.date = sys.argv[3]

	def encrypt(self):
		key = RSA.generate(2048,os.urandom)
		K = '1234'
		enc = key.encrypt(self.user + " " + self.date,K)
		print enc

	def jpg(self):
		im = Image.open(self.file)
		s = stepic.Steganographer(im)
		ime = s.encode(self.user + " " + self.date)
		ime.save('output.jpg','JPEG')

	def png(self):
		im = Image.open(self.file)
		s = stepic.Steganographer(im)
		ime = s.encode(self.user + " " + self.date)
		ime.save('output.png','PNG')

	def bmp(self):
		im = Image.open(self.file)
		s = stepic.Steganographer(im)
		ime = s.encode(self.user + " " + self.date)
		ime.save('output.bmp','BMP')

	def gif(self):
		im = Image.open(self.file)
		s = stepic.Steganographer(im)
		ime = s.encode(self.user + " " + self.date)
		ime.save('output.gif','GIF')

if __name__ == "__main__":
	x = Proc()
	try:
		ext0 = re.search('[^.]*$', x.file)
		ext1 = ext0.group(0)
		func = getattr(x, ext1)
	except:
		syslog.syslog(syslog.LOG_ERR ,"[Python-Proc] Specified file extension not supported, terminating")
		quit()

	if(callable(func)):
	 	func()