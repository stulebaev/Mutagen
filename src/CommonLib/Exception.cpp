#ifdef _MSC_VER
	#define _CRT_SECURE_NO_WARNINGS
#endif

#include "Exception.h"

#include <cstdarg>

namespace Common
{
	const Common::ComString GetFileName(const Common::ComString& strPath)
	{
		Common::ComString strFileName;
		size_t nPos;
		if ((nPos = strPath.find_last_of(_ComStr("/\\"))) != Common::ComString::npos)
		{
			strFileName = strPath.substr(nPos + 1, strPath.length() - (nPos + 1));
		}
		else
		{
			strFileName = strPath;
		}
		return strFileName;
	}

	CException::CException(const CException& srcException)
	{
		init();
		*this = srcException;
		m_bIsInternalError = srcException.m_bIsInternalError;
	}

	CException::CException()
	{
		init();
	}

	CException::~CException()
	{
		Clear();
	}

	void CException::Clear()
	{ 
		delete [] m_szErrorMessage;
		init(); 
	}

	CException& CException::operator = (const Common::ComChar* szErrorMessage)
	{
		Clear();
		if (szErrorMessage == NULL)
		{
			m_szErrorMessage = NULL;
		}
		else
		{
			size_t len = strlen(szErrorMessage);
			try
			{
				if (szErrorMessage[0] == _ComStr('#'))
				{
					if (szErrorMessage[len - 1] == _ComStr('!'))
					{
						m_szErrorMessage = new Common::ComChar[len + 1];
						if (m_szErrorMessage != NULL)
						{
							strncpy(m_szErrorMessage, szErrorMessage, len);
							m_szErrorMessage[len] = _ComStr('\0');
						}
					}
					else
					{
						m_szErrorMessage = new Common::ComChar[len + 2];
						if (m_szErrorMessage != NULL) 
						{
							strncpy(m_szErrorMessage, szErrorMessage, len);
							m_szErrorMessage[len] = _ComStr('!');
							m_szErrorMessage[len + 1] = 0;
						}
					}
				}
				else
				{
					m_szErrorMessage =  new Common::ComChar[len + 3];
					if (m_szErrorMessage != NULL) 
					{
						m_szErrorMessage[0] = _ComStr('#');
						strncpy(m_szErrorMessage + 1, szErrorMessage, len);
						m_szErrorMessage[len + 1] = _ComStr('!');
						m_szErrorMessage[len + 2] = _ComStr('\0');
					}
				}
			}
			catch (std::bad_alloc &)
			{
				// couldn't allocate memory to copy the string over.
				// Continue but won't have the message string.
				m_szErrorMessage = NULL;
			}
		}
		return *this;
	}

	CException& CException::operator = (const CException &srcException)
	{
		Clear();
		if (srcException.m_szErrorMessage == NULL)
		{
			m_szErrorMessage = NULL;
		}
		else
		{
			size_t len = strlen(srcException.m_szErrorMessage);
			m_szErrorMessage = new Common::ComChar[len + 1];
			if (m_szErrorMessage != NULL) 
			{
				strncpy(m_szErrorMessage, srcException.m_szErrorMessage, len);
				m_szErrorMessage[len] = _ComStr('\0');
			}
		}
		return *this;
	}

	void CException::AddContext(const Common::ComChar* szFile, int iLine, const Common::ComChar* context)
	{
		// turns "#InternalError: foo!" into "#InternalError: context: foo!";
		// turns "#foo!" into "#context: foo!";
		//
		// a hack for now, to support adding context info when we have
		// caught a CException and thus have no control over its original construction;
		// later should rationalize this class;
		// the right way to do this class would be to maintain the
		// individual substrings, and only concat them together just-in-time, in Message(),
		// and perhaps cacheing that;

		if (context == NULL)
		{
			return;
		}
		Common::ComString strMsg(context);
		strMsg += _ComStr(": ");
		strMsg += getRawMessage();
		// save these as Clear() wipes them:
		bool bIsInternal = m_bIsInternalError;
		if (m_bIsInternalError)
		{
			internalMessage(szFile, iLine, strMsg.c_str());
		}
		else
		{
			*this = strMsg.c_str();
		}
		m_bIsInternalError = bIsInternal;
	}

	/*static */ void CException::ThrowInternal(const Common::ComChar* szFile, int iLine, const Common::ComChar* szErrorMessage) // throw (CException)
	{
		CException Error;
		Error.internalMessage(szFile, iLine, szErrorMessage);
		Error.Throw();
	}

	/*static */ void CException::Throw(const Common::ComChar* szFile, int iLine, const Common::ComChar* szErrorMessage, ...) // throw (CException)
	{
		va_list pArguments;
		va_start(pArguments, szErrorMessage);
		try
		{
			ThrowV(szFile, iLine, szErrorMessage, pArguments);
		}
		catch (...)
		{
			va_end(pArguments);
			throw;
		}
		va_end(pArguments);
	}

	/*static */ void CException::ThrowV(const Common::ComChar* szFile, int iLine, const Common::ComChar* szErrorMessage, va_list pArguments) // throw (CException)
	///*static */ void CException::ThrowV(const Common::ComChar* szFile, int iLine, const Common::ComChar* szErrorMessage, char* pArguments) // throw (CException)
	{
		const size_t cMaxChars = 1024;
		Common::ComChar buffer[cMaxChars + 1024];

		//if (_vsnprintf(buffer, cMaxChars, szErrorMessage, pArguments) < 0)
		if (vsnprintf(buffer, cMaxChars, szErrorMessage, pArguments) < 0)
		{
			buffer[cMaxChars - 1] = _ComStr('\0');
		}

		if (szFile && *szFile)
		{
			char szLine[1024];
			sprintf(szLine, _ComStr(" [%s.%d]"), GetFileName(szFile).c_str(), iLine);
			strcat(buffer, szLine);
		}

		CException Error = buffer;
		Error.Throw();
	}

	void CException::Throw(const Common::ComChar* szFile, int iLine, const Common::ComString& strErrorMessage) // throw (CException)
	{
		Throw(szFile, iLine, strErrorMessage.c_str());
	}

	void CException::Throw() const // throw (CException)
	{
		throw *this;
	}

	const Common::ComChar* CException::Message() const
	{
		if (m_szErrorMessage == NULL)
		{
			return _ComStr("Unknown Error");
		}
		return m_szErrorMessage;
	}

	bool CException::IsInternalError() const
	{
		return m_bIsInternalError;
	}

	void CException::init()
	{
		m_szErrorMessage = NULL;
		m_bIsInternalError = false;
	}

	void CException::internalMessage(const Common::ComChar* szFile, int iLine,  const Common::ComChar* szErrorMessage)
	{
		Clear();
		m_bIsInternalError = true;
		if (szErrorMessage == NULL)
		{
			m_szErrorMessage = NULL;
		}
		else
		{
			const short extraLen = 16;
			Common::ComChar szInternalError[extraLen + 1] = _ComStr("#InternalError: ");
			int len = 0;
			const Common::ComChar* ptr = szErrorMessage;
			for ( ; *ptr != _ComStr('\0'); ++ptr)
			{
				if (*ptr >= _ComStr(' ') && *ptr <= _ComStr('z')) 
				{
					++len;
				}
			}
			if (len > 0 && (*--ptr == _ComStr('.') || *ptr == _ComStr('!'))) 
			{
				--len;
			}

	//		Common::ComChar*  szFileMessage = (char*) _alloca(len + extraLen + 2 + 300);
			Common::ComChar*  szFileMessage = szInternalError;
			Common::ComChar   szLine[300];

			if (szFile && *szFile)
			{
				sprintf(szLine, _ComStr(" [%s.%d]!"), GetFileName(szFile).c_str(), iLine);
			}
			else
			{
				strcpy(szLine, _ComStr("!"));
			}

			m_szErrorMessage = new Common::ComChar [len + extraLen + 2 + 300];
			if (m_szErrorMessage != NULL)
			{
				strcpy(m_szErrorMessage, szFileMessage);

				char *ptr2 = m_szErrorMessage + extraLen;

				for (ptr = szErrorMessage; *ptr != _ComStr('\0'); ++ptr)
				{
					if (*ptr >= _ComStr(' ') && *ptr <= _ComStr('z'))
					{
						*ptr2++ = *ptr;
					}
				}
				if (*--ptr2 == _ComStr('.') || *ptr2 == _ComStr('!'))
				{
					*ptr2 = _ComStr('\0');
				}
				else
				{
					*++ptr2 = _ComStr('\0');
				}

				*++ptr2 = _ComStr('\0');

				strcat(m_szErrorMessage, szLine);
			}
		}
	}

	const Common::ComChar* CException::getRawMessage() const
	{
		// for "#InternalError: foo!" and "#foo!" returns ptr to "foo!";
		// ie, it's what was originally passed to construction, except for the '!';
		if (m_szErrorMessage == NULL)
		{
			return _ComStr("Unknown Error");
		}
		if (m_bIsInternalError)
		{
			return m_szErrorMessage + sizeof(_ComStr("#InternalError: ")) - 1;
		}
		return m_szErrorMessage + 1;
	}

} //namespace Common
