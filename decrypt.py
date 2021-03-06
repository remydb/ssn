#!/usr/bin/python

import sys
import os
import Image
import stepic
import time
import subprocess
import Crypto.Signature.PKCS1_v1_5 as PKCS1_v1_5
import Crypto.PublicKey.RSA as RSA
import Crypto.Hash.MD5 as MD5

class Decrypt:

	def __init__(self):
		self.file		= sys.argv[1]

	def verify(self,ciphertext):
		f = open('./priv_key.pem', 'r')
		key = RSA.importKey(f.read())
		f.close()
		cipher = PKCS1_v1_5.new(key)
		message = cipher.verify(MD5.new(ciphertext.split('***')[2]),ciphertext.split('***')[1])
		if message == True:
			return "File retrieval info: " + ciphertext.split('***')[2] + "\nFile info has been verified by signature"
		else:
			return "Authenticity of watermark could not be proven. File has been tampered with."
		return message

	def desteg(self):
		try:
			im = Image.open(self.file)
		except:
			print "Image file is damaged, watermark has probably been tampered with."
			quit()
		ciphertext = stepic.decode(im)
		message = self.verify(ciphertext)
		print message


	def dejpg(self):
		with open(os.devnull, 'w') as tempf:
			subprocess.call('steghide --extract -q -sf ' + self.file + ' -p stego -xf ' + os.path.dirname(sys.argv[1]) + '.extract', shell=True, stdout=tempf, stderr=tempf)
		try:
			f = open('.extract','r')
		except:
			print "Steghide could not retrieve information from specified file"
			quit()
		ciphertext = f.read()
		f.close()
		os.remove('.extract')
		message = self.verify(ciphertext)
		print message

if __name__ == "__main__":
	x = Decrypt()
	filename, fileext = os.path.splitext(x.file)
	if fileext == '.jpg' or fileext == '.JPG' or fileext == '.jpeg' or fileext == '.JPEG':
		x.dejpg()
	else:
		x.desteg()
