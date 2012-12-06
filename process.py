#!/usr/bin/python

import sys
import re
import syslog
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
		self.tmp_file	= os.path.dirname(sys.argv[1]) + "/." + md5.new(self.file + self.user + str(datetime.datetime.now())).hexdigest()
		self.tmp_efile	= os.path.dirname(sys.argv[1]) + "/.embedfile"

	def encrypt(self):
		f = open('./priv_key.pem', 'r')
		key = RSA.importKey(f.read())
		f.close()
		cipher = PKCS1_OAEP.new(key)
		message = self.user+ " " + str(datetime.datetime.now())
		ciphertext = cipher.encrypt(message)
		return ciphertext

	def jpg(self):
		data = str(self.encrypt())
		f = open(self.tmp_efile, 'w')
		f.write(data)
		f.close()
		os.system("steghide embed -cf " + self.file + " -ef " + self.tmp_efile + " -p stego -e none -Z -sf " + self.tmp_file)
		os.remove(self.tmp_efile)

	def png(self):
		im = Image.open(self.file)
		data = str(self.encrypt())
		ime = stepic.encode(im,data)
		ime.save(self.tmp_file,'PNG')

	def bmp(self):
		im = Image.open(self.file)
		data = str(self.encrypt())
		ime = stepic.encode(im,data)
		ime.save(self.tmp_file,'BMP')

	def gif(self):
		im = Image.open(self.file)
		data = str(self.encrypt())
		ime = stepic.encode(im,data)
		ime.save(self.tmp_file,'GIF')

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