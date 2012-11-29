#!/usr/bin/python

import sys
import re
import syslog
import subprocess
import os
import Image
import stepic
import datetime
import md5
import Crypto.Cipher.PKCS1_OAEP as PKCS1_OAEP
import Crypto.PublicKey.RSA as RSA

class Proc:

	def __init__(self):
		self.file		= sys.argv[1]
		self.user		= sys.argv[2]
		self.tmp_file	= os.path.dirname(sys.argv[1]) + "/." + md5.new(self.file + str(datetime.datetime.now()))

	def encrypt(self):
		f = open('./priv_key.pem', 'r')
		key = RSA.importKey(f.read())
		f.close()
		cipher = PKCS1_OAEP.new(key)
		message = self.user+ " " + str(datetime.datetime.now())
		ciphertext = cipher.encrypt(message)
		return ciphertext

	def jpg(self):
		im = Image.open(self.file)
		s = stepic.Steganographer(im)
		data = str(self.encrypt())
		ime = s.encode(data)
		ime.save(self.tmp_file,'JPEG')

	def png(self):
		im = Image.open(self.file)
		s = stepic.Steganographer(im)
		data = str(self.encrypt())
		ime = s.encode(data)
		ime.save(self.tmp_file,'PNG')

	def bmp(self):
		im = Image.open(self.file)
		s = stepic.Steganographer(im)
		data = str(self.encrypt())
		ime = s.encode(data)
		ime.save(self.tmp_file,'BMP')

	def gif(self):
		im = Image.open(self.file)
		s = stepic.Steganographer(im)
		data = str(self.encrypt())
		ime = s.encode(data)
		ime.save(self.tmp_file,'GIF')

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
		print x.tmp_file