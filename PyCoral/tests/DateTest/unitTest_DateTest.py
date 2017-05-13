import coral 

import unittest

theDay, anotherDay, otherDay, someDifferentDay = coral.Date(), coral.Date(2006,9,10), coral.Date(day=10, month=9, year=2006), coral.Date(2007)

class PyCoralDateTest(unittest.TestCase):
	def setUp(self):
		print '[OVAL] setUp'
	
	def testDateModule(self):

		print '[OVAL] Test PyCoral Date Module'
		print coral.Date.__doc__
		print "theDay    :: ",theDay.day(),"/",theDay.month(),"/",theDay.year()
		print "anotherDay:: ",anotherDay.day(),"/",anotherDay.month(),"/",anotherDay.year()
		print "otherDay:: ",otherDay.day(),"/",otherDay.month(),"/",otherDay.year()
		print "someDifferentDay:: ",someDifferentDay.day(),"/",someDifferentDay.month(),"/",someDifferentDay.year()
		print "-----------------------------"
                print "Note: Comparison result 0 = True, -1 = False OR Error"
		print "-----------------------------"
		print "Camparison Results of anotherDay & otherDay =", cmp(anotherDay,otherDay)
		print "Camparison Results of anotherDay & someDifferentDay =", cmp(anotherDay,someDifferentDay)

	def tearDown(self):
		print '[OVAL] tearDown'

if __name__=="__main__":
	unittest.main() 	
