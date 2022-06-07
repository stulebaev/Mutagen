#include "FileIOBase.h"

#include <fstream>

//	template <class T> bool ReadData(const T* szFileName_, size_t& nBytes_, void* &pBytes_);
//	template <class T1, class T2> bool ReadData(const T1* szFileName_, std::basic_stringstream<T2>& is_);
//	template <typename T1, typename T2> bool ReadData(const T1* szFileName_, std::vector<std::basic_string<_T2> >& vLines_);

//	template <typename T> bool WriteDataBytes(const T* szFileName, size_t& nBytes_, void* &pBytes_);

namespace FileIO
{
	//static
	Common::ComString CFileIOBase::adjustFileName(const Common::ComString& strFileName, bool bCreateDir /*=false*/)
	{
		//Function makes full path/file from relative path/file
		Common::ComString str = strFileName;
		if (bCreateDir)
		{
			//TODO. Stub - not implemented yet
		}
		return str;
	}

	//static
	Common::ComString CFileIOBase::adjustFileName(const Common::ComString& strPath, const Common::ComString& strFileName, bool /*bCreateDir = false*/)
	{
		//Function makes full path from [relative] path and filename
		Common::ComString strBuffer = strPath;
		if (!strBuffer.empty()
			&& strBuffer[strBuffer.length()-1] != _ComStr('/')
			&& strBuffer[strBuffer.length()-1] != _ComStr('\\')
			)
		{
#if defined( __GNUC__ ) || defined (__linux__)
				strBuffer += _ComStr('/');
#else
				strBuffer += _ComStr('\\');
#endif
		}
		strBuffer += strFileName;

		return adjustFileName(strBuffer.c_str());
	}

	//static
	bool CFileIOBase::ReadFileCharacters(const Common::ComString& strFileName, size_t& nCharacters, Common::ComChar* &pCharacters)
	{
		Common::ComString strNewFileName = adjustFileName(strFileName);
		std::basic_ifstream<Common::ComChar> is(strNewFileName.c_str(), std::ios_base::in | std::ios_base::binary);
		if (!is.is_open())
		{
			return false;
		}
		is.seekg(0, std::ios::end);
		size_t nBytes = is.tellg();
		nCharacters = nBytes / sizeof(Common::ComChar);
		pCharacters = new Common::ComChar[nCharacters + 1];
		if (pCharacters) //Succeeded to allocate required buffer
		{
			is.seekg(0, std::ios::beg);
			is.read((Common::ComChar*)pCharacters, nCharacters);
			pCharacters[nCharacters] = _ComStr('\0');
			return true;
		}
		else //Failed to allocate required buffer
		{
			nCharacters = 0;
			return false;
		}
	}

	//static
	bool CFileIOBase::ReadFileLines(const Common::ComString& strFileName, std::list<Common::ComString>& listLines)
	{
		size_t len;
		Common::ComString strNewFileName = adjustFileName(strFileName);
		std::basic_ifstream<Common::ComChar> is(strNewFileName.c_str(), std::ios_base::in | std::ios_base::binary);
		if (!is.is_open())
		{
			return false;
		}
		while (!is.eof())
		{
			listLines.push_back(Common::ComString());
			Common::ComString& strLine = listLines.back();
			std::getline(is, strLine);
			if ( (len = strLine.length()) //if not empty
				&& strLine.at(len-1) == _ComStr('\r')
				)
			{
				strLine = strLine.substr(0, len-1);
			}
		}
		return true;
	}

	//static
	bool CFileIOBase::ReadFileStream(const Common::ComString& strFileName, std::basic_stringstream<Common::ComChar>& ss)
	{
		Common::ComString strNewFileName = adjustFileName(strFileName);
		std::basic_ifstream<Common::ComChar> is(strNewFileName.c_str(), std::ios_base::in | std::ios_base::binary);
		if (!is.is_open())
		{
			return false;
		}
		is >> ss.rdbuf();
		ss << _ComStr('\0');
		return true;
	}

	//static 
	bool CFileIOBase::GetFileIStream(const Common::ComString& strFileName, std::basic_ifstream<Common::ComChar>& is, bool bBinaryMode)
	{
		Common::ComString strNewFileName = adjustFileName(strFileName);
		std::ios::openmode mode = std::ios_base::in
			| (bBinaryMode ? std::ios_base::binary : (std::ios::openmode)0);
		is.open(strNewFileName.c_str(), mode);
		bool bRes = is.is_open();
		return bRes;
	}

	/////

		//static
	bool CFileIOBase::WriteFileCharacters(const Common::ComString& strFileName, size_t nCharacters, const Common::ComChar* pCharacters, bool bAppend)
	{
		Common::ComString strNewFileName = adjustFileName(strFileName, true);
		std::basic_ofstream<Common::ComChar> os(strNewFileName.c_str(),
												std::ios_base::out
												| std::ios_base::binary
												// | (bAppend ? std::ios::app : (std::ios::openmode)0)
												| (bAppend ? std::ios::ate : (std::ios::openmode)0)
												);
		if (!os.is_open())
		{
			return false;
		}
		os.write(pCharacters, nCharacters);
		return true;
	}

	//static
	bool CFileIOBase::WriteFileLines(const Common::ComString& strFileName, const std::list<Common::ComString>& listLines, bool bAppend)
	{
		Common::ComString strNewFileName = adjustFileName(strFileName, true);
		std::basic_ofstream<Common::ComChar> os(strNewFileName.c_str(),
												std::ios_base::out
												// | std::ios_base::binary
												// | (bAppend ? std::ios::app : (std::ios::openmode)0)
												| (bAppend ? std::ios::ate : (std::ios::openmode)0)
												);
		if (!os.is_open())
		{
			return false;
		}
		for (std::list<Common::ComString>::const_iterator it = listLines.begin(), itEnd = listLines.end(); it != itEnd; ++it)
		{
			os << *it << std::endl;
		}
		return true;
	}

	//static
	bool CFileIOBase::WriteFileStream(const Common::ComString& strFileName, const std::basic_stringstream<Common::ComChar>& ss, bool bAppend)
	{
		Common::ComString strNewFileName = adjustFileName(strFileName, true);
		std::basic_ofstream<Common::ComChar> os(strNewFileName.c_str(),
												std::ios_base::out
												// | std::ios_base::binary
												// | (bAppend ? std::ios::app : (std::ios::openmode)0)
												| (bAppend ? std::ios::ate : (std::ios::openmode)0)
												);
		if (!os.is_open())
		{
			return false;
		}
		os << ss.rdbuf();
		return true;
	}

	//static
	bool CFileIOBase::GetFileOStream(const Common::ComString& strFileName, std::basic_ofstream<Common::ComChar>& os, bool bBinaryMode, bool bAppend)
	{
		Common::ComString strNewFileName = adjustFileName(strFileName, true);
		os.open(strNewFileName.c_str(),
				std::ios_base::out
				| (bBinaryMode ? std::ios_base::binary : (std::ios::openmode)0)
				// | (bAppend ? std::ios::app : (std::ios::openmode)0)
				| (bAppend ? std::ios::ate : (std::ios::openmode)0)
				);
		bool bRes = os.is_open();
		return bRes;
	}

	////////////////////////////////////////////////////////////////////////////////

} //namespace FileIO
