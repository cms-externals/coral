import os
import coral 
import unittest
import sys

#startRef = sys.gettotalrefcount()
#endRef = sys.gettotalrefcount()
dateData = coral.Date(year=2006, month=10, day=10)
timeStampData = coral.TimeStamp()
blobData = coral.Blob()

aList1 = coral.AttributeList()
aList2 = coral.AttributeList()

class PyCoralAttributeListTest(unittest.TestCase):
	def setUp(self):
		print('[OVAL] setUp')
		print(coral.AttributeList.__doc__)		
	
	def testAttributeListSimpleData(self):
		print('[OVAL] testAttributeListSimpleData')

		# Extend AttributeList
		aList1.extend("c","char")
		aList1.extend("i","int")
		aList1.extend("dd","date")
		aList1.extend("tm","time stamp")
                aList1.extend("blob1","blob")                                
                aList1.extend("floatVar", "float")
                aList1.extend("uSlongVar", "unsigned long")
                aList1.extend("sLongVar", "long")
                aList1.extend("boolVar", "bool")
		
                print("Lenght of aList1 ", len(aList1))
                print(" ")

		print("Extend other attribute list")
		
		# Extend second AttributeList
		aList2.extend("d","char")
		aList2.extend("e","int")
     		aList2.extend("ddList2","date")           
     		aList2.extend("timeList1","time stamp")           
     		aList2.extend("timeList2","time stamp")
                aList2.extend("floatVarList2", "float")
                aList2.extend("longLongVar", "long long")
                aList2.extend("doubleVar", "double")
                aList2.extend("longDoubleVar", "long double")
                aList2.extend("blobList2", "blob")
                aList2.extend("boolVar2", "bool")
                
                print("Assgn values to aList1 attributes")
		
		aList1['c'].setData('z')
                j = 10
		aList1['i'].setData(65537 -j)
                aList1['dd'].setData(dateData)
                aList1['tm'].setData(timeStampData)
                aList1['floatVar'].setData(3.0003)
                aList1['uSlongVar'].setData(4294967295)
                aList1['sLongVar'].setData(2147483647)
                aList1['boolVar'].setData(True)
                
		aList2['d'].setData(None) # Set NULL
                aList2['e'].shareData(aList1[1])
                aList2['ddList2'].shareData(aList1[2])
                aList2['timeList1'].setData(timeStampData)
                aList2['timeList2'].setData(timeStampData)
                aList2['floatVarList2'].shareData(aList1[5])
                aList2['longLongVar'].setData(9223372036854775807)
                aList2['doubleVar'].setData(2147483647)
                aList2['longDoubleVar'].setData(36778826444)
                aList2['blobList2'].shareData(aList1[4])
		aList2['boolVar2'].setData(False)

                
                # Read/ Write test of BLOB
                inputFile = input("File name to be read into BLOB: ")
                outputFile = input("Output file to be read from BLOB: ")
                
                inf = open(inputFile, 'rb')
                ouf = open(outputFile, 'wb')

                toReadIntoBlob = coral.Blob(os.path.getsize(inputFile))
                toWriteBlob = coral.Blob(os.path.getsize(inputFile))

                inf.readinto(toReadIntoBlob)

                aList1['blob1'].setData(toReadIntoBlob)
                toWriteBlob = aList1[4].data()
                
                ouf.write(toWriteBlob)
                
                print("Attribute in aList1 ...", " size = ", len(aList1))
                
                for attr in aList1:
                    print("Name = ", attr.specification().name(), "Type Name = ", attr.specification().name(), " Value = ", attr.data())
                        
                print("Attribute in aList2 ...", " size = ", len(aList2))
                
                for attr2 in aList2:
                    print(attr2)

                print("Merge aList1 with aList2 ")        
                aList1.merge(aList2)
                
                print("Attribute in aList1 ...", " size = ", len(aList1))

                for attribute in aList1:
                    print(attribute)
                                     
	def tearDown(self):
		print('[OVAL] tearDown')
		#endRef = sys.gettotalrefcount()
		#print "StartRef = " , startRef, "EndRef = ", endRef

if __name__=="__main__":
	unittest.main() 	
