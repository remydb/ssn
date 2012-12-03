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
import base64
import Crypto.Cipher.PKCS1_OAEP as PKCS1_OAEP
import Crypto.PublicKey.RSA as RSA

class Decrypt:

	def __init__(self):
		self.file		= sys.argv[1]

	def decrypt(self,ciphertext):
		f = open('./priv_key.pem', 'r')
		key = RSA.importKey(f.read())
		f.close()
		cipher = PKCS1_OAEP.new(key)
		message = cipher.decrypt(ciphertext)
		return message

	def desteg(self):
		im = Image.open(self.file)
		ciphertext = stepic.decode(im)
		message = self.decrypt(ciphertext)
		print message

if __name__ == "__main__":
	x = Decrypt()
	x.desteg()