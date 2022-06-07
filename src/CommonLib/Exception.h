// CException.h: class for throwing exceptions
#pragma once

#include "Typedefs.h"

namespace Common
{
	// CException
	// --------------------------------------------------------------------------
	// Class implements functionality for generating and throwing the user-managed exceptions.
	// Gives the opportunity of formatting the message describing exceptional case
	// Класс реализует функциональность генерации и бросания контролируемых пользователем исключений.
	// Предоставляет возможность форматирования сообщения об исключительной ситуации

	class CException
	{
	public:
		CException();
		CException(const Common::ComChar* szErrorMessage)
		{
			init();
			*this = szErrorMessage;
		}
		CException(const CException& );
		CException(const Common::ComString& strErrorMessage)
		{
			init();
			*this = strErrorMessage.c_str();
		}
		~CException();
		void Clear();

		CException& operator = (const Common::ComChar *szErrorMessage);
		CException& operator = (const CException &srcError);
		void AddContext(const Common::ComChar* szFile, int iLine,  const Common::ComChar* context);

		static void ThrowInternal(const Common::ComChar* szFile, int iLine,  const Common::ComChar* szErrorMessage); // throw (CException);
		static void Throw(const Common::ComChar* szFile, int iLine, const Common::ComChar* szErrorMessage, ...); // throw (CException);
		static void ThrowV(const Common::ComChar* szFile, int iLine, const Common::ComChar* szErrorMessage, va_list pArguments); // throw (CException);
		//static void ThrowV(const Common::ComChar* szFile, int iLine, const Common::ComChar* szErrorMessage, Common::ComChar* pArguments); // throw (CException);
		static void Throw(const Common::ComChar* szFile, int iLine, const Common::ComString& strErrorMessage); // throw (CException);
		void Throw() const; // throw (CException);

		const Common::ComChar* Message() const;

		bool IsInternalError() const;

	private:
		Common::ComChar* m_szErrorMessage; //Buffer for message
		bool m_bIsInternalError; //Flag for whether the generated exception is internal one

	private:
		void init();
		void internalMessage(const Common::ComChar* szFile, int iLine, const Common::ComChar* szErrorMessage);
		const Common::ComChar* getRawMessage() const;
	};

	#define CTHROW_MES(message) CException::Throw(__FILE__,__LINE__,_ComStr(#message))

	#define CVERIFY_CALL(func) {HRESULT _nhres; if(FAILED(_nhres=##func)) CException::Throw(__FILE__,__LINE__,_nhres,_ComStr(#func));}
	#define CVERIFY_PTR(ptr) if(!(ptr)) CException::Throw(__FILE__,__LINE__,E_POINTER,_ComStr(#ptr));else
	#define CVERIFY_ARG(exp) if(!(exp)) CException::Throw(__FILE__,__LINE__,E_INVALIDARG,_ComStr(#exp));else
	#define CVERIFY_EXP(exp) if(!(exp)) CException::Throw(__FILE__,__LINE__,_T(#exp) _ComStr(" verification failed"));else

} //namespace Common
