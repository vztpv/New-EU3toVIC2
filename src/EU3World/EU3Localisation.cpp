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



#include "EU3Localisation.h"
#include <fstream>
#include <vector>
#include <boost/tokenizer.hpp>
#include <iterator>
using namespace std;
using namespace boost;

#include <filesystem>
namespace fs = std::filesystem;
#include <set>



enum
{
	CODE,
	ENGLISH,
	FRENCH,
	GERMAN,
	POLSKI,
	SPANISH,
	ITALIAN,
	SWEDISH,
	CZECH,
	HUNGARIAN,
	DUTCH,
	PORTUGESE,
	RUSSIAN,
	FINNISH,
	x
} languages;


void EU3Localisation::ReadFromFile(const std::string& fileName)
{
	std::ifstream in(fileName);

	const int maxLineLength = 10000;	// the maximum line length
	char lineArray[maxLineLength];			// the line being processed
	string line;

	// Subsequent lines are 'KEY: "Text"'
	while (!in.eof())
	{
		in.getline(lineArray, maxLineLength);
		line = lineArray;
		if (!in.eof())
		{
			string key;
			string currentLocalisation;

			char_separator<char> separator(";");
			tokenizer<char_separator<char>> tokens(line, separator);
			int language = CODE;
			for (auto tItr = tokens.begin(); tItr != tokens.end(); ++tItr)
			{
				if (distance(tokens.begin(), tItr) == 0)
				{
					key = *tItr;
				}
				else
				{
					currentLocalisation = *tItr;
					localisations[key][language] = currentLocalisation;
				}
				language++;
			}
		}
	}
}

void EU3Localisation::ReadFromAllFilesInFolder(const std::string& folderPath)
{
	// Get all files in the folder.
	set<std::string> fileNames;
	if (!fs::exists(fs::u8path(folderPath))) return;
	if (fs::is_empty(fs::u8path(folderPath))) return;
	for (auto& p : fs::directory_iterator(fs::u8path(folderPath)))
	{
		if (!p.is_directory())
		{
			fileNames.insert(p.path().filename().string());
		}

	}
	for (const auto& fileName : fileNames)
	{
		ReadFromFile(folderPath + '\\' + fileName);
	}
}

const std::string& EU3Localisation::GetText(const std::string& key, unsigned int language) const
{
	static const std::string noLocalisation = "";	// used if there's no localisation

	const auto keyFindIter = localisations.find(key);	// the localisations for this key
	if (keyFindIter == localisations.end())
	{
		return noLocalisation;
	}
	const auto& localisationsByLanguage = keyFindIter->second;	// the localisations for this language
	const auto languageFindIter = localisationsByLanguage.find(language);	// the localisation we want
	if (languageFindIter == localisationsByLanguage.end())
	{
		return noLocalisation;
	}

	return languageFindIter->second;
}

const std::map<unsigned int, std::string>& EU3Localisation::GetTextInEachLanguage(const std::string& key) const
{
	static const std::map<unsigned int, std::string> noLocalisation;	// used if there's no localisation

	const auto keyFindIter = localisations.find(key);	// the localisation we want
	if (keyFindIter == localisations.end())
	{
		return noLocalisation;
	}

	return keyFindIter->second;
}

std::string EU3Localisation::DetermineLanguageForFile(const std::string& text)
{
	static const std::string noLanguageIndicated = "";	// used when no language is indicated

	if (text.size() < 2 || text[0] != 'l' || text[1] != '_')
	{	// Not in the desired format - no "l_"
		return noLanguageIndicated;
	}
	size_t beginPos = 2;	// Skip l_ for our language name.
	size_t endPos = text.find(':', beginPos);	// the end of the language name
	if (endPos == std::string::npos)
	{	// Not in the desired format - no ":"
		return noLanguageIndicated;
	}

	return text.substr(beginPos, endPos - beginPos);
}

std::pair<std::string, std::string> EU3Localisation::DetermineKeyLocalisationPair(const std::string& text)
{
	static const std::pair<std::string, std::string> noLocalisationPair;	// used when there's no localisation pair

	size_t keyBeginPos = text.find_first_not_of(' ');	// the first non-space character
	if (keyBeginPos == std::string::npos)
	{
		return noLocalisationPair;
	}
	size_t keyEndPos = text.find_first_of(':', keyBeginPos + 1);	// the end of the key
	size_t quotePos = text.find_first_of('"', keyEndPos);				// the begining of the string literal
	if (quotePos == std::string::npos)
	{
		return noLocalisationPair;
	}
	size_t localisationBeginPos = quotePos + 1;	// where the localisation begins
	size_t localisationEndPos = text.find_first_of('"', localisationBeginPos);	// where the localisation ends
	return std::make_pair(text.substr(keyBeginPos, keyEndPos - keyBeginPos), text.substr(localisationBeginPos, localisationEndPos - localisationBeginPos));
}

std::string EU3Localisation::RemoveUTF8BOM(const std::string& text)
{
	if (text.size() >= 3 && text[0] == '\xEF' && text[1] == '\xBB' && text[2] == '\xBF')
	{
		return text.substr(3);
	}
	else
	{
		return text;
	}
}
