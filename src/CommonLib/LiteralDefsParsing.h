#pragma once

#include "MolDefines.h"
#include "Exception.h"


namespace Common
{ //namespace Common

	inline bool StringToIntChar(const Common::ComString& strLine, Common::IndexType& iInteger, Common::ComChar& cChar)
	{ //Function tries to convert input string to integer and may be [alphabetic] char
		Common::ComChar* pEndPtr = NULL;
		size_t nPos = strLine.find_first_not_of(_ComStr("-0123456789"));
		if (nPos == Common::ComString::npos)
		{
			iInteger = _ToLong(strLine.c_str(), &pEndPtr);
			cChar = _ComChar('\0');
			return ( !pEndPtr || *pEndPtr == _ComChar('\0') ) ? true : false;
		}
		else if (nPos + 1 == strLine.length())
		{
			iInteger = _ToLong(strLine.substr(0, nPos).c_str(), &pEndPtr);
			cChar = strLine[nPos];
			return ( !pEndPtr || *pEndPtr == _ComChar('\0') ) ? true : false;
		}
		else // Unexpected case
		{ //STUB
			iInteger = -1;
			cChar = _ComChar('\0');
			return false;
		}
	}

	template <typename _ContainerIntegers>
	inline bool ParseListOfIntegers(const Common::ComString& strLine, _ContainerIntegers& contValues, bool& bAny)
	{	//Function parses the string supposing that it contains a list of integers. 
		//The list can contain ',', ';', ':', '-', '*'. 
		//Asterisk '*' at any position means 'all', so it triggers the 'bAny' flag and exit
		//Dash '-' or colon ':', or double dot ".." are treated as mark for intervals if they go just after and just before numbers, without spaces/tabs,
		//e.g. "19-21", "-17--4","-3-2","5-2", or "19:21", "-17:-4","-3:2","5:2", or "19..21", "-17..-4","-3..2","5..2"
		bAny = false;
		bool bNumber = false, bLastRange = false;
		Int32 nRange = -1;
		Common::IndexType nNumber = 0;
		Common::ComString strNumber;
		size_t i, len = strLine.length(), nPos = 0, nPos2 = Common::ComString::npos;

		if ((nPos = strLine.find_first_not_of(_ComStr(" \t,;.:-*0123456789"))) != Common::ComString::npos)
		{
			Common::CException::Throw(__FILE__, __LINE__, _ComStr("ParseListOfIntegers(): Found unexpected symbol(s) at position %d"), nPos);
			return false; //Found unexpected symbol(s)
		}

		nPos = 0;
		for (i = 0, len = strLine.length(); i < len; ++i)
		{
			switch (strLine[i])
			{
			case _ComStr('*'):
				bAny = true;
				return true;
			case _ComStr('-'):
				if (bNumber) //There was a number before this, so this '-' should be treated as delimiter in a range
				{
					if (nRange >= 0)
					{
						CException::Throw(__FILE__, __LINE__, _ComStr("ParseListOfIntegers(): Found '-' treated as duplicating mark of range at position %d (see previous mark '%c' at position %d)"), i, strLine[nRange], nRange);
						return false;
					}
					bNumber = false;
					nRange = (Common::Int32)i;
					nPos2 = i;
				}
				else //This '-' is [probably] a sign of an incoming number
				{
					if (i+1 < len && strLine[i+1] >= _ComStr('0') && strLine[i+1] <= _ComStr('9'))
					{
						nPos = i;
						//bRange = false; //Not reset the flag just now? the flag will be reset with the next digit
					}
					else
					{
						CException::Throw(__FILE__, __LINE__, _ComStr("ParseListOfIntegers(): Found '-' treated as sign mark (it does not immediately follow any digit) but without subsequent digit at position %d"), i);
						return false;
					}
				}
				break;

			case _ComStr(':'):
				if (nRange >= 0)
				{
					CException::Throw(__FILE__, __LINE__, _ComStr("ParseListOfIntegers(): Found ':' treated as duplicating mark of range at position %d (see previous mark '%c' at position %d)"), i, strLine[nRange], nRange);
					return false;
				}
				if (!bNumber)
				{
					CException::Throw(__FILE__, __LINE__, _ComStr("ParseListOfIntegers(): Found ':' without preceeding number at position %d"), i);
					return false;
				}
				bNumber = false;
				nRange = (Common::Int32)i;
				nPos2 = i;
				break;

			case _ComStr('.'):
				//Check for the dot mark is not single. Do not require it is just double (implicitly allow it being triple etc.)
				if (!(i+1 < len && strLine[i+1] == _ComStr('.')))
				{
					CException::Throw(__FILE__, __LINE__, _ComStr("ParseListOfIntegers(): Found single '.' (only multiple dots are allowed as range marks) at position %d"), i);
					return false;
				}
				if (nRange >= 0)
				{
					CException::Throw(__FILE__, __LINE__, _ComStr("ParseListOfIntegers(): Found '.' treated as duplicating mark of range at position %d (see previous mark '%c' at position %d)"), i, strLine[nRange], nRange);
					return false;
				}
				if (!bNumber)
				{
					CException::Throw(__FILE__, __LINE__, _ComStr("ParseListOfIntegers(): Found '.' without preceeding number at position %d"), i);
					return false;
				}
				bNumber = false;
				nRange = (Common::Int32)i;
				nPos2 = i;
				//Skip remaining dot marks. Do not require dot mark is just double (implicitly allow it being triple etc.)
				while (i+1 < len && strLine[i+1] == _ComStr('.'))
				{
					++i;
					++nRange; //Let nRange point on the last dot '.' symbol in the sequence
				}
				break; //Stub

			case _ComStr(','):
			case _ComStr(';'):
				//bNumber = false; //Do not break
			case _ComStr(' '):
			case _ComStr('\t'):
				bNumber = false;
				nRange = -1;
				if (nPos == i)
				{
					nPos = i + 1;
				}
				else
				{
					nPos2 = i;
				}
				break;

			case _ComStr('0'):
			case _ComStr('1'):
			case _ComStr('2'):
			case _ComStr('3'):
			case _ComStr('4'):
			case _ComStr('5'):
			case _ComStr('6'):
			case _ComStr('7'):
			case _ComStr('8'):
			case _ComStr('9'):
				bNumber = true;
				nRange = -1;
			break;
			default:
				Common::CException::Throw(__FILE__, __LINE__, _ComStr("ParseListOfIntegers(): Found unexpected symbol(s) of '%c' at position %d"), strLine[i], i);
				return false; //Found unexpected symbol(s)
			};
			if (i+1 == len) //Reached end of the line
			{
				nPos2 = len;
			}
			if (nPos2 != Common::ComString::npos //Finished record for a number
				&& nPos2 != nPos
				)
			{
				strNumber = strLine.substr(nPos, nPos2 - nPos);
				nNumber = std::stoi(strNumber);
				if (bLastRange)
				{
					if (nNumber > contValues.back())
					{
						for (Int32 n = contValues.back() + 1; n <= nNumber; ++n)
							contValues.push_back(n);
					}
					else
					{
						for (Int32 n = contValues.back() - 1; n >= nNumber; --n)
							contValues.push_back(n);
					}
					bLastRange = (nRange >= 0); //bLastRange = false;
				}
				else
				{
					contValues.push_back(nNumber);
				}
				nPos = nPos2 + 1;
				nPos2 = Common::ComString::npos;
			}
			else
			{
				bLastRange = (nRange >= 0);
			}
		} //i
		if (nRange >= 0)
		{
			Common::CException::Throw(__FILE__, __LINE__, _ComStr("ParseListOfIntegers(): Detected imcomplete range description (mark '%c' at position %d) while reached the end of line"), strLine[nRange], nRange);
			return false; //Found unexpected symbol(s)
		}
		return true;
	}

	template <typename _ContainerIntegers>
	inline bool ParseListOfIntegerAndChars(const Common::ComString& strLine, _ContainerIntegers& contValues, bool& bAny)
	{	//Function parses the string supposing that it contains a list of integers and/or concatenations of integer and alphabetical char, like '123A'.
		//The list can contain ',', ';', ':', '-', '*'. 
		//Asterisk '*' at any position means 'all', so it triggers the 'bAny' flag and exit
		//Dash '-' or colon ':', or double dot ".." are treated as mark for intervals if they go just after numbers and just before numbers, without spaces/tabs,
		//e.g. "19-21", "-17--4","-3-2","5-2", or "19:21", "-17:-4","-3:2","5:2", or "19..21", "-17..-4","-3..2","5..2".
		//At this, combinations of '-', ':', '..' with 'number+char' are treated as errors
		//STUB: parsing of values without alphabetical chars
		std::vector<Common::IndexType> vValues;
		if (ParseListOfIntegers(strLine, vValues, bAny))
		{
			size_t nOldSize = contValues.size();
			contValues.resize(nOldSize + vValues.size());
			for (size_t i = 0; i < vValues.size(); ++i)
			{
				contValues[nOldSize + i].first = vValues[i];
				contValues[nOldSize + i].second = _ComChar('\0');
			}
			return true; //Stub
		}
		else
		{
			return false;
		}
/*
		bAny = false;
		bool bNumber = false, bLastRange = false;
		Int32 nRange = -1;
		Common::IndexType nNumber = 0;
		Common::ComChar cChar;
		Common::ComString strNumber;
		size_t i, len = strLine.length(), nPos = 0, nPos2 = Common::ComString::npos;

		if ((nPos = strLine.find_first_not_of(_ComStr(" \t,;.:-*0123456789"))) != Common::ComString::npos)
		{
			Common::CException::Throw(__FILE__, __LINE__, _ComStr("ParseListOfIntegerAndChars(): Found unexpected symbol(s) at position %d"), nPos);
			return false; //Found unexpected symbol(s)
		}

		nPos = 0;
		for (i = 0, len = strLine.length(); i < len; ++i)
		{
			switch (strLine[i])
			{
			case _ComStr('*'):
				bAny = true;
				return true;
			case _ComStr('-'):
				if (bNumber) //There was a number before this, so this '-' should be treated as delimiter in a range
				{
					if (nRange >= 0)
					{
						CException::Throw(__FILE__, __LINE__, _ComStr("ParseListOfIntegerAndChars(): Found '-' treated as duplicating mark of range at position %d (see previous mark '%c' at position %d)"), i, strLine[nRange], nRange);
						return false;
					}
					bNumber = false;
					nRange = (Common::Int32)i;
					nPos2 = i;
				}
				else //This '-' is [probably] a sign of an incoming number
				{
					if (i+1 < len && strLine[i+1] >= _ComStr('0') && strLine[i+1] <= _ComStr('9'))
					{
						nPos = i;
						//bRange = false; //Not reset the flag just now? the flag will be reset with the next digit
					}
					else
					{
						CException::Throw(__FILE__, __LINE__, _ComStr("ParseListOfIntegerAndChars(): Found '-' treated as sign mark (it does not immediately follow any digit) but without subsequent digit at position %d"), i);
						return false;
					}
				}
				break;

			case _ComStr(':'):
				if (nRange >= 0)
				{
					CException::Throw(__FILE__, __LINE__, _ComStr("ParseListOfIntegerAndChars(): Found ':' treated as duplicating mark of range at position %d (see previous mark '%c' at position %d)"), i, strLine[nRange], nRange);
					return false;
				}
				if (!bNumber)
				{
					CException::Throw(__FILE__, __LINE__, _ComStr("ParseListOfIntegerAndChars(): Found ':' without preceeding number at position %d"), i);
					return false;
				}
				bNumber = false;
				nRange = (Common::Int32)i;
				nPos2 = i;
				break;

			case _ComStr('.'):
				//Check for the dot mark is not single. Do not require it is just double (implicitly allow it being triple etc.)
				if (!(i+1 < len && strLine[i+1] == _ComStr('.')))
				{
					CException::Throw(__FILE__, __LINE__, _ComStr("ParseListOfIntegerAndChars(): Found single '.' (only multiple dots are allowed as range marks) at position %d"), i);
					return false;
				}
				if (nRange >= 0)
				{
					CException::Throw(__FILE__, __LINE__, _ComStr("ParseListOfIntegerAndChars(): Found '.' treated as duplicating mark of range at position %d (see previous mark '%c' at position %d)"), i, strLine[nRange], nRange);
					return false;
				}
				if (!bNumber)
				{
					CException::Throw(__FILE__, __LINE__, _ComStr("ParseListOfIntegerAndChars(): Found '.' without preceeding number at position %d"), i);
					return false;
				}
				bNumber = false;
				nRange = (Common::Int32)i;
				nPos2 = i;
				//Skip remaining dot marks. Do not require dot mark is just double (implicitly allow it being triple etc.)
				while (i+1 < len && strLine[i+1] == _ComStr('.'))
				{
					++i;
					++nRange; //Let nRange point on the last dot '.' symbol in the sequence
				}
				break; //Stub

			case _ComStr(','):
			case _ComStr(';'):
				//bNumber = false; //Do not break
			case _ComStr(' '):
			case _ComStr('\t'):
				bNumber = false;
				nRange = -1;
				if (nPos == i)
				{
					nPos = i + 1;
				}
				else
				{
					nPos2 = i;
				}
				break;

			case _ComStr('0'):
			case _ComStr('1'):
			case _ComStr('2'):
			case _ComStr('3'):
			case _ComStr('4'):
			case _ComStr('5'):
			case _ComStr('6'):
			case _ComStr('7'):
			case _ComStr('8'):
			case _ComStr('9'):
				bNumber = true;
				nRange = -1;
			break;
			default:
				Common::CException::Throw(__FILE__, __LINE__, _ComStr("ParseListOfIntegerAndChars(): Found unexpected symbol(s) of '%c' at position %d"), strLine[i], i);
				return false; //Found unexpected symbol(s)
			};
			if (i+1 == len) //Reached end of the line
			{
				nPos2 = len;
			}
			if (nPos2 != Common::ComString::npos //Finished record for a number
				&& nPos2 != nPos
				)
			{
				strNumber = strLine.substr(nPos, nPos2 - nPos);
				nNumber = std::stoi(strNumber);
				if (bLastRange)
				{
					if (nNumber > contValues.back())
					{
						for (Int32 n = contValues.back() + 1; n <= nNumber; ++n)
							contValues.push_back(n);
					}
					else
					{
						for (Int32 n = contValues.back() - 1; n >= nNumber; --n)
							contValues.push_back(n);
					}
					bLastRange = (nRange >= 0); //bLastRange = false;
				}
				else
				{
					contValues.push_back(nNumber);
				}
				nPos = nPos2 + 1;
				nPos2 = Common::ComString::npos;
			}
			else
			{
				bLastRange = (nRange >= 0);
			}
		} //i
		if (nRange >= 0)
		{
			Common::CException::Throw(__FILE__, __LINE__, _ComStr("ParseListOfIntegerAndChars(): Detected imcomplete range description (mark '%c' at position %d) while reached the end of line"), strLine[nRange], nRange);
			return false; //Found unexpected symbol(s)
		}
		return true;
*/
	}

	namespace Detail
	{ //namespace Detail

		//Fornat of element definition is supposed to be the following: 
		// "[ [[MolName]([vecMolIDs])::][Inst([InstanceName]):][[[ResName]([vecResIDs])[ChainLabel]:][vecAtomNames] ] ]" 
		// Molecule name, if presented, should be finished with doubled colon symbols '::'
		// IDs of molecule copies, if presented, should be in brackets just behind molecule name and in front of '::'. Several IDs are separated with commas ',' without any spaces or tabs
		// 
		struct CSystemElementDescription
		{
			Common::ComString				strMoleculeName;								// Name of the molecule
			std::vector<std::pair<Common::IndexType, Common::ComChar> >	vecMoleculeNumbers;	// Array of molecule numbers (indices of molecule' copies + substitution codes) (explicit definition). Empty array means that all molecules with such name are included
			Common::ComString				strInstanceName;								// Instance name
			Common::ComChar					cChainLabel;									// Chain identifier. Equals to 'UndefinedChainLabel' if not presented
			Common::ComString				strGroupName;									// Residue name or Logical group name
			std::vector<std::pair<Common::IndexType, Common::ComChar> > vecGroupNumbers;	// Array of group seq. numbers + res. insertion codes (explicit definition). Empty array means that all groups/residues with such name are included
			std::vector<Common::ComString>	vecAtomLabels;									// Array of atom labels (explicit definition). Empty array means that no atoms are included
			//std::vector<Common::IndexType> vecAtomSeqNumbers;

			CSystemElementDescription()
			{
				cChainLabel = UndefinedChainLabel;
			}

			void Clear()
			{
				strMoleculeName.clear();
				vecMoleculeNumbers.clear();
				strInstanceName.clear();
				cChainLabel = UndefinedChainLabel;
				strGroupName.clear();
				vecGroupNumbers.clear();
				vecAtomLabels.clear();
			}

			void PopulateMissingFrom(const CSystemElementDescription& element)
			{
				if ( strMoleculeName.empty() )
				{
					strMoleculeName		= element.strMoleculeName;
				}
				if (vecMoleculeNumbers.empty() )
				{
					vecMoleculeNumbers	= element.vecMoleculeNumbers;
				}
				if ( strInstanceName.empty() )
				{
					strInstanceName		= element.strInstanceName;
				}
				if ( cChainLabel == UndefinedChainLabel )
				{
					cChainLabel			= element.cChainLabel;
				}
				if (strGroupName.empty() )
				{
					strGroupName		= element.strGroupName;
				}
				if ( vecGroupNumbers.empty() )
				{
					vecGroupNumbers		= element.vecGroupNumbers;
				}
				if ( vecAtomLabels.empty() )
				{
					vecAtomLabels		= element.vecAtomLabels;
				}
			}

			bool CanBeBonded(const CSystemElementDescription& element) const
			{
				if (!element.strInstanceName.empty() && element.strInstanceName != strInstanceName)
				{
					Common::CException::Throw(__FILE__, __LINE__, _ComStr("CanBeBonded(): Definition of the 2nd bonded atom has instance name different from that of the 1st bonded atom"));
					return false;
				}
				if (!element.strMoleculeName.empty() && element.strMoleculeName != strMoleculeName)
				{
					Common::CException::Throw(__FILE__, __LINE__, _ComStr("CanBeBonded(): Definition of the 2nd bonded atom has molecule name different from that of the 1st bonded atom"));
					return false;
				}
				if (element.vecMoleculeNumbers != vecMoleculeNumbers)
				{
					Common::CException::Throw(__FILE__, __LINE__, _ComStr("CanBeBonded(): Definition of the 2nd bonded atom has molecule number(s) different from that of the 1st bonded atom"));
					return false;
				}
				if (element.cChainLabel != cChainLabel)
				{
					Common::CException::Throw(__FILE__, __LINE__, _ComStr("CanBeBonded(): Definition of the 2nd bonded atom has chain label different from that of the 1st bonded atom"));
					return false;
				}
				return true;
			}

			bool operator == (const CSystemElementDescription& element) const
			{
				return strMoleculeName		== element.strMoleculeName
					&& vecMoleculeNumbers	== element.vecMoleculeNumbers
					&& strInstanceName		== element.strInstanceName
					&& cChainLabel			== element.cChainLabel
					&& strGroupName			== element.strGroupName
					&& vecGroupNumbers		== element.vecGroupNumbers
					&& vecAtomLabels		== element.vecAtomLabels ;
			}

			bool operator != (const CSystemElementDescription& element) const
			{
				return ! operator == (element);
			}

		}; // struct CSystemElementDescription

		// Returns array of blocks of the string that are available for parsing, because they are Out Of Quotes (OOQ) i.e. outside of blocks bounded by pairwise double quotes ("..") that mean unanalyzable' part of the string
		inline void getBlocks_OOQ(const Common::ComString& strItem, std::vector<std::pair<size_t, size_t> >& vBlocks)
		{
			std::vector<std::pair<size_t, size_t> > v; //Array of blocks. Block is defined as {index_of_first_character, index_after_last_character}. Characters within blocks are alloved to be parsed/analyzed
			size_t nLen(strItem.length()), nOff(0), nPos(0), nPos2(0);
			for ( ; ; )
			{
				nPos = strItem.find(_ComStr("\""), nOff); //Position of opening quote
				if (nPos == Common::ComString::npos) //Opening quote not found
				{
					break;
				}
				nPos2 = strItem.find(_ComStr("\""), nPos + 1);
				if (nPos2 == Common::ComString::npos) //Closing quote not found
				{
					Common::CException::Throw(__FILE__, __LINE__, _ComStr("Parsing string item(): Failed to find closing quote in [sub]string of '") + strItem + _ComStr("'"));
				}
				if (nPos > nOff)
				{
					vBlocks.push_back(std::make_pair(nOff, nPos));
				}
				++nPos2;
				nOff = nPos2;
			}
			if (nLen > nPos2)
			{
				vBlocks.push_back(std::make_pair(nPos2, nLen));
			}
		}

		// Analog for std::basic_string<>::find, but searching only 'Out Of Quotes' (OOQ) i.e. skipping blocks bounded by double quotes ("..")
		inline size_t find_OOQ(const Common::ComString& strItem, const Common::ComChar* _Ptr, size_t _Off = 0)
		{
			std::vector<std::pair<size_t, size_t> > vBlocks;
			getBlocks_OOQ(strItem, vBlocks);
			size_t nPos = Common::ComString::npos;
			for (std::vector<std::pair<size_t, size_t> >::iterator it = vBlocks.begin(), itEnd = vBlocks.end(); it != itEnd; ++it)
			{
				if ((nPos = strItem.find(_Ptr, (std::max)(_Off, it->first))) != Common::ComString::npos
					&& nPos < it->second 
					)
				{
					return nPos;
				}
			}
			return Common::ComString::npos;
		}

		// Analog for std::basic_string<>::find_first_of, but searching only 'Out Of Quotes' (OOQ) i.e. skipping blocks bounded by double quotes ("..")
		inline size_t find_first_of_OOQ(const Common::ComString& strItem, const Common::ComChar* _Ptr, size_t _Off = 0)
		{
			std::vector<std::pair<size_t, size_t> > vBlocks;
			getBlocks_OOQ(strItem, vBlocks);
			size_t nPos = Common::ComString::npos;
			for (std::vector<std::pair<size_t, size_t> >::iterator it = vBlocks.begin(), itEnd = vBlocks.end(); it != itEnd; ++it)
			{
				if ((nPos = strItem.find_first_of(_Ptr, (std::max)(_Off, it->first))) != Common::ComString::npos
					&& nPos < it->second 
					)
				{
					return nPos;
				}
			}
			return Common::ComString::npos;
		}

		// Analog for std::basic_string<>::find_first_not_of, but searching only 'Out Of Quotes' (OOQ) i.e. skipping blocks bounded by double quotes ("..")
		inline size_t find_first_not_of_OOQ(const Common::ComString& strItem, const Common::ComChar* _Ptr, size_t _Off = 0)
		{
			std::vector<std::pair<size_t, size_t> > vBlocks;
			getBlocks_OOQ(strItem, vBlocks);
			size_t nPos = Common::ComString::npos;
			for (std::vector<std::pair<size_t, size_t> >::iterator it = vBlocks.begin(), itEnd = vBlocks.end(); it != itEnd; ++it)
			{
				if ((nPos = strItem.find_first_not_of(_Ptr, (std::max)(_Off, it->first))) != Common::ComString::npos
					&& nPos < it->second 
					)
				{
					return nPos;
				}
			}
			return Common::ComString::npos;
		}

		// Analog for std::basic_string<>::find_last_of, but searching only 'Out Of Quotes' (OOQ) i.e. skipping blocks bounded by double quotes ("..")
		inline size_t find_last_of_OOQ(const Common::ComString& strItem, const Common::ComChar* _Ptr, size_t _Off = Common::ComString::npos)
		{
			std::vector<std::pair<size_t, size_t> > vBlocks;
			getBlocks_OOQ(strItem, vBlocks);
			size_t nPos = Common::ComString::npos;
			for (std::vector<std::pair<size_t, size_t> >::reverse_iterator rit = vBlocks.rbegin(), ritEnd = vBlocks.rend(); rit != ritEnd; ++rit)
			{
				if ((nPos = strItem.find_last_of(_Ptr, (std::min)(_Off, rit->second - 1))) != Common::ComString::npos
					&& nPos >= rit->first
					)
				{
					return nPos;
				}
			}
			return Common::ComString::npos;
		}

		// Analog for std::basic_string<>::find_last_not_of, but searching only 'Out Of Quotes' (OOQ) i.e. skipping blocks bounded by double quotes ("..")
		inline size_t find_last_not_of_OOQ(const Common::ComString& strItem, const Common::ComChar* _Ptr, size_t _Off = Common::ComString::npos)
		{
			std::vector<std::pair<size_t, size_t> > vBlocks;
			getBlocks_OOQ(strItem, vBlocks);
			size_t nPos = Common::ComString::npos;
			for (std::vector<std::pair<size_t, size_t> >::reverse_iterator rit = vBlocks.rbegin(), ritEnd = vBlocks.rend(); rit != ritEnd; ++rit)
			{
				if ((nPos = strItem.find_last_not_of(_Ptr, (std::min)(_Off, rit->second - 1))) != Common::ComString::npos
					&& nPos >= rit->first
					)
				{
					return nPos;
				}
			}
			return Common::ComString::npos;
		}

		// Function cuts the substring describing separate item (e.g. molecular group, logical group, atom, bond, angle, torsion, ring, OOP etc.) from the string of strItems and returns it in strSubItem. 
		// Delimiter characters can be ' ' and/or '\t'. Leading and tailing delimiter(s) as well as delimiter(s) between strSubItem and the rest of strItems are also cut from the returned result. 
		// Function takes into account subblocks in double quotes ("..") that are treated as undivisible and uneditable. 
		// Within subitem, input string can have delimiting spaces and tabs just in front of or behind special symbols: ",:;-()". These delimiting spaces and tabs are eliminated from output string.
		// Function returns true if strSubItem is not empty, and false otherwise.
		inline bool getNextSubitem(Common::ComString& strItems, Common::ComString& strSubItem)
		{
			const ComChar* szSeparators = _ComStr(" \t\r\n"); // Shouldn't we include comma ',' to this list?
			const ComChar* szSpecSymbols = _ComStr(":;-()");
			size_t i, nPos = 0, nLen = strItems.length();
			strSubItem.clear();
			// Remove leading spaces and tabs
			if ( (nPos = strItems.find_first_not_of(szSeparators)) != Common::ComString::npos )
			{
				strItems = strItems.substr(nPos, nLen - nPos);
				nLen -= nPos;
			}
			i = 0;
			for ( ; ; ) //Main cycle by occurrences of separators located outside of quoted ("..") blocks
			{
				if ( (nPos = find_first_of_OOQ(strItems, szSeparators)) == Common::ComString::npos ) // Neither space nor tab found
				{
					strSubItem = strItems;
					strItems.clear();
					break;
				} // Neither space nor tab found
				else // Space or tab found
				{ // This can be either separator between subitems (so one should cut subitem and return), or separator near special symbol(s) ",:;-()" (so one should remove it and continue parsing)
					while (   nPos < nLen 
						   && strItems.find_first_of(szSeparators, nPos) == nPos // strItems[nPos] == ',', ' ' or '\t'
						   )
					{
						strItems = strItems.substr(0, nPos) + strItems.substr(nPos + 1, nLen - (nPos + 1));
						--nLen;
					}
					if (nPos == nLen)
					{
						strSubItem = strItems;
						strItems.clear();
						break;
					}
					else
					{
						if (   strItems.find_first_of(szSpecSymbols, nPos) == nPos // strItems[nPos] == any_of ":;-()"
							|| ( nPos && strItems.find_first_of(szSpecSymbols, nPos - 1) == nPos - 1 ) // strItems[nPos-1] == any_of ":;-()"
							)
						{ //Separator was in front of or behind spec symbol(s). Erase it (it is already done) and continue searching
							continue;
						}
						else // End of subitem description found
						{
							strSubItem = strItems.substr(0, nPos);
							strItems = strItems.substr(nPos, nLen - nPos);
							break;
						}
					}
					strSubItem = strItems.substr(0, nPos);
					strItems = strItems.substr(nPos, nLen - nPos);
				} // Space or tab found
			}
			//Remove tailing comma(s) from output, if any
			nLen = strSubItem.length();
			while ( nLen && strSubItem[nLen-1] == Common::ComChar(',') ) // This could make no sense if comma ',' is separator letter
			{
				--nLen;
				strSubItem = strSubItem.substr(0, nLen);
			}

			return !strSubItem.empty();
		}

		// Supposing that strElement contains description of single element o fthe system (e.g. atom, residue, logical group etc.), 
		// function parses element description and populates output 'element' parameter
		inline bool parseElementFullName(const Common::ComString& strElement, Common::Detail::CSystemElementDescription& element)
		{
			bool bRes = true;
			size_t nPos, nPos1, nPos2, nPos3;
			Common::ComString _strElement(strElement);
			element.Clear();

			// Molecule name. If presented, always [comes first and] ends with doubled colons '::'
			if (   (nPos2  = find_OOQ(_strElement, _ComStr("::"))) != Common::ComString::npos)
			{
				Common::ComString strMoleculeLabel; // Will initially contain molecule name + list of molecule numbers, if any
				if (   !nPos2 
					|| (nPos = find_last_of_OOQ(_strElement, _ComStr(":"), nPos2-1)) == Common::ComString::npos  // Position of ':' preceeding the molecule name section, if any
					)
				{
					strMoleculeLabel = _strElement.substr(0, nPos2);
					_strElement = _strElement.substr(nPos2 + 2, _strElement.length() - (nPos2 + 2));
				}
				else
				{
					strMoleculeLabel = _strElement.substr(nPos + 1, nPos2 - (nPos + 1));
					_strElement = _strElement.substr(0, nPos + 1) + _strElement.substr(nPos2 + 2, _strElement.length() - (nPos2 + 2)); // Keep ':' found at nPos
				}
				// Extract molecule number(s), if any
				nPos =  find_OOQ(strMoleculeLabel, _ComStr("("));
				nPos2 = find_OOQ(strMoleculeLabel, _ComStr(")"));
				if (   nPos  != Common::ComString::npos
					&& nPos2 != Common::ComString::npos
					&& nPos2 > nPos
					)
				{
					element.strMoleculeName = strMoleculeLabel.substr(0, nPos) + strMoleculeLabel.substr(nPos2 + 1, strMoleculeLabel.length() - (nPos2 + 1));
					strMoleculeLabel = strMoleculeLabel.substr(nPos + 1, nPos2 - (nPos + 1)); // Contents in brackets
					bool bAny = false;
					try
					{
						//bRes &= ParseListOfIntegers(strMoleculeLabel, element.vecMoleculeNumbers, bAny);
						bRes &= ParseListOfIntegerAndChars(strMoleculeLabel, element.vecMoleculeNumbers, bAny);
					}
					catch (const Common::CException& e)
					{
						const Common::ComChar* pMessage = e.Message();
						Common::CException::Throw(__FILE__, __LINE__, _ComStr("Detail::parseElementFullName(): While parsing molecule numbers in '") + (strElement + _ComStr("': ")) + pMessage);
					}
				}
				else
				{
					if (   nPos  != Common::ComString::npos
						|| nPos2 != Common::ComString::npos
						)
					{
						Common::CException::Throw(__FILE__, __LINE__, _ComStr("Detail::parseElementFullName(): Found unpaired bracket(s) in 'Molecule Name' section of '") + strElement + _ComStr("'"));
					}
					element.strMoleculeName = strMoleculeLabel;
				}
				//Remove quotes, if any, from molecule name
				for ( ; ; )
				{
					nPos  = element.strMoleculeName.find(_ComStr("\""));
					nPos2 = nPos == Common::ComString::npos ? Common::ComString::npos : element.strMoleculeName.find(_ComStr("\""), nPos + 1);
					if (   nPos == Common::ComString::npos 
						&& nPos2 == Common::ComString::npos 
						)
					{
						break;
					}
					else if (nPos != Common::ComString::npos && nPos2 != Common::ComString::npos)
					{
						element.strMoleculeName = element.strMoleculeName.substr(0, nPos) 
												+ element.strMoleculeName.substr(nPos + 1, nPos2 - (nPos + 1))
												+ element.strMoleculeName.substr(nPos2 + 1, element.strMoleculeName.length() - (nPos2 + 1));
					}
					else
					{
						Common::CException::Throw(__FILE__, __LINE__, _ComStr("Detail::parseElementFullName(): Found unpaired quote in 'Molecule Name' section of '") + strElement + _ComStr("'"));
					}
				}
			} // Molecule name

			// Instance name. If presented, always [comes first in the remaining string and] has the form of 'Inst([Instance Name])[':']
			Common::ComString strInst = _ComStr("Inst(");
			if ( (nPos1 = find_OOQ(_strElement, strInst.c_str())) != Common::ComString::npos )
			{
				if ( (nPos2 = find_OOQ(_strElement, _ComStr(")"), nPos1 + strInst.length())) == Common::ComString::npos )
				{
					Common::CException::Throw(__FILE__, __LINE__, _ComStr("Detail::parseElementFullName(): Found unpaired bracket(s) in 'Instance name' section of '") + strElement + _ComStr("'"));
				}
				if ( (nPos3 = find_OOQ(_strElement, _ComStr(":"), nPos2 + 1)) == Common::ComString::npos) // Position of ':' following the Instance section
				{
					nPos3 = nPos2;
				}
				Common::ComString strInstanceLabel = _strElement.substr(nPos1 + strInst.length(), nPos2 - (nPos1 + strInst.length())); // Will contain Instance name(s)
				if ( (nPos = find_last_of_OOQ(_strElement, _ComStr(":"), nPos1)) == Common::ComString::npos) // Position of ':' preceeding the Instance section, if any
				{
					_strElement = _strElement.substr(nPos3 + 1, _strElement.length() - (nPos3 + 1)); // Skip symbols from nPos2 to nPos3 i.e. at ')..[:]'
				}
				else
				{
					_strElement = _strElement.substr(0, nPos + 1) + _strElement.substr(nPos3 + 1, _strElement.length() - (nPos3 + 1)); // Keep ':' found at nPos, skip symbols from nPos2 to nPos3 i.e. at ')..[:]'
				}
				// Put Instance name 'as is'
				element.strInstanceName = strInstanceLabel;
				//Remove quotes, if any, from instance name
				for ( ; ; )
				{
					nPos  = element.strInstanceName.find(_ComStr("\""));
					nPos2 = nPos == Common::ComString::npos ? Common::ComString::npos : element.strInstanceName.find(_ComStr("\""), nPos + 1);
					if (   nPos == Common::ComString::npos
						&& nPos2 == Common::ComString::npos
						)
					{
						break;
					}
					else if (nPos != Common::ComString::npos
								&& nPos2 != Common::ComString::npos
									)
					{
						element.strInstanceName = element.strInstanceName.substr(0, nPos) 
												+ element.strInstanceName.substr(nPos + 1, nPos2 - (nPos + 1)) 
												+ element.strInstanceName.substr(nPos2 + 1, element.strInstanceName.length() - (nPos2 + 1));
					}
					else
					{
						Common::CException::Throw(__FILE__, __LINE__, _ComStr("Detail::parseElementFullName(): Found unpaired quote in 'Instance Name' section of '") + strElement + _ComStr("'"));
					}
				}
			} // Instance name

			// Group/residue name. If presented, always [comes first in the remaining string and] ends with colon ':' or with end-of-line
			Common::ComString strGroupLabel; // Will initially contain group/residue name + list of group numbers, if any
			nPos2  = find_OOQ(_strElement, _ComStr(":"));
			if ( nPos2 == Common::ComString::npos )
			{
				strGroupLabel = _strElement;
				_strElement.clear();
			}
			else
			{
				strGroupLabel = _strElement.substr(0, nPos2);
				_strElement = _strElement.substr(nPos2 + 1, _strElement.length() - (nPos2 + 1));
			}
			if ( !strGroupLabel.empty() )
			{
				// Extract group number(s), if any
				nPos =  find_OOQ(strGroupLabel, _ComStr("("));
				nPos2 = find_OOQ(strGroupLabel, _ComStr(")"));
				if ( nPos  != Common::ComString::npos
					&& nPos2 != Common::ComString::npos
					&& nPos2 > nPos
					)
				{
					element.strGroupName = strGroupLabel.substr(0, nPos) + strGroupLabel.substr(nPos2 + 1, strGroupLabel.length() - (nPos2 + 1));
					strGroupLabel = strGroupLabel.substr(nPos + 1, nPos2 - (nPos + 1)); // Contents in brackets
					bool bAny = false;
					try
					{
						//bRes &= ParseListOfIntegers(strGroupLabel, element.vecGroupNumbers, bAny);
						bRes &= ParseListOfIntegerAndChars(strGroupLabel, element.vecGroupNumbers, bAny);
					}
					catch (const Common::CException& e)
					{
						const Common::ComChar* pMessage = e.Message();
						Common::CException::Throw(__FILE__, __LINE__, _ComStr("Detail::parseElementFullName(): While parsing group numbers in '") + (strElement + _ComStr("': ")) + pMessage);
					}
				}
				else
				{
					if (   nPos  != Common::ComString::npos
						|| nPos2 != Common::ComString::npos
						)
					{
						Common::CException::Throw(__FILE__, __LINE__, _ComStr("Detail::parseElementFullName(): Found unpaired bracket(s) in 'Group Name' section of '") + strElement + _ComStr("'"));
					}
					element.strGroupName = strGroupLabel;
				}
				//Remove quotes, if any, from group name
				for ( ; ; )
				{
					nPos  = element.strGroupName.find(_ComStr("\""));
					nPos2 = nPos == Common::ComString::npos ? Common::ComString::npos : element.strGroupName.find(_ComStr("\""), nPos + 1);
					if (   nPos == Common::ComString::npos 
						&& nPos2 == Common::ComString::npos 
						)
					{
						break;
					}
					else if (nPos != Common::ComString::npos && nPos2 != Common::ComString::npos )
					{
						element.strGroupName	= element.strGroupName.substr(0, nPos)
												+ element.strGroupName.substr(nPos + 1, nPos2 - (nPos + 1)) 
												+ element.strGroupName.substr(nPos2 + 1, element.strGroupName.length() - (nPos2 + 1));
					}
					else
					{
						Common::CException::Throw(__FILE__, __LINE__, _ComStr("Detail::parseElementFullName(): Found unpaired quote in 'Group Name' section of '") + strElement + _ComStr("'"));
					}
				}
			} // Group/residue name

			// Atom label(s). If presented, is/are in the rest of the string
			if (!_strElement.empty())
			{
				if ( (nPos2 = find_OOQ(_strElement, _ComStr(":"))) != Common::ComString::npos )
				{
					Common::CException::Throw(__FILE__, __LINE__, _ComStr("Detail::parseElementFullName(): Found unexpected ':' separator while parsing 'Atom Label(s)' section of '") + strElement + _ComStr("'"));
				}
				Common::ComString strAtomLabel;
				// Parse atom labels list. Separators can be only ',' or ';'. Atom label can be in quotes (e.g. if it contains spec symbols); so doubled quotes in atom labels, if any, are removed
				while (!_strElement.empty())
				{
					if ( (nPos2 = find_first_of_OOQ(_strElement, _ComStr(",;"))) == Common::ComString::npos )
					{
						strAtomLabel = _strElement;
						_strElement.clear();
					}
					else
					{
						strAtomLabel = _strElement.substr(0, nPos2);
						_strElement = _strElement.substr(nPos2 + 1, _strElement.length() - (nPos2 + 1));
					}

					//Remove quotes, if any, from atom label
					for ( ; ; )
					{
						nPos  = strAtomLabel.find(_ComStr("\""));
						nPos2 = nPos == Common::ComString::npos ? Common::ComString::npos : strAtomLabel.find(_ComStr("\""), nPos + 1);
						if (   nPos == Common::ComString::npos 
							&& nPos2 == Common::ComString::npos 
							)
						{
							break;
						}
						else if (nPos != Common::ComString::npos && nPos2 != Common::ComString::npos)
						{
							strAtomLabel = strAtomLabel.substr(0, nPos)
										 + strAtomLabel.substr(nPos + 1, nPos2 - (nPos + 1))
										 + strAtomLabel.substr(nPos2 + 1, strAtomLabel.length() - (nPos2 + 1));
						}
						else
						{
							Common::CException::Throw(__FILE__, __LINE__, _ComStr("Detail::parseElementFullName(): Found unpaired quote in 'Atom Label(s)' section of '") + strElement + _ComStr("'"));
						}
					}
					if ( !strAtomLabel.empty() )
					{
						element.vecAtomLabels.push_back(strAtomLabel);
					}
				} // Cycle by atom labels
			} // Atom label(s)

			return bRes;
		}

		// Function separates input string containing description of single element (e.g. atom, residue, molecule, logical group) or sequence of elements (e.g. bond, angle, torsion, ring, OOP) 
		// on array of elements descriptions and array of separators between them. 
		// 0th separator is between 0th and 1st elements etc., so the number of separators is equal to the number of element (e.g. for rings with tailing '-'), or less than it by 1 (i.e. for all other items, including ring with duplicated 0th atom)
		//
		inline bool parseItem(const Common::ComString& strItem, std::vector<Common::Detail::CSystemElementDescription>& vecElements, std::vector<Common::ComString>& vecSeparators)
		{
			size_t nPos;
			Common::ComString strElement, strSeparator;
			Common::Detail::CSystemElementDescription element;
			vecElements.clear();
			vecSeparators.clear();
			Common::ComString _strItem(strItem);
			while ( !_strItem.empty() ) // Cycle by elements of the item
			{
				nPos = Common::Detail::find_first_of_OOQ(_strItem, _ComStr("-"));
				if (nPos == Common::ComString::npos)
				{
					strElement = _strItem;
					strSeparator.clear();
					_strItem.clear();
				}
				else
				{
					if (nPos && _strItem[nPos - 1] == Common::ComChar(';')) // OOP separator
					{
						strElement = _strItem.substr(0, nPos - 1);
						strSeparator = _strItem.substr(nPos - 1, 2);
					}
					else // 'ordinary' separator
					{
						strElement = _strItem.substr(0, nPos);
						strSeparator = _strItem.substr(nPos, 1);
					}
					_strItem = _strItem.substr(nPos + 1, _strItem.length() - (nPos + 1));
				}
				if (!parseElementFullName(strElement, element))
				{
					Common::CException::Throw(__FILE__, __LINE__, _ComStr("Detail::parseItem(): Failed to parse element of '") + strElement + _ComStr("' of the string of '") + strItem + _ComStr("'"));
				}
				//Populate missing fields of current element with that of previous element, if any
				if (!vecElements.empty())
				{
					Common::Detail::CSystemElementDescription& prevElement = vecElements.back();
					element.PopulateMissingFrom(prevElement);
					// Preliminary check the result for consistency
					if ( !element.CanBeBonded(prevElement) )
					{
						return false;
					}
				}
				vecElements.push_back(element);
				if (!strSeparator.empty())
				{
					vecSeparators.push_back(strSeparator);
				}
			} // Cycle by elements of the item
			// Special case for ring: if it contains element duplication (last element coinciding with first one, without tailing separator), replace duplicated element with tailing separator
			if ( vecElements.size() > 2
				&& vecElements.back() == vecElements[0]
				&& vecSeparators.size() + 1 == vecElements.size()
				)
			{
				vecElements.pop_back();
				vecSeparators.push_back(_ComStr("-"));
			}

			return true;
		}

		inline bool isBondDescription(const Common::ComString& strItem,
										Common::Detail::CSystemElementDescription* pElement1 = NULL,
										Common::Detail::CSystemElementDescription* pElement2 = NULL)
		{
			std::vector<Common::Detail::CSystemElementDescription> vecElements;
			std::vector<Common::ComString> vecSeparators;
			if ( !Common::Detail::parseItem(strItem, vecElements, vecSeparators)
				|| vecElements.size() != 2
				|| vecSeparators.size() != 1
				|| !( vecElements[0] != vecElements[1] )
				|| !( vecElements[0].vecAtomLabels.size() == 1 && vecElements[1].vecAtomLabels.size() == 1 )
				|| !( vecSeparators[0] == _ComStr("-") )
				)
			{
				return false;
			}
			if (pElement1)
			{
				*pElement1 = vecElements[0];
			}
			if (pElement2)
			{
				*pElement2 = vecElements[1];
			}
			return true;
		}


		inline bool isAngleDescription(const Common::ComString& strItem,
										Common::Detail::CSystemElementDescription* pElement1 = NULL,
										Common::Detail::CSystemElementDescription* pElement2 = NULL,
										Common::Detail::CSystemElementDescription* pElement3 = NULL)
		{
			std::vector<Common::Detail::CSystemElementDescription> vecElements;
			std::vector<Common::ComString> vecSeparators;
			if (  !Common::Detail::parseItem(strItem, vecElements, vecSeparators)
				|| vecElements.size() != 3
				|| vecSeparators.size() != 2
				|| !( vecElements[0] != vecElements[1] && vecElements[0] != vecElements[2] && vecElements[1] != vecElements[2] )
				|| !( vecElements[0].vecAtomLabels.size() == 1 && vecElements[1].vecAtomLabels.size() == 1 && vecElements[2].vecAtomLabels.size() == 1 )
				|| !( vecSeparators[0] == _ComStr("-") && vecSeparators[1] == _ComStr("-") )
				)
			{
				return false;
			}
			if (pElement1)
			{
				*pElement1 = vecElements[0];
			}
			if (pElement2)
			{
				*pElement2 = vecElements[1];
			}
			if (pElement3)
			{
				*pElement3 = vecElements[2];
			}
			return true;
		}

		inline bool isTorsionDescription(const Common::ComString& strItem,
										Common::Detail::CSystemElementDescription* pElement1 = NULL,
										Common::Detail::CSystemElementDescription* pElement2 = NULL,
										Common::Detail::CSystemElementDescription* pElement3 = NULL,
										Common::Detail::CSystemElementDescription* pElement4 = NULL)
		{
			std::vector<Common::Detail::CSystemElementDescription> vecElements;
			std::vector<Common::ComString> vecSeparators;
			if (  !Common::Detail::parseItem(strItem, vecElements, vecSeparators)
				|| vecElements.size() != 4
				|| vecSeparators.size() != 3
				|| !( vecElements[0] != vecElements[1] && vecElements[0] != vecElements[2] && vecElements[0] != vecElements[3] && vecElements[1] != vecElements[2] && vecElements[1] != vecElements[3] && vecElements[2] != vecElements[3] )
				|| !( vecElements[0].vecAtomLabels.size() == 1 && vecElements[1].vecAtomLabels.size() == 1 && vecElements[2].vecAtomLabels.size() == 1 && vecElements[3].vecAtomLabels.size() == 1 )
				|| !( vecSeparators[0] == _ComStr("-") && vecSeparators[1] == _ComStr("-") && vecSeparators[2] == _ComStr("-") )
				)
			{
				return false;
			}
			if (pElement1)
			{
				*pElement1 = vecElements[0];
			}
			if (pElement2)
			{
				*pElement2 = vecElements[1];
			}
			if (pElement3)
			{
				*pElement3 = vecElements[2];
			}
			if (pElement4)
			{
				*pElement4 = vecElements[3];
			}
			return true;
		}

		inline bool isOutOfPlaneDescription(const Common::ComString& strItem,
										Common::Detail::CSystemElementDescription* pElement1 = NULL,
										Common::Detail::CSystemElementDescription* pElement2 = NULL,
										Common::Detail::CSystemElementDescription* pElement3 = NULL,
										Common::Detail::CSystemElementDescription* pElement4 = NULL)
		{
			std::vector<Common::Detail::CSystemElementDescription> vecElements;
			std::vector<Common::ComString> vecSeparators;
			if ( !Common::Detail::parseItem(strItem, vecElements, vecSeparators)
				|| vecElements.size() != 4
				|| vecSeparators.size() != 3
				|| !( vecElements[0] != vecElements[1] && vecElements[0] != vecElements[2] && vecElements[0] != vecElements[3] && vecElements[1] != vecElements[2] && vecElements[1] != vecElements[3] && vecElements[2] != vecElements[3] )
				|| !( vecElements[0].vecAtomLabels.size() == 1 && vecElements[1].vecAtomLabels.size() == 1 && vecElements[2].vecAtomLabels.size() == 1 && vecElements[3].vecAtomLabels.size() == 1 )
				|| !( vecSeparators[0] == _ComStr(";-") && vecSeparators[1] == _ComStr(";-") && vecSeparators[2] == _ComStr(";-") )
				)
			{
				return false;
			}
			if (pElement1)
			{
				*pElement1 = vecElements[0];
			}
			if (pElement2)
			{
				*pElement2 = vecElements[1];
			}
			if (pElement3)
			{
				*pElement3 = vecElements[2];
			}
			if (pElement4)
			{
				*pElement4 = vecElements[3];
			}
			return true;
		}

		inline bool isRingDescription(const Common::ComString& strItem,
										std::vector<Common::Detail::CSystemElementDescription>* pvecElements = NULL)
		{
			std::vector<Common::Detail::CSystemElementDescription> vecElements;
			std::vector<Common::ComString> vecSeparators;
			if ( !Common::Detail::parseItem(strItem, vecElements, vecSeparators) )
			{
				return false;
			}
			size_t i, j, nElements = vecElements.size(), nSeparators = vecSeparators.size();
			if (nElements < 3 || nSeparators != nElements) //If in strItem there was correct Ring description with identity of first and last atoms, it was replaced by additional '-' separator
			{
				return false;
			}
			for ( i = 0; i < nElements; ++i )
			{
				for ( j = i + 1; j < nElements; ++j )
				{
					if ( vecElements[i] == vecElements[j] )
					{
						return false;
					}
				}
				if ( vecElements[i].vecAtomLabels.size() != 1 )
				{
					return false;
				}
				if ( vecSeparators[i] != _ComStr("-") )
				{
					return false;
				}
			}
			if (pvecElements)
			{
				*pvecElements = vecElements;
			}
			return true;
		}

	} //namespace Detail

} //namespace Common
