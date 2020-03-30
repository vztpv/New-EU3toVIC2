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



#ifndef EU3RELIGION_H_
#define EU3RELIGION_H_

#include <string>
#include <map>
using namespace std;

class Object;

class EU3Religion
{
public:
	EU3Religion(Object*, string group);

	// exactly one of these four functions should return true for any given pairing
	bool isSameReligion(const EU3Religion* other) const;	// e.g. catholic <-> catholic
	bool isRelatedTo(const EU3Religion* other) const;		// e.g. orthodox <-> catholic
	bool isInfidelTo(const EU3Religion* other) const;		// e.g. sunni <-> catholic

	string getName() const { return name; }
	string getGroup() const { return group; }

	static void parseReligions(Object* obj);
	static EU3Religion* getReligion(string name);

	static map<string, EU3Religion*> getAllReligions() { return all_religions; }

private:
	string name;
	string group;

	static map<string, EU3Religion*> all_religions;
};

#endif // EU3RELIGION_H_