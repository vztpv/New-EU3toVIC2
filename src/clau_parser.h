

#ifndef CLAU_PARSER_H
#define CLAU_PARSER_H

#include <iostream>
#include <vector>
#include <set>
#include <stack>
#include <string>

#include <fstream>

#include <algorithm>

#include <thread>


#include <map> 
#include <vector>
#include <string> 
using namespace std;
#include <assert.h>
#include "./Source/Parsers/Object.h"

namespace wiz {
	template <typename T>
	inline T pos_1(const T x, const int base = 10)
	{
		if (x >= 0) { return x % base; }// x - ( x / 10 ) * 10; }
		else { return (x / base) * base - x; }
		// -( x - ( (x/10) * 10 ) )
	}


	template <typename T> /// T <- char, int, long, long long...
	std::string toStr(const T x) /// chk!!
	{
		const int base = 10;
		if (base < 2 || base > 16) { return "base is not valid"; }
		T i = x;

		const int INT_SIZE = sizeof(T) << 3; ///*8
		char* temp = new char[INT_SIZE + 1 + 1]; /// 1 NULL, 1 minus
		std::string tempString;
		int k;
		bool isMinus = (i < 0);
		temp[INT_SIZE + 1] = '\0'; //

		for (k = INT_SIZE; k >= 1; k--) {
			T val = pos_1<T>(i, base); /// 0 ~ base-1
									   /// number to ['0'~'9'] or ['A'~'F']
			if (val < 10) { temp[k] = val + '0'; }
			else { temp[k] = val - 10 + 'A'; }

			i /= base;

			if (0 == i) { // 
				k--;
				break;
			}
		}

		if (isMinus) {
			temp[k] = '-';
			tempString = std::string(temp + k);//
		}
		else {
			tempString = std::string(temp + k + 1); //
		}
		delete[] temp;

		return tempString;
	}


	class LoadDataOption
	{
	public:
		char LineComment = '#';	// # 
		char Left = '{', Right = '}';	// { }
		char Assignment = '=';	// = 
		char Removal = ' ';		// ',' 
	};

	inline bool isWhitespace(const char ch)
	{
		switch (ch)
		{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
		case '\v':
		case '\f':
			return true;
			break;
		}
		return false;
	}


	inline int Equal(const long long x, const long long y)
	{
		if (x == y) {
			return 0;
		}
		return -1;
	}

	class InFileReserver
	{
	private:
		// todo - rename.
		static long long Get(long long position, long long length, char ch, const wiz::LoadDataOption& option) {
			long long x = (position << 32) + (length << 3) + 0;

			if (length != 1) {
				return x;
			}

			if (option.Left == ch) {
				x += 2; // 010
			}
			else if (option.Right == ch) {
				x += 4; // 100
			}
			else if (option.Assignment == ch) {
				x += 6;
			}

			return x;
		}

		static long long GetIdx(long long x) {
			return (x >> 32) & 0x00000000FFFFFFFF;
		}
		static long long GetLength(long long x) {
			return (x & 0x00000000FFFFFFF8) >> 3;
		}
		static long long GetType(long long x) { //to enum or enum class?
			return (x & 6) >> 1;
		}
		static bool IsToken2(long long x) {
			return (x & 1);
		}

		static void _Scanning(char* text, long long num, const long long length,
			long long*& token_arr, long long& _token_arr_size, const LoadDataOption& option) {


			long long token_arr_size = 0;

			{
				int state = 0;

				long long token_first = 0;
				long long token_last = -1;

				long long token_arr_count = 0;

				for (long long i = 0; i < length; ++i) {
					const char ch = text[i];

					if ('\"' == ch) {

						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Get(token_first + num, token_last - token_first + 1, text[token_first], option);
							token_arr_count++;
						}

						token_first = i;
						token_last = i;

						token_first = i + 1;
						token_last = i + 1;

						{//
							token_arr[num + token_arr_count] = 1;
							token_arr[num + token_arr_count] += Get(i + num, 1, ch, option);
							token_arr_count++;
						}

					}
					else if ('\\' == ch) {
						{//
							token_arr[num + token_arr_count] = 1;
							token_arr[num + token_arr_count] += Get(i + num, 1, ch, option);
							token_arr_count++;
						}
					}
					else if ('\n' == ch) {
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Get(token_first + num, token_last - token_first + 1, text[token_first], option);
							token_arr_count++;
						}
						token_first = i + 1;
						token_last = i + 1;

						{//
							token_arr[num + token_arr_count] = 1;
							token_arr[num + token_arr_count] += Get(i + num, 1, ch, option);
							token_arr_count++;
						}
					}
					else if ('\0' == ch) {
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Get(token_first + num, token_last - token_first + 1, text[token_first], option);
							token_arr_count++;
						}
						token_first = i + 1;
						token_last = i + 1;

						{//
							token_arr[num + token_arr_count] = 1;
							token_arr[num + token_arr_count] += Get(i + num, 1, ch, option);
							token_arr_count++;
						}
					}
					else if (option.LineComment == ch) {
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Get(token_first + num, token_last - token_first + 1, text[token_first], option);
							token_arr_count++;
						}
						token_first = i + 1;
						token_last = i + 1;

						{//
							token_arr[num + token_arr_count] = 1;
							token_arr[num + token_arr_count] += Get(i + num, 1, ch, option);
							token_arr_count++;
						}

					}
					else if (isWhitespace(ch)) {
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Get(token_first + num, token_last - token_first + 1, text[token_first], option);
							token_arr_count++;
						}
						token_first = i + 1;
						token_last = i + 1;
					}
					else if (option.Left == ch) {
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Get(token_first + num, token_last - token_first + 1, text[token_first], option);
							token_arr_count++;
						}

						token_first = i;
						token_last = i;

						token_arr[num + token_arr_count] = Get(token_first + num, token_last - token_first + 1, text[token_first], option);
						token_arr_count++;

						token_first = i + 1;
						token_last = i + 1;
					}
					else if (option.Right == ch) {
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Get(token_first + num, token_last - token_first + 1, text[token_first], option);
							token_arr_count++;
						}
						token_first = i;
						token_last = i;

						token_arr[num + token_arr_count] = Get(token_first + num, token_last - token_first + 1, text[token_first], option);
						token_arr_count++;

						token_first = i + 1;
						token_last = i + 1;

					}
					else if (option.Assignment == ch) {
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Get(token_first + num, token_last - token_first + 1, text[token_first], option);
							token_arr_count++;
						}
						token_first = i;
						token_last = i;

						token_arr[num + token_arr_count] = Get(token_first + num, token_last - token_first + 1, text[token_first], option);
						token_arr_count++;

						token_first = i + 1;
						token_last = i + 1;
					}
				}

				if (length - 1 - token_first + 1 > 0) {
					token_arr[num + token_arr_count] = Get(token_first + num, length - 1 - token_first + 1, text[token_first], option);
					token_arr_count++;
				}
				token_arr_size = token_arr_count;
			}

			{
				_token_arr_size = token_arr_size;
			}
		}

		static void ScanningNew(char* text, const long long length, const int thr_num,
			long long*& _token_arr, long long& _token_arr_size, const LoadDataOption& option)
		{
			std::vector<std::thread> thr(thr_num);
			std::vector<long long> start(thr_num);
			std::vector<long long> last(thr_num);

			{
				start[0] = 0;

				for (int i = 1; i < thr_num; ++i) {
					start[i] = length / thr_num * i;

					for (long long x = start[i]; x <= length; ++x) {
						if (isWhitespace(text[x]) || '\0' == text[x] ||
							option.Left == text[x] || option.Right == text[x] || option.Assignment == text[x]) {
							start[i] = x;
							break;
						}
					}
				}
				for (int i = 0; i < thr_num - 1; ++i) {
					last[i] = start[i + 1];
					for (long long x = last[i]; x <= length; ++x) {
						if (isWhitespace(text[x]) || '\0' == text[x] ||
							option.Left == text[x] || option.Right == text[x] || option.Assignment == text[x]) {
							last[i] = x;
							break;
						}
					}
				}
				last[thr_num - 1] = length + 1;
			}
			long long real_token_arr_count = 0;

			long long* tokens = new long long[length + 1];
			long long token_count = 0;

			std::vector<long long> token_arr_size(thr_num);

			for (int i = 0; i < thr_num; ++i) {
				thr[i] = std::thread(_Scanning, text + start[i], start[i], last[i] - start[i], std::ref(tokens), std::ref(token_arr_size[i]), option);
			}

			for (int i = 0; i < thr_num; ++i) {
				thr[i].join();
			}

			{
				long long _count = 0;
				for (int i = 0; i < thr_num; ++i) {
					for (long long j = 0; j < token_arr_size[i]; ++j) {
						tokens[token_count] = tokens[start[i] + j];
						token_count++;
					}
				}
			}

			int state = 0;
			long long qouted_start;
			long long slush_start;

			for (long long i = 0; i < token_count; ++i) {
				const long long len = GetLength(tokens[i]);
				const char ch = text[GetIdx(tokens[i])];
				const long long idx = GetIdx(tokens[i]);
				const bool isToken2 = IsToken2(tokens[i]);

				if (isToken2) {
					if (0 == state && '\"' == ch) {
						state = 1;
						qouted_start = i;
					}
					else if (0 == state && option.LineComment == ch) {
						state = 2;
					}
					else if (1 == state && '\\' == ch) {
						state = 3;
						slush_start = idx;
					}
					else if (1 == state && '\"' == ch) {
						state = 0;

						{
							long long idx = GetIdx(tokens[qouted_start]);
							long long len = GetLength(tokens[qouted_start]);
							long long type = GetType(tokens[qouted_start]);

							len = GetIdx(tokens[i]) - idx + 1;

							len -= 2;
							idx += 1;

							if (len >= 0) {
								tokens[real_token_arr_count] = Get(idx, len, type, option);
								real_token_arr_count++;
							}
						}
					}
					else if (3 == state) {
						if (idx != slush_start + 1) {
							--i;
						}
						state = 1;
					}
					else if (2 == state && ('\n' == ch || '\0' == ch)) {
						state = 0;
					}
				}
				else if (0 == state && !('\n' == ch || '\0' == ch)) { // '\\' case?
					tokens[real_token_arr_count] = tokens[i];
					real_token_arr_count++;
				}
			}

			{
				if (0 != state) {
					std::cout << "[ERRROR] state [" << state << "] is not zero \n";
				}
			}


			{
				_token_arr = tokens;
				_token_arr_size = real_token_arr_count;
			}
		}


		static void Scanning(char* text, const long long length,
			long long*& _token_arr, long long& _token_arr_size, const LoadDataOption& option) {

			long long* token_arr = new long long[length + 1];
			long long token_arr_size = 0;

			{
				int state = 0;

				long long token_first = 0;
				long long token_last = -1;

				long long token_arr_count = 0;

				for (long long i = 0; i <= length; ++i) {
					const char ch = text[i];

					if (0 == state) {
						if (option.LineComment == ch) {
							token_last = i - 1;
							if (token_last - token_first + 1 > 0) {
								token_arr[token_arr_count] = Get(token_first, token_last - token_first + 1, text[token_first], option);
								token_arr_count++;
							}

							state = 3;
						}
						else if ('\"' == ch) {
							state = 1;
						}
						else if (isWhitespace(ch) || '\0' == ch) {
							token_last = i - 1;
							if (token_last - token_first + 1 > 0) {
								token_arr[token_arr_count] = Get(token_first, token_last - token_first + 1, text[token_first], option);
								token_arr_count++;
							}
							token_first = i + 1;
							token_last = i + 1;
						}
						else if (option.Left == ch) {
							token_last = i - 1;
							if (token_last - token_first + 1 > 0) {
								token_arr[token_arr_count] = Get(token_first, token_last - token_first + 1, text[token_first], option);
								token_arr_count++;
							}

							token_first = i;
							token_last = i;

							token_arr[token_arr_count] = Get(token_first, token_last - token_first + 1, text[token_first], option);
							token_arr_count++;

							token_first = i + 1;
							token_last = i + 1;
						}
						else if (option.Right == ch) {
							token_last = i - 1;
							if (token_last - token_first + 1 > 0) {
								token_arr[token_arr_count] = Get(token_first, token_last - token_first + 1, text[token_first], option);
								token_arr_count++;
							}
							token_first = i;
							token_last = i;

							token_arr[token_arr_count] = Get(token_first, token_last - token_first + 1, text[token_first], option);
							token_arr_count++;

							token_first = i + 1;
							token_last = i + 1;

						}
						else if (option.Assignment == ch) {
							token_last = i - 1;
							if (token_last - token_first + 1 > 0) {
								token_arr[token_arr_count] = Get(token_first, token_last - token_first + 1, text[token_first], option);
								token_arr_count++;
							}
							token_first = i;
							token_last = i;

							token_arr[token_arr_count] = Get(token_first, token_last - token_first + 1, text[token_first], option);
							token_arr_count++;

							token_first = i + 1;
							token_last = i + 1;
						}
					}
					else if (1 == state) {
						if ('\\' == ch) {
							state = 2;
						}
						else if ('\"' == ch) {
							state = 0;
						}
					}
					else if (2 == state) {
						state = 1;
					}
					else if (3 == state) {
						if ('\n' == ch || '\0' == ch) {
							state = 0;

							token_first = i + 1;
							token_last = i + 1;
						}
					}
				}

				token_arr_size = token_arr_count;

				if (0 != state) {
					std::cout << "[" << state << "] state is not zero.\n";
				}
			}

			{
				_token_arr = token_arr;
				_token_arr_size = token_arr_size;
			}
		}


		static std::pair<bool, int> Scan(std::ifstream& inFile, const int num, const wiz::LoadDataOption& option, int thr_num,
			char*& _buffer, long long* _buffer_len, long long*& _token_arr, long long* _token_arr_len)
		{
			if (inFile.eof()) {
				return { false, 0 };
			}

			long long* arr_count = nullptr; //
			long long arr_count_size = 0;

			std::string temp;
			char* buffer = nullptr;
			long long file_length;

			{
				inFile.seekg(0, inFile.end);
				unsigned long long length = inFile.tellg();
				inFile.seekg(0, inFile.beg);

				file_length = length;
				buffer = new char[file_length + 1]; // 

				// read data as a block:
				inFile.read(buffer, file_length);

				buffer[file_length] = '\0';

				{
					//int a = clock();
					long long* token_arr;
					long long token_arr_size;

					if (thr_num == 1) {
						Scanning(buffer, file_length, token_arr, token_arr_size, option);
					}
					else {
						ScanningNew(buffer, file_length, thr_num, token_arr, token_arr_size, option);
					}

					//int b = clock();
				//	std::cout << b - a << "ms\n";
					_buffer = buffer;
					_token_arr = token_arr;
					*_token_arr_len = token_arr_size;
					*_buffer_len = file_length;
				}
			}

			return{ true, 1 };
		}

	private:
		std::ifstream* pInFile;
	public:
		int Num;
	public:
		explicit InFileReserver(std::ifstream& inFile)
		{
			pInFile = &inFile;
			Num = 1;
		}
		bool end()const { return pInFile->eof(); } //
	public:
		bool operator() (const wiz::LoadDataOption& option, int thr_num, char*& buffer, long long* buffer_len, long long*& token_arr, long long* token_arr_len)
		{
			bool x = Scan(*pInFile, Num, option, thr_num, buffer, buffer_len, token_arr, token_arr_len).second > 0;
			return x;
		}
	};

	class Type : public Object {
	private:
		std::string name;

	public:
		explicit Type(const std::string& name = "", const bool valid = true) : name(name) { }//chk();  }
		explicit Type(std::string&& name, const bool valid = true) : name(move(name)) { }//chk(); }
		Type(const Type& type)
			: name(type.name)
		{
			//chk();
		}
		virtual ~Type() { }
		bool IsFail() const { // change body
			return "" == name;
		}
		std::string& GetName() {
			return name;
		}
		const std::string& GetName() const {
			return name;
		}
		void SetName(const std::string& name)
		{
			this->name = name;

			//chk();
		}
		void SetName(std::string&& name)
		{
			this->name = name;

			//chk();
		}
		bool operator==(const Type& t) const {
			return name == t.name;
		}
		bool operator<(const Type& t) const {
			return name < t.name;
		}
		Type& operator=(const Type& type)
		{
			name = type.name;
			return *this;
		}
		void operator=(Type&& type)
		{
			name = std::move(type.name);
		}
	};

	template < class T >
	class ItemType : public Type {
	public:
		typedef T item_type; //
	private:
		//std::vector<T> arr;
		//std::vector<T> arr;
		T data;
		bool inited;
	public:
		ItemType(const ItemType<T>& ta) : Type(ta), data(ta.data), inited(ta.inited)
		{

		}
		ItemType(ItemType<T>&& ta) noexcept : Type(std::move(ta.GetName()))
		{
			data = std::move(ta.data);
			inited = ta.inited;
		}
	public:
		explicit ItemType()
			: Type("", true), inited(false) { }
		explicit ItemType(const std::string& name, const T& value, const bool valid = true)
			:Type(name, valid), data(value), inited(true)
		{

		}
		explicit ItemType(std::string&& name, T&& value, const bool valid = true)
			:Type(std::move(name), valid), data(std::move(value)), inited(true)
		{

		}
		virtual ~ItemType() { }
	public:
		void Remove(const int idx = 0)
		{
			data = T();
			inited = false;
		}
		bool Push(const T& val) { /// do not change..!!
			if (inited) { throw "ItemType already inited"; }
			data = val;
			inited = true;

			return true;
		}
		bool Push(T&& val) {
			if (inited) { throw "ItemType already inited"; }
			data = std::move(val);
			inited = true;

			return true;
		}
		T& Get(const int index = 0) {
			if (!inited) {
				throw "ItemType, not inited";
			}
			return data;
		}
		const T& Get(const int index = 0) const {
			if (!inited) {
				throw "ItemType, not inited";
			}
			return data;
		}
		void Set(const int index, const T& val) {
			if (!inited) {
				throw "ItemType, not inited";
			} // removal?
			data = val;
		}
		void Set(const int index, T&& val) {
			if (!inited) {
				throw "ItemType, not inited";
			} // removal?
			data = std::move(val);
		}
		int size()const {
			return inited ? 1 : 0;
		}
		bool empty()const { return !inited; }
		std::string ToString()const
		{
			if (Type::GetName().empty()) {
				return Get(0);
			}
			return Type::GetName() + " = " + Get(0);
		}
	public:
		ItemType<T>& operator=(const ItemType<T>& ta)
		{
			ItemType<T> temp = ta;

			Type::operator=((const ItemType<T>&)ta);
			data = std::move(temp.data);
			inited = temp.inited;
			return *this;
		}
		void operator=(ItemType<T>&& ta)
		{
			Type::operator=(ta);
			if (data == ta.data) { return; }

			data = std::move(ta.data);
			inited = ta.inited;
			return;
		}

	public:
		virtual string getKey() {
			return GetName();
		}
		virtual vector<string> getKeys() { return vector<string>(); }
		virtual vector<Object*> getValue(string key) const { return vector<Object*>(); }
		virtual string getLeaf() const {
			return Get();
		}
		virtual string getLeaf(string leaf) const {
			return string();
		}
		virtual vector<Object*> getLeaves() {
			return vector<Object*>();
		}
		virtual void keyCount() { }
		virtual void keyCount(map<string, int>& counter) { }
		virtual string getToken(int index) { return string(); }
		virtual vector<string> getTokens() { return vector<string>(); }
		virtual int numTokens() { return 0; }
		virtual bool isLeaf() const { return true; }
		virtual double safeGetFloat(string k, double def = 0) { return 0.0; }
		virtual string safeGetString(string k, string def = "") { return string(); }
		virtual int safeGetInt(string k, int def = 0) { return 0; }
		virtual Object* safeGetObject(string k, Object* def = 0) { return nullptr; }
		virtual string toString() const {
			if (GetName().empty()) {
				return Get();
			}
			return GetName() + " = " + Get();
		}
	};

	class UserType : public Type { // chk parent? - in move constructor?
	public:
		UserType(const std::string& name = "") : Type(name) { }
		UserType(UserType&& other) noexcept : Type(other.GetName()) {
			this->objectList = std::move(other.objectList);
			isObjectList = std::move(other.isObjectList);

			for (int i = 0; i < objectList.size(); ++i) {
				if (!objectList[i]->isLeaf()) {
					((UserType*)objectList[i])->parent = this;
				}
			}
		}
		UserType(const UserType& other) : Type(other) {
			if (other.objectList.empty()) { return; }

			objectList = std::vector<Object*>(other.objectList.size());
			isObjectList = other.isObjectList;

			for (int i = 0; i < objectList.size(); ++i) {
				if (other.objectList[i]->isLeaf()) {
					objectList[i] = (Object*)new ItemType(*((ItemType<std::string>*)other.objectList[i]));
				}
				else {
					objectList[i] = (Object*)new UserType(*((UserType*)other.objectList[i]));
					((UserType*)objectList[i])->parent = this;
				}
			}
		}
		virtual ~UserType() {
			Remove();
		}
		void Remove() {
			for (int i = 0; i < objectList.size(); ++i) {
				if (objectList[i]) {
					delete objectList[i];
				}
			}
			objectList.clear();
		}
		void LinkUserType(UserType* ut) // friend?
		{
			objectList.push_back(ut);
			ut->parent = this;
		}

	private:
		std::vector<Object*> objectList;
		UserType* parent = nullptr;
		bool isObjectList = false;
	public:
		void operator=(UserType&& ut) noexcept {
			std::swap(objectList, ut.objectList);
			std::swap(isObjectList, ut.isObjectList);

			for (int i = 0; i < objectList.size(); ++i) {
				if (!objectList[i]->isLeaf()) {
					((UserType*)objectList[i])->parent = this;
				}
			}
			for (int i = 0; i < ut.objectList.size(); ++i) {
				if (!objectList[i]->isLeaf()) {
					((UserType*)ut.objectList[i])->parent = &ut;
				}
			}
		}
		UserType& operator=(const UserType& ut) {
			objectList = ut.objectList;
			isObjectList = ut.isObjectList;

			for (int i = 0; i < objectList.size(); ++i) {
				if (!objectList[i]->isLeaf()) {
					((UserType*)objectList[i])->parent = this;
				}
			}

			return *this;
		}

		Object*& GetList(int idx) {
			return objectList[idx];
		}
		const Object* const & GetList(int idx) const {
			return objectList[idx];
		}

		UserType* GetParent() {
			return parent;
		}

		int GetListSize() const {
			return objectList.size();
		}
		void ReserveList(int x) {
			objectList.reserve(x);
		}
		void AddItemType(const std::string& name, const std::string& value) {
			if (name.empty()) {
				this->isObjectList = true;
			}
			objectList.push_back((Object*)new ItemType(name, value));
		}
		void AddItemType(ItemType<std::string>* it) {
			if (it->GetName().empty()) {
				this->isObjectList = true;
			}
			objectList.push_back(it);
		}
		void AddItemType(const ItemType<std::string>& it) {
			if (it.GetName().empty()) {
				this->isObjectList = true;
			}
			objectList.push_back((Object*)new ItemType(it));
		}
		void AddItemType(ItemType<std::string>&& it) {
			if (it.GetName().empty()) {
				this->isObjectList = true;
			}
			objectList.push_back((Object*)new ItemType(std::move(it)));
		}
		void AddUserTypeItem(const UserType& ut) {
			objectList.push_back((Object*)new UserType(ut));
			((UserType*)objectList.back())->parent = this;
		}
		void AddUserTypeItem(UserType&& ut) {
			objectList.push_back((Object*)new UserType(std::move(ut)));
			((UserType*)objectList.back())->parent = this;
		}
		void GetLastUserTypeItemRef(UserType*& ref) {
			ref = ((UserType*)objectList.back());
		}

	public:
		virtual string getKey() {
			return GetName();
		}
		virtual vector<string> getKeys() {
			vector<string> ret;	// the keys to return
			for (vector<Object*>::iterator i = objectList.begin(); i != objectList.end(); ++i)
			{
				string curr = (*i)->getKey();	// the current key
				if (find(ret.begin(), ret.end(), curr) != ret.end())
				{
					continue;
				}
				ret.push_back(curr);
			}
			return ret;
		}

		virtual vector<Object*> getValue(string key) const {
			vector<Object*> ret;	// the objects to return
			for (vector<Object*>::const_iterator i = objectList.begin(); i != objectList.end(); ++i)
			{
				if ((*i)->getKey() != key)
				{
					continue;
				}
				ret.push_back(*i);
			}
			return ret;
		}
		virtual string getLeaf(string leaf) const {
			vector<Object*> leaves = getValue(leaf); // the objects to return
			if (0 == leaves.size())
			{
				cout << "Error: Cannot find leaf " << leaf << " in object " << endl << this->GetName();
				assert(leaves.size());
			}
			return leaves[0]->getLeaf();
		}
		virtual string getLeaf() const {
			return string();
		}
		virtual vector<Object*> getLeaves() {
			return objectList;
		}
		virtual void keyCount() {
			if (isLeaf())
			{
				cout << this->GetName() << " : 1\n";
				return;
			}

			map<string, int> refCount;	// the count of the references
			keyCount(refCount);
			vector<pair<string, int> > sortedCount; // an organized container for the counts
			for (map<string, int>::iterator i = refCount.begin(); i != refCount.end(); ++i)
			{
				pair<string, int> curr((*i).first, (*i).second);
				if (2 > curr.second)
				{
					continue;
				}
				if ((0 == sortedCount.size()) || (curr.second <= sortedCount.back().second))
				{
					sortedCount.push_back(curr);
					continue;
				}

				for (vector<pair<string, int> >::iterator j = sortedCount.begin(); j != sortedCount.end(); ++j)
				{
					if (curr.second < (*j).second)
					{
						continue;
					}
					sortedCount.insert(j, 1, curr);
					break;
				}
			}

			for (vector<pair<string, int> >::iterator j = sortedCount.begin(); j != sortedCount.end(); ++j)
			{
				cout << (*j).first << " : " << (*j).second << "\n";
			}
		}

		virtual void keyCount(map<string, int>& counter) {
			for (vector<Object*>::iterator i = objectList.begin(); i != objectList.end(); ++i)
			{
				counter[((Type*)(*i))->GetName()]++;
				if ((*i)->isLeaf())
				{
					continue;
				}
				(*i)->keyCount(counter);
			}
		}
		virtual string getToken(int index) {
			if (!isObjectList)
			{
				return "";
			}
			if (index >= (int)objectList.size())
			{
				return "";
			}
			if (index < 0)
			{
				return "";
			}
			return ((ItemType<std::string>*)objectList[index])->Get();
		}
		virtual vector<string> getTokens() { 
			if (!this->isObjectList) {
				return vector<string>();
			}

			vector<string> temp(objectList.size());

			for (int i = 0; i < temp.size(); ++i) {
				temp[i] = ((ItemType<std::string>*)objectList[i])->Get();
			}

			return temp;
		}
		virtual int numTokens() {
			if (!this->isObjectList) {
				return 0;
			}
			return objectList.size();
		}
		virtual bool isLeaf() const { return false; }
		virtual double safeGetFloat(string k, double def = 0) {
			objvec vec = getValue(k);	// the objects with the keys to be returned
			if (0 == vec.size()) return def;
			return atof(vec[0]->getLeaf().c_str());
		}
		virtual string safeGetString(string k, string def = "") {
			objvec vec = getValue(k);	// the objects with the strings to be returned
			if (0 == vec.size())
			{
				return def;
			}
			return vec[0]->getLeaf();
		}
		virtual int safeGetInt(string k, int def = 0) {
			objvec vec = getValue(k);	// the objects with the ints to be returned
			if (0 == vec.size())
			{
				return def;
			}
			return atoi(vec[0]->getLeaf().c_str());
		}
		virtual Object* safeGetObject(string k, Object* def = 0) {
			objvec vec = getValue(k);	// the objects with the objects to be returned 
			if (0 == vec.size())
			{
				return def;
			}
			return vec[0];
		}
		virtual string toString() const {
			return "";
		}
	};

	// LoadData
	class LoadData
	{
		enum {
			TYPE_LEFT = 1, // 01
			TYPE_RIGHT = 2, // 10
			TYPE_ASSIGN = 3 // 11
		};
	private:
		static long long check_syntax_error1(long long str, int* err) {
			long long len = GetLength(str);
			long long type = GetType(str);

			if (1 == len && (type == TYPE_LEFT || type == TYPE_RIGHT ||
				type == TYPE_ASSIGN)) {
				*err = -4;
			}

			return str;
		}
	public:
		static int Merge(UserType* next, UserType* ut, UserType** ut_next)
		{
			//check!!
			while (ut->GetListSize() >= 1 && !ut->GetList(0)->isLeaf()
				&& (((Type*)(ut->GetList(0)))->GetName() == "#"))
			{
				ut = (UserType*)ut->GetList(0);
			}

			bool chk_ut_next = false;

			while (true) {
				UserType* _ut = ut;
				UserType* _next = next;


				if (ut_next && _ut == *ut_next) {
					*ut_next = _next;
					chk_ut_next = true;
				}

				for (int i = 0; i < _ut->GetListSize(); ++i) {
					if (!_ut->GetList(i)->isLeaf()) {
						if (((UserType*)(_ut->GetList(i)))->GetName() == "#") {
							((Type*)_ut->GetList(i))->SetName("");
						}
						else {
							{
								_next->LinkUserType((UserType*)_ut->GetList(i));
								_ut->GetList(i) = nullptr;
							}
						}
					}
					else {
						_next->AddItemType((ItemType<std::string>*)_ut->GetList(i));
						_ut->GetList(i) = nullptr;
					}
				}
				_ut->Remove();

				ut = ut->GetParent();
				next = next->GetParent();


				if (next && ut) {
					//
				}
				else {
					// right_depth > left_depth
					if (!next && ut) {
						return -1;
					}
					else if (next && !ut) {
						return 1;
					}

					return 0;
				}
			}
		}
	private:
		static long long GetIdx(long long x) {
			return (x >> 32) & 0x00000000FFFFFFFF;
		}
		static long long GetLength(long long x) {
			return (x & 0x00000000FFFFFFF8) >> 3;
		}
		static long long GetType(long long x) { //to enum or enum class?
			return (x & 6) >> 1;
		}
	private:
		static bool __LoadData(const char* buffer, const long long* token_arr, long long token_arr_len, UserType* _global, const wiz::LoadDataOption* _option,
			int start_state, int last_state, UserType** next, int* err)
		{
			std::vector<long long> varVec;
			std::vector<long long> valVec;


			if (token_arr_len <= 0) {
				return false;
			}

			UserType& global = *_global;
			wiz::LoadDataOption option = *_option;

			int state = start_state;
			int braceNum = 0;
			std::vector< UserType* > nestedUT(1);
			long long var = 0, val = 0;

			nestedUT.reserve(10);
			nestedUT[0] = &global;


			long long count = 0;
			const long long* x = token_arr;
			const long long* x_next = x;

			for (long long i = 0; i < token_arr_len; ++i) {
				x = x_next;
				{
					x_next = x + 1;
				}
				if (count > 0) {
					count--;
					continue;
				}
				long long len = GetLength(token_arr[i]);

				switch (state)
				{
				case 0:
				{
					// Left 1
					if (len == 1 && (-1 != Equal(TYPE_LEFT, GetType(token_arr[i])))) {
						if (!varVec.empty()) {
							nestedUT[braceNum]->ReserveList(nestedUT[braceNum]->GetListSize() + varVec.size());
							
							for (long long x = 0; x < varVec.size(); ++x) {
								nestedUT[braceNum]->AddItemType(std::string(buffer + GetIdx(varVec[x]), GetLength(varVec[x])),
									std::string(buffer + GetIdx(valVec[x]), GetLength(valVec[x])));
							}

							varVec.clear();
							valVec.clear();
						}

						UserType temp("");

						nestedUT[braceNum]->AddUserTypeItem(temp);
						UserType* pTemp = nullptr;
						nestedUT[braceNum]->GetLastUserTypeItemRef(pTemp);

						braceNum++;

						/// new nestedUT
						if (nestedUT.size() == braceNum) { /// changed 2014.01.23..
							nestedUT.push_back(nullptr);
						}

						/// initial new nestedUT.
						nestedUT[braceNum] = pTemp;
						///

						state = 0;
					}
					// Right 2
					else if (len == 1 && (-1 != Equal(TYPE_RIGHT, GetType(token_arr[i])))) {
						state = 0;

						if (!varVec.empty()) {

							{
								nestedUT[braceNum]->ReserveList(nestedUT[braceNum]->GetListSize() + varVec.size());
								
								for (long long x = 0; x < varVec.size(); ++x) {
									nestedUT[braceNum]->AddItemType(std::string(buffer + GetIdx(varVec[x]), GetLength(varVec[x])),
										std::string(buffer + GetIdx(valVec[x]), GetLength(valVec[x])));
								}
							}

							varVec.clear();
							valVec.clear();
						}

						if (braceNum == 0) {
							UserType ut;
							ut.AddUserTypeItem(UserType("#")); // json -> "var_name" = val  // clautext, # is line comment delimiter.
							UserType* pTemp = nullptr;
							ut.GetLastUserTypeItemRef(pTemp);
							
							auto max = nestedUT[braceNum]->GetListSize();
							for (auto i = 0; i < max; ++i) {
								if (!nestedUT[braceNum]->GetList(i)->isLeaf()) {
									((UserType*)ut.GetList(0))->AddUserTypeItem(std::move(*((UserType*)nestedUT[braceNum]->GetList(i))));
								}
								else {
									((UserType*)ut.GetList(0))->AddItemType(std::move(*((ItemType<std::string>*)(nestedUT[braceNum]->GetList(i)))));
								}
							}

							nestedUT[braceNum]->Remove();
							nestedUT[braceNum]->AddUserTypeItem(std::move(*((UserType*)ut.GetList(0))));

							braceNum++;
						}

						{
							if (braceNum < nestedUT.size()) {
								nestedUT[braceNum] = nullptr;
							}
							braceNum--;
						}
					}
					else {
						if (x < token_arr + token_arr_len - 1) {
							long long _len = GetLength(token_arr[i + 1]);
							// EQ 3
							if (_len == 1 && -1 != Equal(TYPE_ASSIGN, GetType(token_arr[i + 1]))) {
								var = token_arr[i];

								state = 1;

								{
									count = 1;
								}
							}
							else {
								// var1
								if (x <= token_arr + token_arr_len - 1) {

									val = token_arr[i];

									varVec.push_back(check_syntax_error1(var, err));
									valVec.push_back(check_syntax_error1(val, err));

									val = 0;

									state = 0;

								}
							}
						}
						else
						{
							// var1
							if (x <= token_arr + token_arr_len - 1)
							{
								val = token_arr[i];
								varVec.push_back(check_syntax_error1(var, err));
								valVec.push_back(check_syntax_error1(val, err));
								val = 0;

								state = 0;
							}
						}
					}
				}
				break;
				case 1:
				{
					// LEFT 1
					if (len == 1 && (-1 != Equal(TYPE_LEFT, GetType(token_arr[i])))) {
						nestedUT[braceNum]->ReserveList(nestedUT[braceNum]->GetListSize() + varVec.size());
						
						for (long long x = 0; x < varVec.size(); ++x) {
							nestedUT[braceNum]->AddItemType(std::string(buffer + GetIdx(varVec[x]), GetLength(varVec[x])),
								std::string(buffer + GetIdx(valVec[x]), GetLength(valVec[x])));
						}


						varVec.clear();
						valVec.clear();

						///
						{
							nestedUT[braceNum]->AddUserTypeItem(UserType(std::string(buffer + GetIdx(var), GetLength(var))));
							UserType* pTemp = nullptr;
							nestedUT[braceNum]->GetLastUserTypeItemRef(pTemp);
							var = 0;
							braceNum++;

							/// new nestedUT
							if (nestedUT.size() == braceNum) {
								nestedUT.push_back(nullptr);
							}

							/// initial new nestedUT.
							nestedUT[braceNum] = pTemp;
						}
						///
						state = 0;
					}
					else {
						if (x <= token_arr + token_arr_len - 1) {
							val = token_arr[i];

							varVec.push_back(check_syntax_error1(var, err));
							valVec.push_back(check_syntax_error1(val, err));
							var = 0; val = 0;

							state = 0;
						}
					}
				}
				break;
				default:
					// syntax err!!
					*err = -1;
					return false; // throw "syntax error ";
					break;
				}
			}

			if (next) {
				*next = nestedUT[braceNum];
			}

			if (varVec.empty() == false) {
				nestedUT[braceNum]->ReserveList(nestedUT[braceNum]->GetListSize() + varVec.size());

				for (long long x = 0; x < varVec.size(); ++x) {
					nestedUT[braceNum]->AddItemType(std::string(buffer + GetIdx(varVec[x]), GetLength(varVec[x])),
						std::string(buffer + GetIdx(valVec[x]), GetLength(valVec[x])));
				}


				varVec.clear();
				valVec.clear();
			}

			if (state != last_state) {
				*err = -2;
				return false;
				// throw std::string("error final state is not last_state!  : ") + toStr(state);
			}
			if (x > token_arr + token_arr_len) {
				*err = -3;
				return false;
				//throw std::string("error x > buffer + buffer_len: ");
			}

			return true;
		}


		static long long FindDivisionPlace(const char* buffer, const long long* token_arr, long long start, long long last, const wiz::LoadDataOption& option)
		{
			for (long long a = last; a >= start; --a) {
				long long len = GetLength(token_arr[a]);
				long long val = GetType(token_arr[a]);


				if (len == 1 && (-1 != Equal(TYPE_RIGHT, val))) { // right
					return a;
				}

				bool pass = false;
				if (len == 1 && (-1 != Equal(TYPE_LEFT, val))) { // left
					return a;
				}
				else if (len == 1 && -1 != Equal(TYPE_ASSIGN, val)) { // assignment
					//
					pass = true;
				}

				if (a < last && pass == false) {
					long long len = GetLength(token_arr[a + 1]);
					long long val = GetType(token_arr[a + 1]);

					if (!(len == 1 && -1 != Equal(TYPE_ASSIGN, val))) // assignment
					{ // NOT
						return a;
					}
				}
			}
			return -1;
		}

		static bool _LoadData(InFileReserver& reserver, UserType& global, wiz::LoadDataOption option, const int lex_thr_num, const int parse_num) // first, strVec.empty() must be true!!
		{
			const int pivot_num = parse_num - 1;
			char* buffer = nullptr;
			long long* token_arr = nullptr;
			long long buffer_total_len;
			long long token_arr_len = 0;

			{
				int a = clock();

				bool success = reserver(option, lex_thr_num, buffer, &buffer_total_len, token_arr, &token_arr_len);


				int b = clock();
			//	std::cout << b - a << "ms\n";

				//	{
				//		for (long long i = 0; i < token_arr_len; ++i) {
				//			std::string(buffer + GetIdx(token_arr[i]), GetLength(token_arr[i]));
			//				if (0 == GetIdx(token_arr[i])) {
				//				std::cout << "chk";
				//			}
				//		}
				//	}

				if (!success) {
					return false;
				}
				if (token_arr_len <= 0) {
					if (buffer) {
						delete[] buffer;
					}
					if (token_arr) {
						delete[] token_arr;
					}
					return true;
				}
			}

			UserType* before_next = nullptr;
			UserType _global;

			bool first = true;
			long long sum = 0;

			long long count = 0;
			for (long long i = 0; i < token_arr_len - 1; ++i) {
				if (GetType(token_arr[i]) == TYPE_LEFT) {
					count++;
				}
				else if (GetType(token_arr[i]) == TYPE_RIGHT) {
					count--;
				}
			}

			bool do_except_last = false;
			if (count == 0 && GetType(token_arr[token_arr_len - 1]) == TYPE_RIGHT) {
				do_except_last = true;
			}

			if (GetType(token_arr[token_arr_len - 1]) == TYPE_LEFT) {
				count++;
			}
			else if (GetType(token_arr[token_arr_len - 1]) == TYPE_RIGHT) {
				count--;
			}

			if (!do_except_last && count != 0) {
				std::cout << "\n\n parsing error \n\n";
				delete[] buffer;
				delete[] token_arr;
				return false;
			}
			if (do_except_last) {
				token_arr_len--;
			}

			if (token_arr_len < 10000) {
				try {
					UserType* next = nullptr;
					int err = 0;

					__LoadData(buffer, token_arr, token_arr_len, &_global, &option, 0, 0, &next, &err);
					
					{
						switch (err) {
						case 0:
							break;
						case -1:
						case -4:
							std::cout << "Syntax Error\n";
							break;
						case -2:
							std::cout << "error final state is not last_state!\n";
							break;
						case -3:
							std::cout << "error x > buffer + buffer_len:\n";
							break;
						default:
							std::cout << "unknown parser error\n";
							break;
						}
					}

				}
				catch (...) {
					//
				}

				delete[] buffer;
				delete[] token_arr;

				global = std::move(_global);

				return true;
			}

			{
				std::set<long long> _pivots;
				std::vector<long long> pivots;
				const long long num = token_arr_len; //

				if (pivot_num > 0) {
					std::vector<long long> pivot;
					pivots.reserve(pivot_num);
					pivot.reserve(pivot_num);

					for (int i = 0; i < pivot_num; ++i) {
						pivot.push_back(FindDivisionPlace(buffer, token_arr, (num / (pivot_num + 1)) * (i), (num / (pivot_num + 1)) * (i + 1) - 1, option));
					}

					for (int i = 0; i < pivot.size(); ++i) {
						if (pivot[i] != -1) {
							_pivots.insert(pivot[i]);
						}
					}

					for (auto& x : _pivots) {
						pivots.push_back(x);
					}
				}

				std::vector<UserType*> next(pivots.size() + 1, nullptr);

				{
					std::vector<UserType> __global(pivots.size() + 1);

					std::vector<std::thread> thr(pivots.size() + 1);
					std::vector<int> err(pivots.size() + 1, 0);
					{
						long long idx = pivots.empty() ? num - 1 : pivots[0];
						long long _token_arr_len = idx - 0 + 1;

						thr[0] = std::thread(__LoadData, buffer, token_arr, _token_arr_len, &__global[0], &option, 0, 0, &next[0], &err[0]);
					}

					for (int i = 1; i < pivots.size(); ++i) {
						long long _token_arr_len = pivots[i] - (pivots[i - 1] + 1) + 1;

						thr[i] = std::thread(__LoadData, buffer, token_arr + pivots[i - 1] + 1, _token_arr_len, &__global[i], &option, 0, 0, &next[i], &err[i]);

					}

					if (pivots.size() >= 1) {
						long long _token_arr_len = num - 1 - (pivots.back() + 1) + 1;

						thr[pivots.size()] = std::thread(__LoadData, buffer, token_arr + pivots.back() + 1, _token_arr_len, &__global[pivots.size()],
							&option, 0, 0, &next[pivots.size()], &err[pivots.size()]);
					}

					// wait
					for (int i = 0; i < thr.size(); ++i) {
						thr[i].join();
					}

					for (int i = 0; i < err.size(); ++i) {
						switch (err[i]) {
						case 0:
							break;
						case -1:
						case -4:
							std::cout << "Syntax Error\n";
							break;
						case -2:
							std::cout << "error final state is not last_state!\n";
							break;
						case -3:
							std::cout << "error x > buffer + buffer_len:\n";
							break;
						default:
							std::cout << "unknown parser error\n";
							break;
						}
					}

					// Merge
					try {
						if (__global[0].GetListSize() > 0 && ((Type*)__global[0].GetList(0))->GetName() == "#") {
							std::cout << "not valid file1\n";
							throw 1;
						}
						if (next.back()->GetParent() != nullptr) {
							std::cout << "not valid file2\n";
							throw 2;
						}

						int err = Merge(&_global, &__global[0], &next[0]);
						if (-1 == err || (pivots.size() == 0 && 1 == err)) {
							std::cout << "not valid file3\n";
							throw 3;
						}

						for (int i = 1; i < pivots.size() + 1; ++i) {
							// linearly merge and error check...
							int err = Merge(next[i - 1], &__global[i], &next[i]);
							if (-1 == err) {
								std::cout << "not valid file4\n";
								throw 4;
							}
							else if (i == pivots.size() && 1 == err) {
								std::cout << "not valid file5\n";
								throw 5;
							}
						}
					}
					catch (...) {
						delete[] buffer;
						delete[] token_arr;
						buffer = nullptr;
						throw "in Merge, error";
					}

					before_next = next.back();
				}
			}

			delete[] buffer;
			delete[] token_arr;

			global = std::move(_global);

			return true;
		}
	public:
		static bool LoadDataFromFile(const std::string& fileName, UserType& global, int lex_thr_num, int parse_thr_num) /// global should be empty
		{
			if (lex_thr_num <= 0) {
				lex_thr_num = std::thread::hardware_concurrency();
			}
			if (lex_thr_num <= 0) {
				lex_thr_num = 1;
			}

			if (parse_thr_num <= 0) {
				parse_thr_num = std::thread::hardware_concurrency();
			}
			if (parse_thr_num <= 0) {
				parse_thr_num = 1;
			}

			bool success = true;
			std::ifstream inFile;
			inFile.open(fileName, std::ios::binary);


			if (true == inFile.fail())
			{
				inFile.close(); return false;
			}

			UserType globalTemp;

			try {

				InFileReserver ifReserver(inFile);
				wiz::LoadDataOption option;
				option.Assignment = ('=');
				option.Left = '{';
				option.Right = '}';
				option.LineComment = ('#');
				option.Removal = ' '; // ','

				char* buffer = nullptr;
				ifReserver.Num = 1 << 19;
				//	strVec.reserve(ifReserver.Num);
				// cf) empty file..
				if (false == _LoadData(ifReserver, globalTemp, option, lex_thr_num, parse_thr_num))
				{
					inFile.close();
					return false; // return true?
				}

				inFile.close();
			}
			catch (const char* err) { std::cout << err << "\n"; inFile.close(); return false; }
			catch (const std::string& e) { std::cout << e << "\n"; inFile.close(); return false; }
			catch (const std::exception& e) { std::cout << e.what() << "\n"; inFile.close(); return false; }
			catch (...) { std::cout << "not expected error" << "\n"; inFile.close(); return false; }


			global = std::move(globalTemp);

			return true;
		}
		static bool LoadWizDB(UserType& global, const std::string& fileName, const int thr_num) {
			UserType globalTemp = UserType("global");

			// Scan + Parse 
			if (false == LoadDataFromFile(fileName, globalTemp, thr_num, thr_num)) { return false; }
			//std::cout << "LoadData End" << "\n";

			global = std::move(globalTemp);
			return true;
		}
		// SaveQuery
		static bool SaveWizDB(const UserType& global, const std::string& fileName, const bool append = false) {
			std::ofstream outFile;
			if (fileName.empty()) { return false; }
			if (false == append) {
				outFile.open(fileName);
				if (outFile.fail()) { return false; }
			}
			else {
				outFile.open(fileName, std::ios::app);
				if (outFile.fail()) { return false; }

				outFile << "\n";
			}

			/// saveFile
			///global.Save1(outFile); // cf) friend

			outFile.close();

			return true;
		}
	};

}


ostream& operator<< (ostream& os, const Object& obj);


#endif
