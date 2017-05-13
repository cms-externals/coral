import coral 
import sys
import unittest

date1 = coral.Date()
list1 = coral.AttributeList()
x = 'a'
xType1 = 'in'
xType2 = 't'
list1.extend(x,xType1+xType2)
list1.extend("bx","date")
list1.extend("c","int")


list2 = coral.AttributeList()
list2.extend("c","int")
list2.extend("d","long")
list2.extend("e","long double")

list1.merge(list2)


list3 = coral.AttributeList()
list3.extend("a","int")
list3.extend("b","date")
list3.extend("c","int")
list3.extend("d","long")
list3.extend("e","long double")

list4 = coral.AttributeList()
list4.extend("c","int")
list4.extend("d","long")
list4.extend("e","long double")

class PyCoralAttributeListTest(unittest.TestCase):
	def setUp(self):
		print '[OVAL] setUp'
	
	def testAttributeListModule(self):

		print coral.AttributeList.__doc__
                print len(list1)
                list1[0].setData(100)
                list1['bx'].setData(date1)
                list1[2].setData(1000)
                list1[3].setData(10000)
                list1[4].setData(100000L)

                list3.copyData(list1)
                list4.fastCopyData(list2)

                print "---LIST1----"
                for attr in list1:
                 print attr.data()

                print "---LIST2----"
                for attr in iter(list2):
                 print attr.data()

                print "---LIST3----"
                for attr in list3:
                 print attr.data()

                print "---LIST4----"
                for attr in list4:
                 print attr.data()

		print "List1 = ....", str(list1)
		print "List2 = ....", str(list2)
		print "List3 = ....", str(list3)
		print "List4 = ....", str(list4)

                print "-----------------------------------"
                print "Note: 0 = True, -1 = False Or Error"
                print "-----------------------------------"
                print "Comparison Result of list1,list2 = ", cmp(list1,list2)
                print "Comparison Result of list1,list3 = ", cmp(list1,list3)
                print "Comparison Result of list3,list4 = ", cmp(list3,list4)
                print "Comparison Result of list2,list4 = ", cmp(list2,list4)

		print "Length of List1 = " , len(list1)
		print "Length of List2 = " , list2.size()
		print "Length of List3 = " , list3.size()
		print "Length of List4 = " , len(list4)

	def tearDown(self):
		print '[OVAL] tearDown'

if __name__=="__main__":
	unittest.main() 	
