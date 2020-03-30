/*Copyright (c) 2014 The Paradox Game Converters Project

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/



/*Copyright (c) 2010 Rolf Andreassen
 
 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:
 
 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/



#ifndef OBJECT_H
#define OBJECT_H


#include <map> 
#include <vector>
#include <string> 
using namespace std;



class Object {
public:
	virtual ~Object() { }

	virtual string getKey() = 0;
	virtual vector<string> getKeys() = 0;
	virtual vector<Object*> getValue(string key) const=0;
	virtual string getLeaf() const = 0;
	virtual string getLeaf(string leaf) const=0;
	virtual vector<Object*> getLeaves() = 0;
	virtual void keyCount()=0;
	virtual void keyCount(map<string, int>& counter)=0;
	virtual string getToken(int index)=0;
	virtual vector<string> getTokens() = 0;
	virtual int numTokens() = 0; // maybe not used?
	virtual bool isLeaf() const = 0;
	virtual double safeGetFloat(string k, double def = 0) = 0;
	virtual string safeGetString(string k, string def = "") = 0;
	virtual int safeGetInt(string k, int def = 0) = 0;
	virtual Object* safeGetObject(string k, Object* def = 0) = 0;
	virtual string toString() const = 0;
};


typedef vector<Object*>::iterator objiter;
typedef vector<Object*> objvec; 
typedef map<string, Object*> stobmap;
typedef map<string, string> ststmap;
typedef map<Object*, Object*> obobmap;

#endif	// OBJECT_H
