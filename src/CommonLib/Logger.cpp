#ifdef _MSC_VER
	#define _CRT_SECURE_NO_WARNINGS
#endif

#include "Logger.h"
#include <thread>
#include <ctime>
#include <iostream>

namespace Common
{ //namespace Common

Common::ComString Logger::s_sDefaultModuleName;
Logger::LogSingleton Logger::s_LoggingInstance;

void Logger::SetMessageFormat(Common::Uint32 uiMsgFormat)
{
	s_LoggingInstance.s_uiMesgFormat = uiMsgFormat;
}

bool Logger::InitializeLogging(ELoggerLevels eLoggingLevel, bool bStdOutput, bool bFileOutput, const ComString& sLogFile, ELogMessageFormat eFormat)
{
	s_LoggingInstance.m_bInitialized = true;
	s_LoggingInstance.m_eLoggingLevel = eLoggingLevel;
	s_LoggingInstance.m_bStdOutput = bStdOutput;
	s_LoggingInstance.m_bFileOutput = bFileOutput;
	s_LoggingInstance.m_sFileName = sLogFile;
	s_LoggingInstance.s_uiMesgFormat = eFormat;
	if (bFileOutput)
	{
		s_LoggingInstance.m_fileLog.open(s_LoggingInstance.m_sFileName);  
		if (!(s_LoggingInstance.m_bInitialized = s_LoggingInstance.m_fileLog.is_open()))
		{
			s_LoggingInstance.m_bFileOutput = false;
			s_LoggingInstance.m_bStdOutput = true;
			Start(eInfo) << "Log file " << s_LoggingInstance.m_sFileName <<" cannot be opened" << End;
		}
	}
	return s_LoggingInstance.m_bInitialized;
}

Logger::Logger(ELoggerLevels eLevel, const Common::ComString& sModuleName)
	: m_eLevel(eLevel), m_sModuleName(sModuleName)
{

}

Logger::Logger(const Common::ComString& sModuleName, ELoggerLevels eLevel)
	: m_eLevel(eLevel), m_sModuleName(sModuleName)
{

}

Logger::Logger()
	: m_eLevel(eInfo), m_sModuleName("UnknownModule")
{

}

/*
bool Logger::Initialize(const Common::ComString sLogPath,const Common::ComString sPrefix,const Common::ComString sExtention)
{
	s_bInitialized = true;
	return s_bInitialized;
}
*/

Logger Logger::Start(ELoggerLevels eLevel, const Common::ComString& sModuleName)
{
	return Logger(eLevel, sModuleName);
}

Logger Logger::Start(ELoggerLevels eLevel)
{
	return Logger(eLevel); 
}
Logger Logger::Start(const Common::ComString& sModuleName)
{
	return Logger(sModuleName);
}
Logger Logger::Start()
{
	return Logger(); 
}

const ComString Logger::s_csDelimiter = _ComStr("  ");

const char* GetErorLevel(ELoggerLevels eLevel)
{
	switch (eLevel)
	{
	case Common::eTrace:
		return "TRACE";
	case Common::eDebug:
		return "DEBUG";
	case Common::eInfo:
		return "INFO";
	case Common::eWarn:
		return "WARNING";
	case Common::eError:
		return "ERROR";
	case Common::eFatal:
		return "FATAL";
	default:
		return "UNKNOWN";
	}
}

void Logger::PrintMessage(bool bForceConsole)
{
	if(s_LoggingInstance.m_bInitialized)
	{
		const int cTimeBufferSize=64;
		char buf[cTimeBufferSize] = {0};
		std::stringstream strstrm;
		if (s_LoggingInstance.s_uiMesgFormat & eLogTimeStamp)
		{
			std::time_t result = std::time(NULL);
			std::tm* tm = std::gmtime(&result);
			std::strftime(buf, cTimeBufferSize, "%y-%m-%d-%H:%M:%S", tm);
			strstrm << buf << s_csDelimiter;
		}
		if (s_LoggingInstance.s_uiMesgFormat & eLogProcId)
		{
			strstrm << std::this_thread::get_id() << s_csDelimiter;
		}
		if (s_LoggingInstance.s_uiMesgFormat & eLogModuleName)
		{
			strstrm << m_sModuleName << s_csDelimiter;
		}
		if (s_LoggingInstance.s_uiMesgFormat & eLogErrorLevel)
		{
			strstrm << GetErorLevel(m_eLevel) << s_csDelimiter;
		}

		if (s_LoggingInstance.m_bStdOutput || bForceConsole)
		{
			std::cout << strstrm.str() << s_csDelimiter << m_Strm.str() << std::endl;
		}
		if (s_LoggingInstance.m_bFileOutput && s_LoggingInstance.m_fileLog.is_open())
		{
			s_LoggingInstance.m_fileLog << strstrm.str() << s_csDelimiter << m_Strm.str() << std::endl;
		}
	}
	m_Strm.str(ComString());
}

void Logger::End(Logger& log)
{
	log.PrintMessage();
}

void Logger::EndConsole(Logger& log)
{
	log.PrintMessage(true);
}

} //namespace Common
