#pragma once

#include <string>

namespace Common {

  typedef signed int Int32;
  typedef unsigned int Uint32;
  typedef Int32 IndexType;
  typedef double NUMERIC;

  #define _ONEBYTECHARACTERS
  typedef char ComChar;
  typedef std::string ComString;
  #define _ComStr(X) X
  #define _ComChar(X) X
  #define _ToString(X) std::to_string(X)
  #define _ToLong(pX,ppY) std::strtol(pX, ppY, 10)

} //namespace Common
