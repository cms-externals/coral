#!/usr/bin/env python
import coral 
import sys
import unittest

class PyCoralAttributeListTest(unittest.TestCase):

    def setUp(self):
        pass

    def test01AttributeListModule(self): # Ported to unittest CORALCOOL-2940
        print coral.AttributeList.__doc__
        list1 = coral.AttributeList()
        date1 = coral.Date()
        x = 'a'
        xType1 = 'in'
        xType2 = 't'
        list1.extend(x,xType1+xType2)
        list1.extend("bx","date")
        list1.extend("c","int") # AL1 has size 3
        list2 = coral.AttributeList()
        list2.extend("c","int")
        list2.extend("d","long")
        list2.extend("e","long double") # AL2 has size 3
        list1.merge(list2) # AL1 has now size 5 (c,d,e shared with AL2)
        list3 = coral.AttributeList()
        list3.extend("a","int")
        list3.extend("b","date") # NB: AL1 has bx, AL3 has b
        list3.extend("c","int")
        list3.extend("d","long")
        list3.extend("e","long double") # AL3 has size 5
        list4 = coral.AttributeList()
        list4.extend("c","int")
        list4.extend("d","long")
        list4.extend("e","long double") # AL4 has size 3
        list1[0].setData(100)
        list1['bx'].setData(date1)
        list1[2].setData(1000) # also sets it for AL2
        list1[3].setData(10000) # also sets it for AL2
        list1[4].setData(100000L) # also sets it for AL2
        list3.copyData(list1) # NB: weird, copies AL1[bx] into AL3[b]
        list4.fastCopyData(list2) # NB: no type checking, but c,d,e match types
        print "---LIST1----"
        for attr in iter(list1):
            print attr.data()
        print "---LIST1----\n", list1
        print "---LIST2----\n", list2
        print "---LIST3----\n", list3
        print "---LIST4----\n", list4
        self.assertEqual(list1.size(),len(list1))
        self.assertEqual(list1.size(),5)
        self.assertEqual(list2.size(),3)
        self.assertEqual(list3.size(),5)
        self.assertEqual(list4.size(),3)
        self.assertTrue( list1 != list2 )
        self.assertTrue( list1 == list3 ) # WEIRD! Element names [2] differ, bx!=b
        self.assertEqual( list1, list3 ) # WEIRD as above...
        self.assertTrue( list3 != list4 )
        self.assertTrue( list2 == list4 )

    def test02AttributeListCORALCOOL605(self): # CORALCOOL-605
        list1 = coral.AttributeList()
        list1.extend("i1","unsigned int")
        list1.extend("i2","unsigned int")
        list1["i1"].setData(1)
        list1["i2"].setData(-1)
        self.assertEqual(list1["i1"].data(),1)
        self.assertEqual(list1["i2"].data(),4294967295) # WEIRD! CORALCOOL-605
        #self.assertEqual(list1["i2"].data(),-1) # this test failsprint list1

    def tearDown(self):
        pass

if __name__=="__main__":
    #unittest.main() writes to stderr, leading to a failure even when the tests succeeds.
    unittest.main(testRunner=unittest.TextTestRunner(stream=sys.stdout))

