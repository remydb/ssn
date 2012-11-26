#!/usr/bin/python

import sys
import re
import syslog
import subprocess
import Crypto.PublicKey.RSA as RSA
import os
import Image
import stepic
import datetime

class Proc:

	def __init__(self):
		self.file = sys.argv[1]
		self.user = sys.argv[2]

	def encrypt(self):
		f = open('./priv_key.pem', 'r')
		key = RSA.importKey(f.read())
		f.close()
		K = '' # Can leave K empty, as the encrypt function actually ignores this value
		enc = key.encrypt(self.user + " " + str(datetime.datetime.now()),K)
		return enc

	def jpg(self):
		im = Image.open(self.file)
		s = stepic.Steganographer(im)
		data = str(self.encrypt())
		ime = s.encode(data)
		ime.save(self.file + '.tmp','JPEG')

	def png(self):
		im = Image.open(self.file)
		s = stepic.Steganographer(im)
		data = str(self.encrypt())
		ime = s.encode(data)
		ime.save(self.file + '.tmp','PNG')

	def bmp(self):
		im = Image.open(self.file)
		s = stepic.Steganographer(im)
		data = str(self.encrypt())
		ime = s.encode(data)
		ime.save(self.file + '.tmp','BMP')

	def gif(self):
		im = Image.open(self.file)
		s = stepic.Steganographer(im)
		data = str(self.encrypt())
		ime = s.encode(data)
		ime.save(self.file + '.tmp','GIF')

	def mp3(self):
		#Placeholder for calling mp3 stego
		quit()

	def pdf(self):
		#Placeholder for calling pdf stego
		quit()

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
	 	print x.file + '.tmp'