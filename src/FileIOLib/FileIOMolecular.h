#pragma once

#include "FileIOBase.h"
#include "IOMolecule.h"
#include "CommonLib/LiteralDefsParsing.h"

#include <vector>
#include <memory>


namespace FileIO
{ //namespace FileIO

	template <typename _Molecule = CIOMolecule, typename _ContainerMol = std::vector<std::shared_ptr<_Molecule> >, typename _ContainerComments = std::vector<Common::ComString>, typename _ContainerPendingKeylines = std::vector<Common::ComString> >
	class CFileIOMolecularT : public CFileIOBase
	{
	public:
		CFileIOMolecularT()
		{
		}
		virtual ~CFileIOMolecularT()
		{
		}
	public:

		virtual bool ReadContents(const Common::ComString& strFileName)
		{
			return ReadContents(strFileName, m_contMolecules, m_contComments, m_contPendingKeylines);
		}

		virtual bool ReadContents(const Common::ComString& strFileName,
								_ContainerMol& contMolecules,
								_ContainerComments& contComments,
								_ContainerPendingKeylines& contPendingKeylines = _ContainerPendingKeylines())
		{
			std::basic_stringstream<Common::ComChar> ss;
			bool bRes = ReadFileStream(strFileName, ss);
			if (bRes)
			{
				bRes &= ReadContents(ss, contMolecules, contComments, contPendingKeylines);
			}
			return bRes;
		}

		virtual bool ReadContents(std::basic_istream<Common::ComChar>& is,
								_ContainerMol& contMolecules,
								_ContainerComments& contComments,
								_ContainerPendingKeylines& contPendingKeylines = _ContainerPendingKeylines())
		{
			if (is.bad())
			{
				return false;
			}
			bool bRes = true;
			size_t nOldSize = contMolecules.size();
			std::insert_iterator<_ContainerMol> itret(contMolecules, contMolecules.end());
			for ( ; ; )
			{
				typename _ContainerMol::value_type mol;
				createMolecule(mol);
				bool bSubRes = ReadMolecule(is, getMolecule(mol), contComments, contPendingKeylines);
				if (!getMolecule(mol).IsUndefined())
				{
					*itret++ = mol;
				}
				if (is.eof())
				{
					break;
				}
				bRes &= bSubRes;
			}

			//Apply pending lines with keywords. Do this only for newly inserted molecules
			if (!contMolecules.empty() && !contPendingKeylines.empty())
			{
				typename _ContainerMol::iterator it = contMolecules.begin();
					for (size_t i = 0; i < nOldSize; ++i, ++it) ; //make 'it' pointing on 'contMolecules.begin() + nOldSize'
				for ( ; it != contMolecules.end(); ++it)
				{
					bRes &= ApplyKeywords(getMolecule(*it), contPendingKeylines);
				}
			}

			//Extract list of molecular groups from [own] list of comments and apply it to molecules
			std::vector<Common::ComString> strMolecularGroups;
			ExtractMolecularGroupNames(contMolecules, contComments, strMolecularGroups);

			return bRes;
		}

		// Analog for ReadContents() but non-virtual and allowing another type of container(s) to be populated
		template <typename _ContainerMol2, typename _ContainerComments2, typename _ContainerPendingKeylines2>
		bool ReadContentsEx(const Common::ComString& strFileName,
								_ContainerMol2& contMolecules,
								_ContainerComments2& contComments,
								_ContainerPendingKeylines2& contPendingKeylines = _ContainerPendingKeylines())
		{
			std::basic_stringstream<Common::ComChar> ss;
			bool bRes = ReadFileStream(strFileName, ss);
			if (bRes)
			{
				bRes &= ReadContentsEx(ss, contMolecules, contComments, contPendingKeylines);
			}
			return bRes;
		}

		// Analog for ReadContents() but non-virtual and allowing another type of container(s) to be populated
		template <typename _ContainerMol2, typename _ContainerComments2, typename _ContainerPendingKeylines2>
		bool ReadContentsEx(std::basic_istream<Common::ComChar>& is, 
								_ContainerMol2& contMolecules, 
								_ContainerComments2& contComments, 
								_ContainerPendingKeylines2& contPendingKeylines = _ContainerPendingKeylines())
		{
			if (is.bad())
			{
				return false;
			}
#if 1
	// Perform custom cycle by molecules. This could be inappropriate if serialization should involve all molecules together rather than be per-molecule pick-ups.
			bool bRes = true;
			size_t nOldSize = contMolecules.size();
			_ContainerComments contCommentsTmp;
			_ContainerPendingKeylines contPendingKeylinesTmp;
			std::insert_iterator<_ContainerMol2> itret(contMolecules, contMolecules.end());
			for ( ; ; )
			{
				typename _ContainerMol2::value_type mol;
				createMolecule(mol);
				bool bSubRes = ReadMolecule(is, getMolecule(mol), contCommentsTmp, contPendingKeylinesTmp);
				if (!getMolecule(mol).IsUndefined())
				{
					*itret++ = mol;
				}
				if (is.eof())
				{
					break;
				}
				bRes &= bSubRes;
			}
			//Apply pending lines with keywords. Do this only for newly inserted molecules
			if (!contMolecules.empty() && !contPendingKeylinesTmp.empty())
			{
				typename _ContainerMol2::iterator it = contMolecules.begin(); 
					for (size_t i = 0; i < nOldSize; ++i, ++it) ; //make 'it' pointing on 'contMolecules.begin() + nOldSize'
				for ( ; it != contMolecules.end(); ++it)
				{
					bRes &= ApplyKeywords(getMolecule(*it), contPendingKeylinesTmp);
				}
			}
			//Copy comments and keylines from temporary storages to the destination ones
			std::insert_iterator<_ContainerComments2> itretComments(contComments, contComments.end());
			for (typename _ContainerComments::const_iterator itComments = contCommentsTmp.begin(), itCommentsEnd = contCommentsTmp.end(); itComments != itCommentsEnd; ++itComments)
			{
				*itretComments++ = *itComments;
			}
			std::insert_iterator<_ContainerComments2> itretKeylines(contPendingKeylines, contPendingKeylines.end());
			for (typename _ContainerPendingKeylines::const_iterator itKeylines = contPendingKeylinesTmp.begin(), itKeylinesEnd = contPendingKeylinesTmp.end(); itKeylines != itKeylinesEnd; ++itKeylines)
			{
				*itretKeylines++ = *itKeylines;
			}
			return bRes;
#else
	// Create on-fly auxiliary object of file IO handler and call its 'standard' serialization function
			CFileIOMolecularT<CIOMolecule, _ContainerMol2, _ContainerComments2, _ContainerPendingKeylines2> fileIO;
			return fileIO.ReadContents(os, contMolecules, contComments, contPendingKeylines);
#endif
		}

		virtual bool ReadMolecule (std::basic_istream<Common::ComChar>& is,
								_Molecule& mol, //Molecule structure to be populated
								_ContainerComments& contComments, //List of comments to be populated. Comments are detected only in the lines range from current input stream position to the end of molecule/stream
								_ContainerPendingKeylines& contPendingKeylines //List of lines starting from HIN keywords that are not handled within this function, or are related to the whole file. 
								) = 0;

		virtual bool ApplyKeywords(_Molecule& mol, const _ContainerPendingKeylines& contPendingKeylines) = 0;


		virtual bool WriteContents(const Common::ComString& strFileName)
		{
			return WriteContents(strFileName, m_contMolecules, m_contComments, m_contPendingKeylines);
		}

		virtual bool WriteContents(const Common::ComString& strFileName,
								const _ContainerMol& contMolecules,
								const _ContainerComments& contComments,
								_ContainerPendingKeylines& contPendingKeylines = _ContainerPendingKeylines())
		{
			std::basic_stringstream<Common::ComChar> ss;
			bool bRes = WriteContents(ss, contMolecules, contComments, contPendingKeylines);
			if (bRes)
			{
				bRes &= WriteFileStream(strFileName, ss);
			}
			return bRes;
		}

		virtual bool WriteContents(std::basic_ostream<Common::ComChar>& os,
								const _ContainerMol& contMolecules,
								const _ContainerComments& contComments,
								_ContainerPendingKeylines& contPendingKeylines = _ContainerPendingKeylines())
		{
			if (os.bad())
			{
				return false;
			}
			bool bRes = true;
			//Write comments and keylines
			bRes &= WriteKeylines(os, contPendingKeylines);
			bRes &= WriteComments(os, contComments);
			//Write molecule(s)
			for (typename _ContainerMol::const_iterator it = contMolecules.begin(), itEnd = contMolecules.end(); it != itEnd; ++it)
			{
				bool bSubres = WriteMolecule(os, getMolecule(*it));
				bRes &= bSubres;
			}
			return bRes;
		}

		// Analog for WriteContents() but non-virtual and allowing another type of container to be serialized to the stream
		template <typename _ContainerMol2, typename _ContainerComments2, typename _ContainerPendingKeylines2>
		bool WriteContentsEx(const Common::ComString& strFileName,
								const _ContainerMol2& contMolecules,
								const _ContainerComments2& contComments,
								_ContainerPendingKeylines2& contPendingKeylines = _ContainerPendingKeylines())
		{
			std::basic_stringstream<Common::ComChar> ss;
			bool bRes = WriteContentsEx(ss, contMolecules, contComments, contPendingKeylines);
			if (bRes)
			{
				bRes &= WriteFileStream(strFileName, ss);
			}
			return bRes;
		}

		// Analog for WriteContents() but non-virtual and allowing another type of container to be serialized to the stream
		template <typename _ContainerMol2, typename _ContainerComments2, typename _ContainerPendingKeylines2>
		bool WriteContentsEx(std::basic_ostream<Common::ComChar>& os,
								const _ContainerMol2& contMolecules,
								const _ContainerComments2& contComments,
								_ContainerPendingKeylines2& contPendingKeylines = _ContainerPendingKeylines())
		{
			if (os.bad())
			{
				return false;
			}
#if 1
	// Perform custom cycle by molecules. This could be inappropriate if serialization should involve all molecules together rather than be per-molecule one.
			bool bRes = true;

			//Copy comments and keylines to buffer containers of 'native' format
			_ContainerComments contCommentsTmp;
			std::insert_iterator<_ContainerComments> itretComments(contCommentsTmp, contCommentsTmp.end());
			for (typename _ContainerComments2::const_iterator itComments = contComments.begin(), itCommentsEnd = contComments.end(); itComments != itCommentsEnd; ++itComments)
			{
				*itretComments++ = *itComments;
			}
			_ContainerPendingKeylines contPendingKeylinesTmp;
			std::insert_iterator<_ContainerComments> itretKeylines(contPendingKeylinesTmp, contPendingKeylinesTmp.end());
			for (typename _ContainerPendingKeylines2::const_iterator itKeylines = contPendingKeylines.begin(), itKeylinesEnd = contPendingKeylines.end(); itKeylines != itKeylinesEnd; ++itKeylines)
			{
				*itretKeylines++ = *itKeylines;
			}
			//Write comments and keylines
			bRes &= WriteKeylines(os, contPendingKeylines);
			bRes &= WriteComments(os, contComments);
			//Write molecule(s)
			for (typename _ContainerMol2::const_iterator it = contMolecules.begin(), itEnd = contMolecules.end(); it != itEnd; ++it)
			{
				bool bSubres = WriteMolecule(os, getMolecule(*it));
				bRes &= bSubres;
			}
			return bRes;
#else
	// Create on-fly auxiliary object of file IO handler and call its 'standard' serialization function
			CFileIOMolecularT<CIOMolecule, _ContainerMol2, _ContainerComments2, _ContainerPendingKeylines2> fileIO;
			return fileIO.WriteContents(os, contMolecules, contComments, contPendingKeylines);
#endif
		}

		virtual bool WriteMolecule(std::basic_ostream<Common::ComChar>& os,
									const _Molecule& mol, //Molecule structure to be serialized
									const _ContainerComments& contComments = _ContainerComments(), //List of comments to be serialized
									const _ContainerPendingKeylines& contPendingKeylines = _ContainerPendingKeylines() //List of lines with keywords to be serialized
									) = 0;

		virtual bool WriteComments(std::basic_ostream<Common::ComChar>& os,
									const _ContainerComments& contComments //List of comments to be serialized
									) = 0;

		virtual bool WriteKeylines(std::basic_ostream<Common::ComChar>& os,
									const _ContainerPendingKeylines& contPendingKeylines //List of lines with keywords to be serialized
									) = 0;

		_ContainerMol& GetMolecules()
		{
			return m_contMolecules;
		}

		const _ContainerMol& GetMolecules() const
		{
			return m_contMolecules;
		}

		template <typename _ContainerMol2>
		void GetMolecules(_ContainerMol2& contMoleculesOut) const
		{	//Function populates the provided container with all its molecules
			GetMolecules(m_contMolecules, contMoleculesOut);
		}

		template <typename _ContainerMol2, typename _ContainerMol3>
		void GetMolecules(const _ContainerMol2& contMoleculesIn, _ContainerMol3& contMoleculesOut) const
		{	//Function populates the provided container with all its molecules
			std::insert_iterator<_ContainerMol3> itret(contMoleculesOut, contMoleculesOut.end());
			for (typename _ContainerMol2::const_iterator itcont = contMoleculesIn.begin(), itcontEnd = contMoleculesIn.end(); itcont != itcontEnd; ++itcont)
			{
				//*itret++ = *itcont;
				//getCopy(*itret++, *itcont);
				typename _ContainerMol2::value_type molTmp;
				getCopy(molTmp, *itcont);
				*itret++ = molTmp;
			}
		}

		template <typename _ContainerMol2>
		void GetMolecules(const Common::ComString& strMolName, _ContainerMol2& contMoleculesOut) const
		{	//Function populates the provided container with all of its molecules that have the specified name
			GetMolecules(m_contMolecules, strMolName, contMoleculesOut);
		}

		template <typename _ContainerMol2, typename _ContainerMol3>
		void GetMolecules(const _ContainerMol2& contMoleculesIn, const Common::ComString& strMolName, _ContainerMol3& contMoleculesOut) const
		{	//Function populates the provided container with all of its molecules that have the specified name
			std::insert_iterator<_ContainerMol3> itret(contMoleculesOut, contMoleculesOut.end());
			for (typename _ContainerMol2::const_iterator itcont = contMoleculesIn.begin(), itcontEnd = contMoleculesIn.end(); itcont != itcontEnd; ++itcont)
			{
				if (getMolecule(*itcont).GetName() == strMolName)
				{
					//*itret++ = *itcont;
					//getCopy(*itret++, *itcont);
					typename _ContainerMol2::value_type molTmp;
					getCopy(molTmp, *itcont);
					*itret++ = molTmp;
				}
			}
		}

		template <typename _ContainerMol2>
		void GetMoleculesFromMolecularGroup(const Common::ComString& strMolGroupName, _ContainerMol2& contMoleculesOut) const
		{	//Function populates the provided container with all of its molecules that are members of molecular group with specified name
			GetMoleculesFromMolecularGroup(m_contMolecules, strMolGroupName, contMoleculesOut);
		}

		template <typename _ContainerMol2, typename _ContainerMol3>
		void GetMoleculesFromMolecularGroup(const _ContainerMol2& contMoleculesIn, const Common::ComString& strMolGroupName, _ContainerMol3& contMoleculesOut) const
		{	//Function populates the provided container with all of its molecules that are members of molecular group with specified name
			std::insert_iterator<_ContainerMol3> itret(contMoleculesOut, contMoleculesOut.end());
			for (typename _ContainerMol2::const_iterator itcont = contMoleculesIn.begin(), itcontEnd = contMoleculesIn.end(); itcont != itcontEnd; ++itcont)
			{
				const std::vector<Common::ComString>& vMmolGroups = getMolecule((*itcont)).GetContainingMolGroups();
				if (std::find(vMmolGroups.begin(), vMmolGroups.end(), strMolGroupName) != vMmolGroups.end())
				{
					//*itret++ = *itcont;
					//getCopy(*itret++, *itcont);
					typename _ContainerMol2::value_type molTmp;
					getCopy(molTmp, *itcont);
					*itret++ = molTmp;
				}
			}
		}

		template <typename _ContainerMol2>
		void GetMoleculesByNameFromMolecularGroup(const Common::ComString& strMolName, const Common::ComString& strMolGroupName, _ContainerMol2& contMoleculesOut) const
		{	//Function populates the provided container with all of its molecules that have the specified name and are members of molecular group with specified name
			GetMoleculesByNameFromMolecularGroup(m_contMolecules, strMolName, strMolGroupName, contMoleculesOut);
		}

		template <typename _ContainerMol2, typename _ContainerMol3>
		void GetMoleculesByNameFromMolecularGroup(const _ContainerMol2& contMoleculesIn, const Common::ComString& strMolName, const Common::ComString& strMolGroupName, _ContainerMol3& contMoleculesOut) const
		{	//Function populates the provided container with all of its molecules that have the specified name and are members of molecular group with specified name
			std::insert_iterator<_ContainerMol3> itret(contMoleculesOut, contMoleculesOut.end());
			for (typename _ContainerMol2::const_iterator itcont = contMoleculesIn.begin(), itcontEnd = contMoleculesIn.end(); itcont != itcontEnd; ++itcont)
			{
				if (getMolecule(*itcont).GetName() == strMolName)
				{
					const std::vector<Common::ComString>& vMmolGroups = getMolecule((*itcont)).GetContainingMolGroups();
					if (std::find(vMmolGroups.begin(), vMmolGroups.end(), strMolGroupName) != vMmolGroups.end())
					{
						//*itret++ = *itcont;
						//getCopy(*itret++, *itcont);
						typename _ContainerMol2::value_type molTmp;
						getCopy(molTmp, *itcont);
						*itret++ = molTmp;
					}
				}
			}
		}

		const _ContainerComments& GetComments() const
		{
			return m_contComments;
		}

		template <typename _ContainerMol2, typename _ContainerComments2, typename _ContainerNames>
		void ExtractMolecularGroupNames(_ContainerMol2& contMolecules, 
										const _ContainerComments2& contComments,
										_ContainerNames& contNames)
		{	//Function extracts from comments lines the list of referenced molecular groups (by tag of 'MOLGROUPDEF')
			//and simultaneously populates, in provided molecules, their own lists of molecular groups they belong to
			const Common::ComString strKeyword = _ComStr("MOLGROUPDEF");
			size_t nPos;
			std::vector<Common::ComString> vecLines; //Buffer array of comments lines with keyword of strKeyword
			Common::ComString strMolGroupName, strBuffer, strMolName, strNumber;
			std::map<Common::ComString, std::map<std::pair<Common::IndexType, Common::ComChar>, std::vector<Common::ComString> > > mapGroupNames; //Main data container. Key is molecule name, value is map of (molecule_number+subst_code + vector of group_names)
			//Look through the array of own comments lines 
			for (auto itCont = contComments.begin(), itContEnd = contComments.end(); itCont != itContEnd; ++itCont)
			{
				if ((nPos = (*itCont).find(strKeyword)) != Common::ComString::npos)
				{
					vecLines.push_back((*itCont).substr(nPos + strKeyword.length(), (*itCont).length() - (nPos + strKeyword.length()) ) );
				}
			}
			//Look through the list of comments of molecules (while loading molecules, comments could be put into them)
			for (auto itMol = contMolecules.begin(), itMolEnd = contMolecules.end(); itMol != itMolEnd; ++itMol)
			{
				for (auto itCont = getMolecule((*itMol)).GetComments().begin(), itContEnd = getMolecule((*itMol)).GetComments().end(); itCont != itContEnd; ++itCont)
				{
					if ((nPos = (*itCont).find(strKeyword)) != Common::ComString::npos)
					{
						vecLines.push_back((*itCont).substr(nPos + strKeyword.length(), (*itCont).length() - (nPos + strKeyword.length()) ) );
					}
				}
			}
			//Handle the list
			std::sort(vecLines.begin(), vecLines.end());
			vecLines.resize(std::distance(vecLines.begin(), std::unique(vecLines.begin(), vecLines.end()) ) );
			for (auto itvecLines = vecLines.begin(), itvecLinesEnd = vecLines.end(); itvecLines != itvecLinesEnd; ++itvecLines)
			{
				// Use 'universal' functionality for parsing
				strMolGroupName = _ComStr("");
				Common::ComString strComment = *itvecLines, strItem;

				if ( !Common::Detail::getNextSubitem(strComment, strItem) )  //Read molecular group title
				{
					//logger(Common::eError).Log(_ComStr("Found molecular group with no title"));
					Common::CException::Throw(__FILE__, __LINE__, _ComStr("CFileIOMolecular::ExtractMolecularGroupNames(): Found molecular group with no title"));
					continue;
				}
				{ // Parse molecular group title
					//Check for whether the logical group title is compuunded, i.e. includes Molecule/Instance/... specification
					std::vector<Common::Detail::CSystemElementDescription> vecElements;
					std::vector<Common::ComString> vecSeparators;
					if ( !Common::Detail::parseItem(strItem, vecElements, vecSeparators)
						|| !( vecElements.size() == 1 && vecSeparators.size() == 0 ) //Only molecule/Instance/residue(s) are expected; no bonds/angles/torsions/OOPs/rings
						)
					{
						Common::CException::Throw(__FILE__, __LINE__, _ComStr("CFileIOMolecular::ExtractMolecularGroupNames(): Wrong format of logical group title of '%s'"), strItem.c_str());
					}
					Common::Detail::CSystemElementDescription& element = vecElements[0];
						//Skip handling of 'Instance' info, if any...
					strItem = ( element.strMoleculeName.empty() && element.strGroupName.empty() && element.vecAtomLabels.size() == 1 ) //Molecule name was with e.g. single ':' and was wrongly treated as group name
						? element.vecAtomLabels[0]
						: ( element.strGroupName.empty() 
							? ( element.strMoleculeName.empty() ? _ComStr("<UnknownMolecularGroupName>") : element.strMoleculeName ) //Check only for 'molecule name' field, do not try to treat instance name
							: element.strGroupName // Molecular group name
							);
				} // Parse molecular group title
				strMolGroupName = strItem;
				//Put found molecular group name into output array
				if (std::find(contNames.begin(), contNames.end(), strMolGroupName) == contNames.end())
				{
					contNames.push_back(strMolGroupName);
				}
				while ( Common::Detail::getNextSubitem(strComment, strItem) )
				{ //Cycle by molecules participating in current molecular group
					std::vector<Common::Detail::CSystemElementDescription> vecElements;
					std::vector<Common::ComString> vecSeparators;
					if ( !Common::Detail::parseItem(strItem, vecElements, vecSeparators)
						|| !( vecElements.size() == 1 && vecSeparators.size() == 0 ) //Only molecules are expected; no combined elements like bonds/angles/torsions/OOPs/rings
						)
					{
						Common::CException::Throw(__FILE__, __LINE__, _ComStr("CFileIOMolecular::ExtractMolecularGroupNames(): Wrong format of element of '%s'"), strItem.c_str());
					}
					Common::Detail::CSystemElementDescription& element = vecElements[0];
					if (element.strMoleculeName.empty() && element.vecMoleculeNumbers.empty()) //Probably data was put to Group name/numbers instead. Swap it
					{
						if (element.strGroupName.empty())
						{
							if (element.vecAtomLabels.size() == 1) //Treat the value form 'atom label' entity as molecule name with empty list of molecule numbers
							{
								std::swap(element.strMoleculeName, element.vecAtomLabels.front());
								element.vecAtomLabels.clear();
							}
							else
							{
								Common::CException::Throw(__FILE__, __LINE__, _ComStr("CFileIOMolecular::ExtractMolecularGroupNames(): Element of '%s' seems to have both molecule name and group name empty"), strItem.c_str());
							}
						}
						else
						{
							std::swap(element.strMoleculeName, element.strGroupName);
							element.vecMoleculeNumbers.swap(element.vecGroupNumbers);
						}
					}
					strMolName = element.strMoleculeName;
					if (element.vecMoleculeNumbers.empty()) //Molecule number(s) are not specified. Treat it as all molecules with given name
					{
						_ContainerMol vecMol;
						GetMolecules(strMolName, vecMol);
						for (auto itMol = vecMol.begin(), itMolEnd = vecMol.end(); itMol != itMolEnd; ++itMol)
						{
							std::pair<Common::IndexType, Common::ComChar> pairIdSubst = std::make_pair((*itMol)->GetID(), UndefinedMolSubstitutionCode);
							element.vecMoleculeNumbers.push_back(pairIdSubst);
						}
					}
					// Populate map with found info
					for (auto itvec = element.vecMoleculeNumbers.begin(), itvecEnd = element.vecMoleculeNumbers.end(); itvec != itvecEnd; ++itvec)
					{
						mapGroupNames[strMolName][*itvec].push_back(strMolGroupName); //On-fly inserts new objects into container if necessary
					}
				}
			}
			//Put found molecular group names into output container and into molecules themselves
			for (auto itmapmap = mapGroupNames.begin(), itmapmapEnd = mapGroupNames.end(); itmapmap != itmapmapEnd; ++itmapmap)
			{
				strMolName = itmapmap->first;
				std::map<std::pair<Common::IndexType, Common::ComChar>, std::vector<Common::ComString> >& mapNames = itmapmap->second;
				typename std::map<std::pair<Common::IndexType, Common::ComChar>, std::vector<Common::ComString> >::iterator itmap;
				_ContainerMol2 vecMol;
				GetMolecules(contMolecules, strMolName, vecMol);
				for (auto itMol = vecMol.begin(), itMolEnd = vecMol.end(); itMol != itMolEnd; ++itMol)
				{
					std::pair<Common::IndexType, Common::ComChar> pairIdSubst = std::make_pair( (*itMol)->GetID(), UndefinedMolSubstitutionCode ); // ID of molecule plus its 'substitution code'
					if ((itmap = mapNames.find(pairIdSubst)) == mapNames.end())
					{
						Common::CException::Throw(__FILE__, __LINE__, _ComStr("ExtractMolecularGroupNames(): failed to find referenced molecule '%s' number=%d among owned molecules"),
							strMolName.c_str(), pairIdSubst.first);
					}
					else //Specified molecule is found. Put [newly found] group names into its array of containing groups (and remove duplicates, if any)
					{
						std::vector<Common::ComString>& refvecMolGroups = (*itMol)->GetContainingMolGroups();
						refvecMolGroups.insert(refvecMolGroups.end(), itmap->second.begin(), itmap->second.end());
						std::sort(refvecMolGroups.begin(), refvecMolGroups.end());
						refvecMolGroups.resize(std::distance(refvecMolGroups.begin(), std::unique(refvecMolGroups.begin(), refvecMolGroups.end()) ) );
					}
				}
			}
			//return true;
		}

	protected:
		template <typename _Container, typename _LinePrefix>
		bool writeLines(std::basic_ostream<Common::ComChar>& os, const _Container& contLines, const _LinePrefix& prefix)
		{
			for (typename _Container::const_iterator it = contLines.begin(), itEnd = contLines.end(); it != itEnd; ++it)
			{
				os	<< prefix
					<< *it
					<< std::endl
					;
			}
			return true;
		}

	protected:
		_Molecule& getMolecule(_Molecule& mol) { return mol; }
		const _Molecule& getMolecule(const _Molecule& mol) const { return mol; }
		_Molecule& getMolecule(std::shared_ptr<_Molecule> spMol) { return *spMol.get(); }
		const _Molecule& getMolecule(const std::shared_ptr<_Molecule>& spMol) const { return *spMol.get(); }

		void createMolecule(_Molecule& mol) {}
		void createMolecule(std::shared_ptr<_Molecule>& spMol) { spMol = std::shared_ptr<_Molecule>(new _Molecule()); }

		void getCopy(_Molecule& molDest, const _Molecule& molSrc) const { molDest = molSrc; }
		void getCopy(_Molecule& molDest, const std::shared_ptr<_Molecule>& spMolSrc) const { molDest = *spMolSrc.get(); }
		void getCopy(std::shared_ptr<_Molecule>& spMolDest, const _Molecule& molSrc) const { spMolDest = std::shared_ptr<_Molecule>(new _Molecule(molSrc)); }
		void getCopy(std::shared_ptr<_Molecule>& spMolDest, const std::shared_ptr<_Molecule>& spMolSrc) const { spMolDest = spMolSrc; } //Copies only pointers; [shared] molecule object stays the same

	protected:
		_ContainerComments m_contComments;
		_ContainerPendingKeylines m_contPendingKeylines;
		_ContainerMol m_contMolecules;
	};
	typedef CFileIOMolecularT<CIOMolecule, std::vector<std::shared_ptr<CIOMolecule> >, std::vector<Common::ComString>, std::vector<Common::ComString> > CFileIOMolecular;

} //namespace FileIO
