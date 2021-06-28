import coral 
import os 
import filecmp 
import unittest

print("---------------------------------------------")
print("Note: The Input File should reside in the current directory")
print("Note: The Output File will be created in the current Directory")
print("---------------------------------------------")
inputFile = input("Input file name for writing to Blob:")
outputFile = input("Output file name for writing from Blob:")
print("----------------------------------------------")

inf = open(inputFile, 'rb')
ouf = open(outputFile, 'wb')

toReadIntoBlob = coral.Blob(os.path.getsize(inputFile))
print(toReadIntoBlob)
print("To Read Into Blob Size = " , toReadIntoBlob.size())
toWriteBlob = coral.Blob(os.path.getsize(inputFile))
print(toWriteBlob)
print("To Write Blob Size = " , toWriteBlob.size())

inf.readinto(toReadIntoBlob)

attrList = coral.AttributeList()
attrList.extend("blob1","blob")

class PyCoralBlobReadWriteTest(unittest.TestCase):
	def setUp(self):
		print('[OVAL] setUp')
	
	def testBlobModule(self):

		print('[OVAL] Test PyCoral Blob Module')
		print(coral.Blob.__doc__)

                attrList['blob1'].setData(toReadIntoBlob)
                toWriteBlob = attrList[0].data()
                ouf.write(toWriteBlob)

		ouf.close()
                ouf1 = open(outputFile, 'rb')

		print("File Comparision:: for Input File = " , inputFile , " and OutputFile = " , outputFile , "is " , filecmp.cmp(inputFile,outputFile))
		inf.close()
		ouf1.close()

 

	def tearDown(self):
		print('[OVAL] tearDown')

if __name__=="__main__":
	unittest.main() 	
