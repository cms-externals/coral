import coral 
import os 
import filecmp 
import unittest

print "---------------------------------------------"
print "Note: The Input File should reside in the current directory"
print "Note: The Output File will be created in the current Directory"
print "---------------------------------------------"
inputFile = raw_input("Input file name for writing to Blob:")
outputFile = raw_input("Output file name for writing from Blob:")
print "----------------------------------------------"

inf = open(inputFile, 'rb')
ouf = open(outputFile, 'wb')

toReadIntoBlob = coral.Blob()
tempBlob = coral.Blob(100)
toReadIntoBlob.append(tempBlob)
print "After append, Blob Size = ", toReadIntoBlob.size()
toReadIntoBlob.extend(2500)
print "After 1 extend, Blob Size = ", toReadIntoBlob.size()
toReadIntoBlob.extend(80)
print "After 2 extends Blob Size = ", toReadIntoBlob.size()
toReadIntoBlob.resize(os.path.getsize(inputFile))
print "After Resize Blob Size = " , toReadIntoBlob.size()

inf.readinto(toReadIntoBlob)

class PyCoralBlobReadWriteTest(unittest.TestCase):
	def setUp(self):
		print '[OVAL] setUp'
	
	def testBlobModule(self):

		print '[OVAL] Test PyCoral Blob Module'
		print coral.Blob.__doc__

                ouf.write(toReadIntoBlob)
		ouf.close()

                ouf1 = open(outputFile, 'rb')

		print "File Comparision Result: for Input File = " , inputFile , " and OutputFile = " , outputFile , "is ", filecmp.cmp(inputFile,outputFile) 
		inf.close()
		ouf1.close()

	def tearDown(self):
		print '[OVAL] tearDown'

if __name__=="__main__":
	unittest.main() 	
