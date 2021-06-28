import coral 
import unittest

date1 = coral.Date()
date2 = coral.Date(2007,9,10)
timestamp1 = coral.TimeStamp()
timestamp2 = coral.TimeStamp(2007, 12, 21, 12, 10, 30, 10000000)
blob1 = coral.Blob(100)
blob2 = coral.Blob(200)

list1 = coral.AttributeList()
list1.extend("a","date")
list1.extend("b","time stamp")
list1.extend("c","blob")
list1BeforeMerging = len(list1)
list1[0].setData(date1)
list1[1].setData(timestamp1)
list1[2].setData(blob1)
print "------------------------------------------------------"
print "Length of list1 before merging = " , list1BeforeMerging
print "Contents of list1 before merging"
for attr in list1:
 print "Atribute Name =" , attr.specification().name() , ":TypeName =" , attr.specification().typeName() , ":Value = " , attr.data()
print "------------------------------------------------------"

list2 = coral.AttributeList()
list2.extend("c","blob")
list2.extend("d","time stamp")
list2.extend("e","date")
list2BeforeMerging = len(list2)
list2[0].setData(blob1)
list2[1].setData(timestamp1)
list2[2].setData(date1)
print "------------------------------------------------------"
print "Length of list2 before merging = " , list2BeforeMerging
print "Contents of list2 before merging"
for attr in list2:
 print "Atribute Name =" , attr.specification().name() , ":TypeName =" , attr.specification().typeName() , ":Value = " , attr.data()
print "------------------------------------------------------"

list1.merge(list2)
list1AfterMerging = len(list1)

class PyCoralAttributeListTest(unittest.TestCase):
	def setUp(self):
		print '[OVAL] setUp'
	
	def testAttributeListModule(self):

		list1.merge(list2)
		list1AfterMerging = len(list1)

                list1['a'].setData(date2)
                list1[1].setData(timestamp2)
                list1['c'].setData(blob2)
                list1[3].setData(timestamp2)
                list1['e'].setData(date2)
		print coral.AttributeList.__doc__
		print "------------------------------------------------------"
                print "Length of list1 after merging with list2 and changing data = ", list1AfterMerging 
                print "Contents of list1 after merging"

                for attr in list1:
 		 print "Atribute Name =" , attr.specification().name() , ":TypeName =" , attr.specification().typeName() , ":Value = " , attr.data()
		print "------------------------------------------------------"

		print "Length of list2 after merging = " , len(list2)
                print "Contents of list2 after merging"
                for attr in list2:
 		 print "Atribute Name =" , attr.specification().name() , ":TypeName =" , attr.specification().typeName() , ":Value = " , attr.data()
		print "------------------------------------------------------"


	def tearDown(self):
		print '[OVAL] tearDown'

if __name__=="__main__":
	unittest.main() 	
