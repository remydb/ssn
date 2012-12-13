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
	def encrypt(self):
		key = RSA.generate(1024,os.urandom)
		cipher = PKCS1_OAEP.new(key)
		message = 'homo'+ " " + str(datetime.datetime.now())
		ciphertext = cipher.encrypt(message)
		print ciphertext
		f = open('./blaa', 'w')
		f.write(ciphertext)
		f.close()
		print sys.getsizeof(ciphertext)
		return ciphertext

if __name__ == '__main__':
	x = Proc()
	x.encrypt()