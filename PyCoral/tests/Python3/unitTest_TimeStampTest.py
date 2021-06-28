import coral

import unittest

thisMoment = coral.TimeStamp()
anotherMoment = coral.TimeStamp(2007, 12, 21, 12, 10, 30, 10000000)
otherMoment = coral.TimeStamp(day=1, month = 10, year = 2007, hour = 12, minute = 30, second = 20, nanosecond = 230000000)

class PyCoralTimeTest(unittest.TestCase):
	def setUp(self):
		print('[OVAL] setUp')
	
	def testTimeStampModule(self):

		print('[OVAL] Test PyCoral TimeStamp module')
		print(coral.TimeStamp.__doc__)
		print("thisMoment   :: ",thisMoment.day(),"/",thisMoment.month(),"/",thisMoment.year(),"/",thisMoment.hour(),"/",thisMoment.minute(),"/",thisMoment.second(),"/", (thisMoment.nanosecond() / 100000000))  

		print("anotherMoment:: ",anotherMoment.day(),"/",anotherMoment.month(),"/",anotherMoment.year(),"/",anotherMoment.hour(),"/",anotherMoment.minute(),"/",anotherMoment.second(),"/", (anotherMoment.nanosecond() / 100000000))  

		print("otherMoment::   ",otherMoment.day(),"/",otherMoment.month(),"/",otherMoment.year(),"/",otherMoment.hour(),"/",otherMoment.minute(),"/",otherMoment.second(),"/", (otherMoment.nanosecond() / 100000000))  

	def tearDown(self):
		print('[OVAL] tearDown')

if __name__=="__main__":
	unittest.main() 	
