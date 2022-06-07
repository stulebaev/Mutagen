#pragma once

#include "CommonLib/Typedefs.h"

#include <list>
#include <vector>
#include <sstream>

namespace FileIO
{ //namespace FileIO

	class CFileIOBase
	{
		//CFileIOBase()
		//{
		//}
		//virtual ~CFileIOBase()
		//{
		//}
	public:
		//enum EFileTypes { eFT_None=0,eFT_FileIOBase=1 };

		//virtual EFileTypes GetFileType() const;

		//Function takes filename and reads its contents to bitwise buffer pBytes_ containing nCharacters of characters
		static bool ReadFileCharacters(const Common::ComString& strFileName, size_t& nCharacters, Common::ComChar* &pCharacters);
		static bool ReadFileLines(const Common::ComString& strFileName, std::list<Common::ComString>& listLines);
		//template <typename _StringContainer>
		//static bool ReadFileLines(const Common::ComString& strFileName, _StringContainer& contLines);
		static bool ReadFileStream(const Common::ComString& strFileName, std::basic_stringstream<Common::ComChar>& ss);
		static bool GetFileIStream(const Common::ComString& strFileName, std::basic_ifstream<Common::ComChar>& is, bool bBinaryMode = false);

		static bool WriteFileCharacters(const Common::ComString& strFileName, size_t nCharacters, const Common::ComChar* pCharacters, bool bAppend = false);
		static bool WriteFileLines(const Common::ComString& strFileName, const std::list<Common::ComString>& listLines, bool bAppend = false);
		static bool WriteFileStream(const Common::ComString& strFileName, const std::basic_stringstream<Common::ComChar>& ss, bool bAppend = false);
		static bool GetFileOStream(const Common::ComString& strFileName, std::basic_ofstream<Common::ComChar>& os, bool bBinaryMode = false, bool bAppend = false);

	protected:
		static Common::ComString adjustFileName(const Common::ComString& strFileName, bool bCreateDir = false);
		static Common::ComString adjustFileName(const Common::ComString& strPath, const Common::ComString& strFileName, bool bCreateDir = false);
	};

	////////////////////////////////////////////////////////////////////////////////

	//Auxiliary functions

	inline Common::ComString GetNameFromLine(const Common::ComString& strSource)
	{
		// If name is in quotes, retrieve it 'as is', i.e. with all leading and tailing spaces, tabs etc. lying within [either outer or successive] pair of quotes.
		// Otherwise, remove leading and tailing spaces, tabs, quotes, caret return.
		Common::ComString strResult;
		size_t firstPos = strSource.find_first_of(_ComStr("\""));
#if 1
		size_t lastPos = (firstPos == Common::ComString::npos) ? Common::ComString::npos : strSource.find_first_of(_ComStr("\""), firstPos + 1); // Look for second quote symbol (if any)
#else
		size_t lastPos = strSource.find_last_of(_ComStr("\"")); // Look for last quote symbol
#endif
		if ( firstPos != Common::ComString::npos // [first] quote presents
			&& lastPos != Common::ComString::npos // [last] quote presents
			&& firstPos < lastPos // [fitst] and [last] quote symbols are differnet, i.e. there are not less than two quote symbols
			) // Pair of quotes found
		{
			strResult = strSource.substr(firstPos + 1, lastPos - firstPos - 1);
		}
		else // Pair of quotes not found
		{
			// Remove leading and tailing 'special symbols'
			firstPos = strSource.find_first_not_of(_ComStr(" \t\"")); // Counts spaces and tabs after the quote symbol(s) too!
			lastPos = strSource.find_last_not_of(_ComStr(" \t\"\r")); // Counts spaces and tabs before the quote symbol(s) too!
			strResult = (firstPos == Common::ComString::npos) // strSource does not contain any meaningful symbols
				? _ComStr("")
				: strSource.substr(firstPos, lastPos - firstPos + 1);
		}
		return strResult;
	}

	template <typename _Stream>
	inline Common::ComString GetNameFromRestOfLine(_Stream& is)
	{
		Common::ComString strBuffer;
		std::getline(is, strBuffer); //Read the 'raw' string
		return GetNameFromLine(strBuffer);
	}

	inline Common::ComString ExtractLine(std::basic_istream<Common::ComChar>& is, std::basic_ios<Common::ComChar>::pos_type& pos)
	{
		Common::ComString str;
		std::getline(is, str);
		pos = is.tellg();
		return str;
	}
	inline void SetPos(std::basic_istream<Common::ComChar>& is, std::basic_ios<Common::ComChar>::pos_type& pos)
	{
		is.seekg(pos);
	}
	inline bool IsEOF(const std::basic_istream<Common::ComChar>& is, const std::basic_ios<Common::ComChar>::pos_type& /*pos*/)
	{
		return is.eof();
	}

	///////

	inline Common::ComString ExtractLine(std::list<Common::ComString>& /*listString*/, std::list<Common::ComString>::iterator& itList)
	{
		return *itList++;
	}
	inline void SetPos(std::list<Common::ComString>& /*listString*/, std::list<Common::ComString>::iterator& /*itList*/)
	{

	}
	inline bool IsEOF(const std::list<Common::ComString>& listString, const std::list<Common::ComString>::iterator& itList)
	{
		return itList == listString.end();
	}

	///////

	inline Common::ComString ExtractLine(std::vector<Common::ComString>& /*vecString*/, std::vector<Common::ComString>::iterator& itVec)
	{
		return *itVec++;
	}
	inline void SetPos(std::vector<Common::ComString>& /*vecString*/, std::vector<Common::ComString>::iterator& /*itVec*/)
	{

	}
	inline bool IsEOF(const std::vector<Common::ComString>& vecString, const std::vector<Common::ComString>::iterator& itVec)
	{
		return itVec == vecString.end();
	}

} //namespaceIO
