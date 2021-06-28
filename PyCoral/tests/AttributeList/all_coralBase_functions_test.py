import coral
import sys
import gc
import cPickle
import unittest

#startRef = sys.gettotalrefcount();
for temp in range(1,200):

 x=coral.AttributeList()
 y=coral.AttributeList()
 z=coral.AttributeList()
 w=coral.AttributeList()
 print "AttributeList x RefCount = ", sys.getrefcount(x)

 x.extend("a","int")
 x.extend("b","int")
 x.extend("c","int")
 y.extend("c","int")
 y.extend("d","int")
 y.extend("e","int")


 z.extend("a","int")
 z.extend("b","int")
 z.extend("c","int")
 z.extend("d","int")
 z.extend("e","int")

 w.extend("a","int")
 w.extend("b","int")
 w.extend("c","int")
 w.extend("d","int")
 w.extend("e","int")

 l=len(x)
 m=x.size()
 x[0].setData(100)
 x['b'].setData(101)
 x[2].setData(102)
 print "Before Merging AttributeList (x)"
 for attr in x:
  print attr.data()
  del attr
 x.merge(y)
 x['c'].setData(103)
 x['d'].setData(104)
 x['e'].setData(105)
 print "After Merging AttributeList (x)"
 for attr in x:
  print attr.data()
  del attr

 z.copyData(x)

 print "After copyData AttributeList (z)"
 for attr in z:
  print attr.data()
  del attr

 w.fastCopyData(z)


 print "After fastcopyData AttributeList (w)"
 for attr in w:
  print attr.data()
  del attr

 print str(x)
 print str(y)
 print str(z)
 print str(w)

 print "Comparison Results of x and y = ", cmp(x,y)
 print "Comparison Results of x and z = ", cmp(x,z)
 print "Comparison Results of z and w = ", cmp(z,w)

 print "Element w[0] isNull OR not: " ,w[0].isNull()
 print str(w[0])
 w[0].setData(None)
 print "Element w[0] isNull OR not after setData(None): " ,w[0].isNull()
 print str(w[0])
 w[0].shareData(w[1])
 print "Elements w[0] and w[1] after shareData = " , str(w[0]) , str(w[1])


 spec = w[0].specification()
 print "Attribute Specification details of w[0] = ", spec.name(), spec.typeName()
 print str(spec)
 print "Comparison Results of specifications of w[0] and w[1] = ", cmp(w[0].specification(),w[1].specification())
 print "Comparison Results of specifications of z[2] and w[2] = ", cmp(z[2].specification(),w[2].specification())
 
 f=coral.Date()
 g=coral.TimeStamp()
 h=coral.Blob()

 w.extend("f","date");
 w.extend("g","time stamp");
 w.extend("h","blob");

 li1 = []
 li2 = []
 for j in range(1,500):
  li1.append(j)
  del j

 cPickle.dump(li1,h,1)

 w[5].setData(f)
 w[6].setData(g)
 w[7].setData(h)

 for i in range(5,8):
  print w[i].data()
  if (i==5):
   print w[i].data().day(), w[i].data().month(), w[i].data().year()
  if (i==6):
   print w[i].data().day(), w[i].data().month(), w[i].data().year(), w[i].data().hour() , w[i].data().minute() , w[i].data().second() , w[i].data().nanosecond()
  if (i==7):
   #contents = w[i].data().read(7)
   #print sys.getrefcount(h)
   li2 = cPickle.loads(w[i].data().readline())
   #li2 = cPickle.load(w[i].data())
   print "[" ,
   for j in li2:
    print j,
    del j
   print "]"
 blobSize = h.size()
 print "Size of the Blob = " , blobSize
 h.resize(200)
 print "Size of the Blob after resize = " , h.size() 
 h.extend(100)
 print "Size of the Blob after extend = " , h.size() 
 aBlob = coral.Blob(80)
 h.append(aBlob)
 print "Size of the Blob after append = " , h.size() 
  
 del aBlob
 del blobSize
 #del contents
 del li1
 del li2
 del spec
 del l
 del m
 del x
 del y
 del z
 del w
 del f
 del g
 del h
 del i

#print "Start RefCount = " ,startRef
#del startRef
del temp
#print "End RefCount = " ,sys.gettotalrefcount()



##############################################################################
#for o in x:
# print hex(id(o)), type(o), sys.getrefcount(o), len(gc.get_referrers(o))
#print len(gc.get_referrers(x))
#print sys.getobjects(0)
##############################################################################
