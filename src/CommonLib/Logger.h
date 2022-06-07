#pragma once

#include "Typedefs.h"

#include <sstream>
#include <fstream>
#include <chrono>

namespace Common
{

enum ELoggerLevels
{
	eTrace, eDebug, eInfo, eWarn, eError, eFatal
};

class Logger;

typedef void (*VoidFuncPtrType)(Logger& log);
void EndRecord(Logger& log);

class Logger
{
public:
	enum ELogMessageFormat
	{
		eLogTimeStamp = 0x01,
		eLogProcId = 0x02,
		eLogModuleName = 0x04,
		eLogErrorLevel = 0x08,
		eLogFullFormat = 0x0F
	};
private:
	struct LogSingleton
	{
		bool m_bInitialized;
		ELoggerLevels m_eLoggingLevel;
		std::ofstream  m_fileLog;
		bool m_bStdOutput;
		bool m_bFileOutput;
		ComString m_sFileName;
		Common::Uint32 s_uiMesgFormat;
	};
private:
	Logger(ELoggerLevels eLevel, const Common::ComString& sModuleName = _ComStr("UnknownModule"));
	Logger(const Common::ComString& sModuleName, ELoggerLevels eLevel = eInfo);
	Logger();
	Logger(const Logger& /*log*/){}

	static LogSingleton s_LoggingInstance;
	static const ComString s_csDelimiter;

	std::basic_stringstream<Common::ComChar> m_Strm;
	ELoggerLevels m_eLevel;
	Common::ComString m_sModuleName;

	void PrintMessage(bool bForceConsole = false);
public:

	static Common::ComString s_sDefaultModuleName;

static bool InitializeLogging(ELoggerLevels eLoggingLevel, bool bStdOutput, bool bFileOutput, const ComString& sLogFile,ELogMessageFormat eFormat = eLogFullFormat);
static void SetMessageFormat(Common::Uint32 uiMsgFormat);


static Logger Start(ELoggerLevels eLevel, const Common::ComString& sModuleName);
static Logger Start(ELoggerLevels eLevel);
static Logger Start(const Common::ComString& sModuleName);
static Logger Start();
static void End(Logger& log);
static void EndConsole(Logger& log);

//void LogMessage(const Common::ComString sLogMsg, const Common::ComString sLogLevel, const Common::ComString sModuleName, const Common::ComString& ThreadId);

template <typename T>
Logger& operator << (const T& in)
{
	m_Strm << in;
	return *this;
}
Logger& operator << ( VoidFuncPtrType funcptr)
{
	funcptr(*this);
	return *this;
}

};

class LogHelper
{
	ELoggerLevels m_eLevel;
	Common::ComString m_sModuleName;
	bool m_bForseConsole;
	std::chrono::high_resolution_clock::time_point m_tmCheckPointBegin;
	std::chrono::high_resolution_clock::time_point m_tmCheckPointEnd;
public :
LogHelper(ELoggerLevels eLevel = Common::eInfo, Common::ComString sModuleName = _ComStr("UnkmownModule"))
	:m_eLevel(eLevel),m_sModuleName(sModuleName),m_bForseConsole(false){}
LogHelper(Common::ComString sModuleName)
	:m_eLevel(Common::eInfo),m_sModuleName(sModuleName),m_bForseConsole(false){}

LogHelper& operator() ()
{
	return *this;
}
LogHelper& operator() (ELoggerLevels eLevel)
{
	m_eLevel=eLevel; 
	return *this;
}
LogHelper& operator() (ELoggerLevels eLevel, const Common::ComString& sModuleName)
{
	m_eLevel=eLevel;
	m_sModuleName = sModuleName;
	return *this;
}
LogHelper& operator() (const Common::ComString& sModuleName)
{
	m_sModuleName = sModuleName;
	return *this;
}
LogHelper& SetTimeCheckPoint()
{
	operator()(eInfo).Log(_ComStr("Time check point was set"));
	m_tmCheckPointBegin = std::chrono::high_resolution_clock::now(); //std::clock();
	return *this;
}
LogHelper& SecondsInterval()
{
	m_tmCheckPointEnd = std::chrono::high_resolution_clock::now();//std::clock();
	auto interval = std::chrono::duration_cast<std::chrono::seconds>(m_tmCheckPointEnd - m_tmCheckPointBegin).count();
	operator()(eInfo).Log(_ComStr("Check point interval is "),interval,_ComStr(" seconds"));
	return *this;
}
LogHelper& MillisecondsInterval()
{
	m_tmCheckPointEnd = std::chrono::high_resolution_clock::now();//std::clock();
	auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(m_tmCheckPointEnd - m_tmCheckPointBegin).count();
	operator()(eInfo).Log(_ComStr("Check point interval is "),interval,_ComStr(" milliseconds"));
	return *this;
}
LogHelper& MicrosecondsInterval()
{
	m_tmCheckPointEnd = std::chrono::high_resolution_clock::now();//std::clock();
	auto interval = std::chrono::duration_cast<std::chrono::microseconds>(m_tmCheckPointEnd - m_tmCheckPointBegin).count();
	operator()(eInfo).Log(_ComStr("Check point interval is "),interval,_ComStr(" microseconds"));
	return *this;
}
LogHelper& NanosecondsInterval()
{
	m_tmCheckPointEnd = std::chrono::high_resolution_clock::now();//std::clock();
	auto interval = std::chrono::duration_cast<std::chrono::nanoseconds>(m_tmCheckPointEnd - m_tmCheckPointBegin).count();
	operator()(eInfo).Log(_ComStr("Check point interval is "),interval,_ComStr(" nanoseconds"));
	return *this;
}

template<typename T1>
void Log(const T1& t1)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << Logger::End;
}

template<typename T1, typename T2>
void Log(const T1& t1, const T2& t2)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 <<Logger::End;
}

template<typename T1, typename T2, typename T3>
void Log(const T1& t1, const T2& t2, const T3& t3)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << Logger::End;
}

template<typename T1, typename T2, typename T3, typename T4>
void Log(const T1& t1, const T2& t2, const T3& t3, const T4& t4)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << Logger::End;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5>
void Log(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << Logger::End;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void Log(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << Logger::End;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void Log(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << Logger::End;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void Log(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << Logger::End;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
void Log(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << Logger::End;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
void Log(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << Logger::End;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11>
void Log(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10, const T11& t11)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << Logger::End;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12>
void Log(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10, const T11& t11, const T12& t12)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << Logger::End;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13>
void Log(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10, const T11& t11, const T12& t12, const T13& t13)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << Logger::End;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14>
void Log(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10, const T11& t11, const T12& t12, const T13& t13, const T14& t14)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << t14 << Logger::End;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15>
void Log(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10, const T11& t11, const T12& t12, const T13& t13, const T14& t14, const T15& t15)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << t14 << t15 << Logger::End;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16>
void Log(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10, const T11& t11, const T12& t12, const T13& t13, const T14& t14, const T15& t15, const T16& t16)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << t14 << t15 << t16 << Logger::End;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17>
void Log(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10, const T11& t11, const T12& t12, const T13& t13, const T14& t14, const T15& t15, const T16& t16, const T17& t17)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << t14 << t15 << t16 << t17 << Logger::End;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18>
void Log(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10, const T11& t11, const T12& t12, const T13& t13, const T14& t14, const T15& t15, const T16& t16, const T17& t17, const T18& t18)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << t14 << t15 << t16 << t17 << t18 << Logger::End;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19>
void Log(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10, const T11& t11, const T12& t12, const T13& t13, const T14& t14, const T15& t15, const T16& t16, const T17& t17, const T18& t18, const T19& t19)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << t14 << t15 << t16 << t17 << t18 << t19 << Logger::End;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20>
void Log(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10, const T11& t11, const T12& t12, const T13& t13, const T14& t14, const T15& t15, const T16& t16, const T17& t17, const T18& t18, const T19& t19, const T20& t20)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << t14 << t15 << t16 << t17 << t18 << t19 << t20 << Logger::End;
}

template<typename T1>
void Console(const T1& t1)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << Logger::EndConsole;
}

template<typename T1, typename T2>
void Console(const T1& t1, const T2& t2)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << Logger::EndConsole;
}

template<typename T1, typename T2, typename T3>
void Console(const T1& t1, const T2& t2, const T3& t3)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << Logger::EndConsole;
}

template<typename T1, typename T2, typename T3, typename T4>
void Console(const T1& t1, const T2& t2, const T3& t3, const T4& t4)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << Logger::EndConsole;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5>
void Console(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << Logger::EndConsole;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void Console(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << Logger::EndConsole;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void Console(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << Logger::EndConsole;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void Console(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << Logger::EndConsole;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
void Console(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << Logger::EndConsole;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
void Console(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << Logger::EndConsole;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11>
void Console(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10, const T11& t11)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << Logger::EndConsole;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12>
void Console(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10, const T11& t11, const T12& t12)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << Logger::EndConsole;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13>
void Console(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10, const T11& t11, const T12& t12, const T13& t13)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << Logger::EndConsole;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14>
void Console(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10, const T11& t11, const T12& t12, const T13& t13, const T14& t14)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << t14 << Logger::EndConsole;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15>
void Console(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10, const T11& t11, const T12& t12, const T13& t13, const T14& t14, const T15& t15)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << t14 << t15 << Logger::EndConsole;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16>
void Console(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10, const T11& t11, const T12& t12, const T13& t13, const T14& t14, const T15& t15, const T16& t16)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << t14 << t15 << t16 << Logger::EndConsole;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17>
void Console(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10, const T11& t11, const T12& t12, const T13& t13, const T14& t14, const T15& t15, const T16& t16, const T17& t17)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << t14 << t15 << t16 << t17 << Logger::EndConsole;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18>
void Console(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10, const T11& t11, const T12& t12, const T13& t13, const T14& t14, const T15& t15, const T16& t16, const T17& t17, const T18& t18)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << t14 << t15 << t16 << t17 << t18 << Logger::EndConsole;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19>
void Console(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10, const T11& t11, const T12& t12, const T13& t13, const T14& t14, const T15& t15, const T16& t16, const T17& t17, const T18& t18, const T19& t19)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << t14 << t15 << t16 << t17 << t18 << t19 << Logger::EndConsole;
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20>
void Console(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10, const T11& t11, const T12& t12, const T13& t13, const T14& t14, const T15& t15, const T16& t16, const T17& t17, const T18& t18, const T19& t19, const T20& t20)
{
	Logger::Start(m_eLevel, m_sModuleName) << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << t14 << t15 << t16 << t17 << t18 << t19 << t20 << Logger::EndConsole;
}

};

}
