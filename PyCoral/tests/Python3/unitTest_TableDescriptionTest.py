import coral 

import unittest

description = coral.TableDescription()
attrList = coral.AttributeList()
attrList.extend("Id","int")
attrList.extend("Data","string")
attrList.extend("x","float")
attrList.extend("y","double")
attrList.extend("a","float")
attrList.extend("b","double")

name = attrList[0].specification().name()
tName = attrList[0].specification().typeName()

data = attrList[1].specification().name()
tData = attrList[1].specification().typeName()

x = attrList[2].specification().name()
tX = attrList[2].specification().typeName()

y = attrList[3].specification().name()
tY = attrList[3].specification().typeName()

a = attrList[4].specification().name()
tA = attrList[4].specification().typeName()

b = attrList[5].specification().name()
tB = attrList[5].specification().typeName()

class PyCoralTableDescriptionTest(unittest.TestCase):
	def setUp(self):
		print('[OVAL] setUp')
	
	def testTableDescriptionModule(self):

	 try:
		print('[OVAL] Test PyCoral TableDescription Module')
		print(coral.TableDescription.__doc__)
		description.setName("MyTable")
                description.insertColumn(name, tName)
                description.insertColumn(data, tData, 1000, 1)
		description.setPrimaryKey(name)
                description.insertColumn(x, tX)
                description.insertColumn(y, tY)
                description.insertColumn(a, tA)
                description.insertColumn(b, tB)
                uid3 = (name,data)
		description.setUniqueConstraint(uid3,"uID3",1)
		description.setUniqueConstraint(a,"uID1")
		description.setUniqueConstraint(b,"uID2")
		print("Table Description: Name =" , description.name())
		print("Tbale Description: NoOfColumns =" , description.numberOfColumns())
		if (description.numberOfColumns() != 6):
	         print("Unexpected Number of Columns 1") 
                description.dropColumn("x")
		if (description.numberOfColumns() != 5):
	         print("Unexpected Number of Columns 2") 
                description.setNotNullConstraint("Id")
                description.setNotNullConstraint("y")
                description.createIndex("idx","y",1)
                col1 = description.columnDescription(0)
                if ( not col1.isNotNull() ):
                 print("Missing Not Null Attribute")
                col2 = description.columnDescription("y")
                if ( col2.indexInTable() != 2 ):
                 print("Unexpected column Index")
		if ( not description.hasPrimaryKey() ):
		 print("Primary Key not found")
		primaryKey = description.primaryKey()
		if ( len(primaryKey.columnNames()) != 1):
		 print("Unexpected number of columns in primary key") 
		if ( primaryKey.columnNames()[0] != "Id" ):
		 print("Unexpected column name in the primary key")
		print("Number of UniqueConstraints =" , description.numberOfUniqueConstraints())
		if ( description.numberOfUniqueConstraints() != 3 ):
		 print("Unexpected number of unique constraint columns")

         except Exception as e:
            print(str(e))

	def tearDown(self):
		print('[OVAL] tearDown')

if __name__=="__main__":
	unittest.main() 	
