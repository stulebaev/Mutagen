#pragma once

#include "FileIOMolecular.h"
#include "CommonLib/Logger.h"
#include "CommonLib/PeriodicTable.h"

#include <map>
#include <sstream>

#ifdef _MSC_VER
	#pragma warning(disable : 4503)
#endif

namespace FileIO
{ //namespace FileIO

	//General function that populates molecule structure from 'stream' containing serialized data in HIN file format.
	//There can be the following types of input 'stream' and 'streampos':
	// - '_Stream' is std::basic_istream<Common::ComChar> (and derived classes), '_Streampos' is in this case just a constant like std::ios::cur
	// - '_Stream' is std::vector<Common::ComString>, '_Streampos' is std::vector<Common::ComString>::iterator (iterator to this container)
	// - '_Stream' is std::list<Common::ComString>, '_Streampos' is std::list<Common::ComString>::iterator (iterator to this container)
	template <typename _Stream, typename _Streampos, /*typename _Atom,*/ typename _Molecule, typename _IteratorComments, typename _IteratorKeylines>
	bool LoadMoleculeFromHINStream(_Stream& is,
								_Streampos pos,
								_Molecule& mol, //Molecule structure to be populated
								_IteratorComments itComments, //Back insert iterator for the container of comments to be populated. Comments are detected only in the lines range from current input stream position to the end of molecule/stream
								_IteratorKeylines itKeylines //Back insert iterator for the container of file lines starting from HIN keywords that are not handled within this function, or are related to the whole file. 
								)
	{
		typedef typename _Molecule::Atom _Atom;

		std::map<Common::IndexType, Common::IndexType> mapIDs; //Key is atom sequence number, value is atom ID in the molecule
		std::map<std::pair<Common::IndexType, Common::IndexType>, Common::ComChar> mapBonds; // Key is pair [unordered] pair of atoms sequence numbers, value is symbolic bond type
		std::map<Common::IndexType, std::pair<Common::IndexType, std::pair<Common::ComChar, Common::ComChar> > > mapRes; //To check uniqueness of residue numbers. Key is res ID (value is resID PDB for more information)

		Common::ComString strLine, strField;
		Common::ComString::size_type nPos;
		_Atom atomSample;//sample 'Atom' object to be modified on-fly and inserted into molecule 

		//const IndexType nDefID = atomSample.GetID(); //Remember default value
		//const IndexType nDefSequenceNumber = atomSample.GetSequenceNumber(); //Atom sequence number as it is given in file. It can be not sequential within the atoms record and potentially can be not unique without some extra info (e.g. chain id)
		Common::ComString strDefElement = Common::g_PT[Common::ePTEUndefined].m_strElement; //Symbol of chemical element (1 to 3 characters)

		const int nDefMolNumber = 0;
		const Common::IndexType nDefResSequenceNumber = atomSample.GetResSequenceNumber(); //Remember default value
		const Common::ComString strDefResName = atomSample.GetResName(); //Remember default value
		const Common::ComChar cDefResInsertionCode = atomSample.GetResInsertionCode();
		const Common::ComChar cDefResChainID = atomSample.GetResChainID();

		int nCurMolNumber = nDefMolNumber; //Molecule number (index) as it is given in file' molecule description
		Common::IndexType nCurResSequenceNumber = nDefResSequenceNumber; //Residue number as it is given in file' residue description

		Common::IndexType nID, nID2; //External key - ID of atom as stored in some external data structure
		Common::IndexType nSequenceNumber; //Atom sequence number as it is given in file. It can be not sequential within the atoms record and potentially can be not unique without some extra info (e.g. chain id)
		Common::ComString strElement; //Symbol of chemical element (1 to 3 characters)
		int nElementID; //ID of atom chemical element as given in periodic table (1 based)
		Common::ComString strFFType; //[Symbolic] name of FF-type of atom
		Common::ComString strLabel; //Symbolic atom label (atom name as given in file)
		Common::ComString strFlags;

		typename _Atom::PositionNumeric x, y, z; //Atom position (cartesian coordinates)
		typename _Atom::PositionNumeric vx, vy, vz; //Atom velocity (cartesian coordinates)
		typename _Atom::EnergyNumeric dCharge; //Fractional atom charge as it is given in file
		typename _Atom::EnergyNumeric dTemperature; //Temperature factor of the atom, used primarily in PDB-files
		typename _Atom::EnergyNumeric dMass; //Mass of the atom
		unsigned char atomFlags; //Set of atom flags. Some of flag sets can be bitwise combined
		unsigned char bondFlags;
		int nBonds;
		Common::IndexType nSeqNumber2;
		Common::ComChar cBondType;

		try
		{
			for ( ; ; )
			{
				//if (helper.IsEOF())
				if (IsEOF(is, pos))
				{
					return false; //Position marker is at the end of the source stream/container. Nothing to do
				}
				//Read current line from the stream/container
				//strLine = helper.GetLine();
				strLine = ExtractLine(is, pos);
				//Check for comments block in the current line, cut it if found
				if ((nPos = strLine.find_first_of(_ComStr(';'))) != Common::ComString::npos) //Comments detected
				{
					Common::ComString strComment = strLine.substr(nPos + 1, strLine.length() - (nPos + 1));
					if (!strComment.empty())
					{
						*itComments++ = strComment; //Add Comment line to output container
						mol.AddComment(strComment); //Add Comment line to molecule' own container of comments

					}
					strLine = strLine.substr(0, nPos);
				}
				//Read the keyword of the line
				strField = _ComStr("");
				std::basic_stringstream<Common::ComChar> ssBuffer(strLine);
				ssBuffer >> strField;
					//Check for most common keyword(s) first
				if (strField == _ComStr("atom"))
				{
					ssBuffer >> nSequenceNumber; //Sequence number
					ssBuffer >> strLabel; //Atom label name;
					ssBuffer >> strElement; //Element;
					ssBuffer >> strFFType; //FFtype;
					ssBuffer >> strFlags; //Flags;
					ssBuffer >> dCharge;
					ssBuffer >> x;
					ssBuffer >> y;
					ssBuffer >> z;
					nElementID = Common::g_PT[strElement].m_nNumber;
					atomFlags = 0;
					if (strFlags.find(_ComStr('h')) != Common::ComString::npos)
					{
						atomFlags |= Common::eAtomOwnershipHetero;
					}
					//if (strFlags.find(_ComStr('i')) != Common::ComString::npos) //Is not supported by existing bond definition format
					//{
					//}
					//if (strFlags.find(_ComStr('x')) != Common::ComString::npos) //Is not supported by existing bond definition format
					//{
					//}
					if (strFlags.find(_ComStr('s')) != Common::ComString::npos)
					{
						atomFlags |= Common::eAtomIsSelected;
					}
					atomSample.SetSequenceNumber(nSequenceNumber);
					atomSample.SetLabel(strLabel);
					atomSample.SetElementID(nElementID);
					atomSample.SetFFType(strFFType);
					atomSample.SetAtomFlags(atomFlags);
					atomSample.SetCharge(dCharge);
					atomSample.SetX(x);
					atomSample.SetY(y);
					atomSample.SetZ(z);
					nID = mol.AddAtom(atomSample);
					if (mapIDs.find(nSequenceNumber) != mapIDs.end())
					{
						Common::CException::Throw(__FILE__, __LINE__, _ComStr("Inconsistency in HIN file: 'atom' %d is duplicated in the molecule "), nSequenceNumber);
					}
					mapIDs.insert(std::make_pair(nSequenceNumber, nID));
					//Read bonds information
					nBonds = -1;
					ssBuffer >> nBonds;
					if (nBonds == -1) //No bonds information
					{
						Common::CException::Throw(__FILE__, __LINE__, _ComStr("Inconsistency in HIN file: 'atom' %d does not contain bond information"), nSequenceNumber);
					}
					if (nBonds > MaxAtomBondsNumber) //Format voilation: too many bonds
					{
						Common::CException::Throw(__FILE__, __LINE__, _ComStr("Inconsistency in HIN file: 'atom' %d contains %d bonds that exceeds the limit of 12"), nSequenceNumber, nBonds);
					}
					for (Common::BondIndexType iBond = 0; iBond < nBonds; ++iBond)
					{
						nSeqNumber2 = -1;
						cBondType = 0;
						ssBuffer >> nSeqNumber2 >> cBondType;
						if (nSeqNumber2 <= 0 || !(cBondType == _ComStr('s') || cBondType == _ComStr('d') || cBondType == _ComStr('t') || cBondType == _ComStr('a')))
						{
							Common::CException::Throw(__FILE__, __LINE__, _ComStr("Inconsistency in HIN file: 'atom' %d contains not enough or corrupted bond information: bond %d is '%d %c'"), nSequenceNumber, iBond+1, nSeqNumber2, cBondType);
						}
						if (nSequenceNumber == nSeqNumber2)
						{
							Common::CException::Throw(__FILE__, __LINE__, _ComStr("Inconsistency in HIN file: 'atom' %d references itself"), nSequenceNumber);
						}
						if (mapBonds.find(std::make_pair(nSequenceNumber, nSeqNumber2)) != mapBonds.end())
						{
							Common::CException::Throw(__FILE__, __LINE__, _ComStr("Inconsistency in HIN file: 'atom' %d contains duplicated bond reference on atom %d"), nSequenceNumber, nSeqNumber2);
						}
						mapBonds.insert(std::make_pair(std::make_pair(nSequenceNumber, nSeqNumber2), cBondType));
					}
					atomSample.SetID(nID); //Remember index of the currently inserted atom. It can be needed while handling further records of the file
					continue; //After 'atom' record, go to next line immediately to keep the sampleAtom' Sequence number (it is hold only one line after last 'atom' record)
				} //if (strField == "atom")
				if (strField == _ComStr("vel")) //By standard, this 'vel' record applies to the preceding atom record
				{
					Common::IndexType nSequenceNumber = 0;
					vx = vy = vz = 0.;
					ssBuffer >> nSequenceNumber >> vx >> vy >> vz;
					if (nSequenceNumber != atomSample.GetSequenceNumber()) 
					{
						Common::CException::Throw(__FILE__, __LINE__, _ComStr("Inconsistency in HIN file: 'vel' appeared for atom n=%d is not matched to current atom n=%d"), nSequenceNumber, atomSample.GetSequenceNumber());
					}
					_Atom& atom = mol.GetAtom(atomSample.GetID());
					atom.SetVx(vx);
					atom.SetVy(vy);
					atom.SetVz(vz);
					continue;
				} //if (strField == "vel")
				if (strField == _ComStr("mass")) //By standard, this 'mass' record applies to the preceding atom record
				{
					Common::IndexType nSequenceNumber = 0;
					dMass = 0.;
					ssBuffer >> nSequenceNumber >> dMass;
					if (nSequenceNumber != atomSample.GetSequenceNumber()) 
					{
						Common::CException::Throw(__FILE__, __LINE__, _ComStr("Inconsistency in HIN file: 'mass' appeared for atom n=%d is not matched to current atom n=%d"), nSequenceNumber, atomSample.GetSequenceNumber());
					}
					_Atom& atom = mol.GetAtom(atomSample.GetID());
					atom.SetMass(dMass);
					continue;
				} //strField == "mass"
				if (strField == _ComStr("basisset"))
				{
					//TODO. Stub - not implemented yet
					continue;
				} //strField == "basisset"
				//
				if (strField == _ComStr("res"))
				{
					Common::IndexType nOldResSequenceNumber = nCurResSequenceNumber; //Remember previous value for further checking for consistency
					nCurResSequenceNumber = nDefResSequenceNumber;
					Common::ComString strCurResName = strDefResName;
					Common::IndexType nCurResNumberPDB = nDefResSequenceNumber;
					Common::ComChar cCurResInsertionCodePDB = cDefResInsertionCode;
					Common::ComChar cCurResChainIDPDB = cDefResChainID;
					ssBuffer >> nCurResSequenceNumber >> strCurResName >> nCurResNumberPDB >> cCurResInsertionCodePDB >> cCurResChainIDPDB;
					if (nOldResSequenceNumber != nDefResSequenceNumber) //[Previous] residue description was not properly finished
					{
						Common::CException::Throw(__FILE__, __LINE__, _ComStr("Inconsistency in HIN file: 'res'=%d appears before 'res'=%d has been finished"), nCurResSequenceNumber, nOldResSequenceNumber);
					}
					if (mapRes.find(nCurResSequenceNumber) != mapRes.end())
					{
						Common::CException::Throw(__FILE__, __LINE__, _ComStr("Inconsistency in HIN file: 'res'=%d is not unique among residue numbers"), nCurResSequenceNumber);
					}
					mapRes.insert(std::make_pair(nCurResSequenceNumber, std::make_pair(nCurResNumberPDB, std::make_pair(cCurResInsertionCodePDB, cCurResChainIDPDB))));
					//Perform fields correction to replace stubs with defaults
					if (strCurResName == _ComStr("-")) //"-" is default substitution in HIN-file for 'nothing'
					{
						strCurResName = strDefResName;
					}
					if (nCurResNumberPDB == nDefResSequenceNumber) //PDB insertion code is not given? Set it to [given] current Res. number
					{
						nCurResNumberPDB = nCurResSequenceNumber; //Potentially this can result in issues if half of PDB seq. numbers is given and others are not. But probably this is better than use 'default' PDB seq. numbers (they are all identical)
					}
					if (cCurResInsertionCodePDB == _ComStr('-')) //'-' is default substitution in HIN-file for 'nothing'
					{
						cCurResInsertionCodePDB = cDefResInsertionCode;
					}
					if (cCurResChainIDPDB == _ComStr('-')) //'-' is default substitution in HIN-file for 'nothing'
					{
						cCurResChainIDPDB = cDefResChainID;
					}
					atomSample.SetResSequenceNumber(nCurResNumberPDB); //Should it have been nCurResSequenceNumber instead?
						//atomSample.SetResSequenceNumber(nCurResSequenceNumber);
					atomSample.SetResName(strCurResName);
					atomSample.SetResInsertionCode(cCurResInsertionCodePDB);
					atomSample.SetResChainID(cCurResChainIDPDB);
				} //if (strField == "res")
				if (strField == _ComStr("endres"))
				{
					Common::IndexType nFinishedResSequenceNumber = nDefResSequenceNumber;
					ssBuffer >> nFinishedResSequenceNumber;
					if (nCurResSequenceNumber != nFinishedResSequenceNumber) 
					{
						Common::CException::Throw(__FILE__, __LINE__, _ComStr("Inconsistency in HIN file: 'endres'=%d is not matched to 'res'=%d"), nFinishedResSequenceNumber, nCurResSequenceNumber);
					}
					nCurResSequenceNumber = nDefResSequenceNumber;
					atomSample.SetResSequenceNumber(nDefResSequenceNumber);
					atomSample.SetResName(strDefResName);
					atomSample.SetResInsertionCode(cDefResInsertionCode);
					atomSample.SetResChainID(cDefResChainID);
				} //if (strField == "endres")
				else if (strField == _ComStr("mol"))
				{
					int nOldMolNumber = nCurMolNumber; //Remember previous value for further checking for consistency
					nCurMolNumber = nDefMolNumber;
					ssBuffer >> nCurMolNumber; //Read the ID of the molecule
					if (nOldMolNumber != nDefMolNumber) //[Previous] molecule description was not properly finished
					{
						Common::CException::Throw(__FILE__, __LINE__, _ComStr("Inconsistency in HIN file: 'mol'=%d appears before 'mol'=%d has been finished"), nCurMolNumber, nOldMolNumber);
					}
					mol.SetID(nCurMolNumber);
					strField = GetNameFromRestOfLine(ssBuffer);
					mol.SetName(strField == _ComStr("-") ? Common::ComString(_ComStr("")) : strField);
				} //strField == "mol"
				else if (strField == _ComStr("endmol"))
				{
					if (nCurResSequenceNumber != nDefResSequenceNumber) 
					{
						Common::CException::Throw(__FILE__, __LINE__, _ComStr("Inconsistency in HIN file: 'endmol' appears before current 'endres'=%d has finished"), nCurResSequenceNumber);
					}
					int nFinishedMolNumber = nDefMolNumber; //Remember previous value for further checking for consistency
					ssBuffer >> nFinishedMolNumber; //Read the ID of the molecule
					if (nCurMolNumber != nFinishedMolNumber) 
					{
						Common::CException::Throw(__FILE__, __LINE__, _ComStr("Inconsistency in HIN file: 'endmol'=%d is not matched to 'mol'=%d"), nFinishedMolNumber, nCurMolNumber);
					}
					nCurMolNumber = nDefMolNumber;
					break; //Stop looking through input stream/container records
				} //strField == "endmol"
				//Other keywords
				else if (strField == _ComStr("forcefield"))
				{	
					*itKeylines++ = strLine; //Stub. Save the line for using by higher-level functions
					strField = GetNameFromRestOfLine(ssBuffer);
					mol.SetForceField(strField);
				} //strField == "forcefield"
				else if (strField == _ComStr("sys"))
				{
					ssBuffer >> dTemperature;
					*itKeylines++ = strLine; //Stub. Save the line for using by higher-level functions
				} //strField == "sys"
				else if (strField == _ComStr("seed"))
				{
					*itKeylines++ = strLine; //Stub. Save the line for using by higher-level functions
				} //strField == "seed"
				else if (strField == _ComStr("view"))
				{
					*itKeylines++ = strLine; //Stub. Save the line for using by higher-level functions
				} //strField == "view"
				else if (strField == _ComStr("box"))
				{
					*itKeylines++ = strLine; //Stub. Save the line for using by higher-level functions
				} //strField == "box"
				else if (strField == _ComStr("selection"))
				{
					//TODO. Stub - not implemented yet
				} //strField == "selection"
				else if (strField == _ComStr("selectatom"))
				{
					//TODO. Stub - not implemented yet
				} //strField == "selectatom"
				else if (strField == _ComStr("endselection"))
				{
					//TODO. Stub - not implemented yet
				} //strField == "endselection"
				else if (strField == _ComStr("dynamics"))
				{
					*itKeylines++ = strLine; //Stub. Save the line for using by higher-level functions
				} //strField == "dynamics"
				atomSample.SetSequenceNumber(0); //To supervise strictly the format rules: records that rely on atom ('vel', 'mass', 'basisset') are valid only if they appear immediately after 'atom' record
			}

			//Set bonds information to atoms
			std::map<std::pair<Common::IndexType, Common::IndexType>, Common::ComChar>::iterator itm = mapBonds.begin(), itm2 = itm;
			for ( ; itm != mapBonds.end(); ++itm)
			{
				if (  itm->first.first > itm->first.second
					|| (itm2 = mapBonds.find(std::make_pair(itm->first.second, itm->first.first))) == mapBonds.end()
					)
				{
					Common::CException::Throw(__FILE__, __LINE__, _ComStr("Inconsistency in HIN file: 'atom' %d references atom %d but not vice versa"), itm->first.first, itm->first.second);
				}
				if (itm->second != itm2->second)
				{
					Common::CException::Throw(__FILE__, __LINE__, _ComStr("Inconsistency in HIN file: 'atom' %d references atom %d with bond type '%c', but atom %d references atom %d with bond type '%c'"), 
						itm->first.first, itm->first.second, itm->second, itm2->first.first, itm2->first.second, itm2->second);
				}
				nID  = mapIDs[itm->first.first]; //The records must exist
				nID2 = mapIDs[itm->first.second];
				bondFlags = (itm->second == _ComStr('s')) ? Common::eBondSingle
							: (itm->second == _ComStr('d')) ? Common::eBondDouble
							: (itm->second == _ComStr('t')) ? Common::eBondTriple
							: (itm->second == _ComStr('a')) ? Common::eBondAromatic
							: Common::eBondDummy; //Stub. Should never happen - see checks above
				mol.AddBond(nID, nID2, bondFlags);
				mapBonds.erase(itm2); //To further check for bonds consistency. This does not affect current iterator
			}

		}
		catch (const Common::CException& e)
		{
			Common::ComString strMessage = e.Message();
			Common::Logger::Start(Common::eError, _ComStr("LoadMoleculeFromHINStream()")) << strMessage << Common::Logger::End;
			throw;
		}
		catch (...)
		{
			Common::ComString strMessage = _ComStr("Unknown exception");
			Common::Logger::Start(Common::eError, _ComStr("LoadMoleculeFromHINStream()")) << strMessage << Common::Logger::End;
			throw;
		}

		return true;
	}

	template <typename _Atom, typename _Molecule>
	bool LoadMoleculeFromHINStream(std::basic_istream<Common::ComChar>& is, _Molecule& mol)
	{
		std::list<Common::ComString> listComments; //Information will be lost
		std::list<Common::ComString> listKeylines; //Information will be lost
		return LoadMoleculeFromHINStream (is,
								is.tellg(), //std::ios::cur,
								mol,
								std::back_insert_iterator<std::list<Common::ComString> >(listComments),
								std::back_insert_iterator<std::list<Common::ComString> >(listKeylines)
								);
	}

	//Light-weight structire, shell on the molecule to allow its serializing from HIN-format stream using '>>' operator
	template <typename _Molecule>
	struct HINParserT
	{
		HINParserT(_Molecule& mol)
		{
			m_pMolecule = &mol;
		}
		_Molecule *m_pMolecule;
	};

	//Implementation of operator '>>' that populates molecule from HIN-format stream
	template <typename _Molecule>
	std::istream& operator >> (std::istream& is, HINParserT<_Molecule>& hinParser)
	{
		LoadMoleculeFromHINStream(is, *hinParser.m_pMolecule);
		return is;
	}

	//Proxy to be used for more compact call of molecule populating from HIN. An example scenario would be
	// [Container / stream with HIN data] >> ReadHIN(molecule);
	template <typename _Molecule>
	HINParserT<_Molecule> ReadHIN(_Molecule& mol)
	{
		return HINParserT<_Molecule>(mol);
	}

////////////////////////////////////////////////////////////////////////////////////////

	//General function that serialises molecule structure to 'stream' in HIN file format.
	//There can be the follwing types of output 'stream' and 'streampos':
	// - '_Stream' is std::basic_ostream<Common::ComChar> (and derived classes), '_Streampos' is in this case just a constant like std::ios::cur
	// the serialization to vector / list of strings is not implemented by now. It can easily be performed on later stage using populated stream.
	template <typename _Stream, /*typename _Atom,*/ typename _Molecule>
	bool SaveMoleculeToHINStream(_Stream& os,
		const _Molecule& mol //Molecule structure to be serialized
		)
	{
		typedef typename _Molecule::Atom _Atom;

		const Common::ComChar cEmptyChar = _ComStr(' '); 
		const Common::ComChar cNullChar = _ComStr('\0');
		const Common::ComChar cStubChar  = _ComStr('-');
		const Common::ComString strEmptyString = _ComStr("");
		const Common::ComString strStubString = _ComStr("-");
		const int nDefResSequenceNumber = 1;
		const Common::ComString strDefResName = _ComStr(""); //Symbolic name of residue
		const char cDefResInsertionCode  = _ComStr('\0');
		const char cDefResInsertionCode2 = _ComStr(' ');
		const char cDefResChainID  = UndefinedChainLabel;
		const char cDefResChainID2 = _ComStr(' ');
		const Common::ComString strDelimiter = _ComStr(" ");//_ComStr("\t"); 

		Common::IndexType nThroughResNumber = 1;
		Common::IndexType nCurResSequenceNumber = nDefResSequenceNumber; //Residue number as it is given in atom' residue description
		Common::ComString strCurResName = strDefResName; //Residue name as it appears in PDB file ('res-name' record in 'res' line of HIN-file)
		char cCurResInsertionCode = cDefResInsertionCode; //Residue insertion code as it appears in atom
		char cCurResChainID = cDefResChainID; //Chain ID as it appears in PDB file ('next-res#' record in 'res' line of HIN-file)

		Common::ComString strLabel; //Symbolic atom label
		Common::ComString strElement; //Symbol of chemical element (1 to 3 characters)
		Common::ComString strFFType; //[Symbolic] name of FF-type of atom
		Common::ComString strAtomFlags;
		char cBondType;
		int iBond, nBonds;
		bool bResidueIsOpen = false;

		try
		{
				//Molecule number and name
			Common::ComString strMoleculeName = mol.GetName().empty() ? strStubString : mol.GetName();
			os << _ComStr("mol")
				<< strDelimiter << mol.GetID()
				<< strDelimiter << (strMoleculeName.find_first_of(_ComStr(" \t\r\n\"")) == Common::ComString::npos
									? strMoleculeName
									: (_ComStr("\"") + strMoleculeName + _ComStr("\""))
									)
				<< std::endl;

			//	//Molecule forcefield as a comment?
			//if (!mol.GetForceField().empty())
			//{
			//	os << "; " << "forcefield"
			//		<< " " << mol.GetForceField()
			//		<< std::endl;
			//}
				//Molecule own comments
			for (auto it = mol.GetComments().begin(), itEnd = mol.GetComments().end(); it != itEnd; ++it)
			{
				os << _ComStr("; ") << *it << std::endl;
			}

				//Cycle by atoms
			for (Common::IndexType iAtom = 0, nAtoms = mol.GetAtomsNumber(); iAtom < nAtoms; ++iAtom)
			{
				const _Atom& curAtom = mol.GetAtom(iAtom);
				//Analyze for whether one should insert 'res'/entres' entries
				if ( (nCurResSequenceNumber != curAtom.GetResSequenceNumber())
					|| (strCurResName != curAtom.GetResName())
					|| (cCurResInsertionCode != curAtom.GetResInsertionCode())
					|| (cCurResChainID != curAtom.GetResChainID())
					)
				{	//Something has changed in residue description
					//Analyze for whether one should insert 'endres' field for preceeding residue
					if (bResidueIsOpen)
					{	//Current resudue identifiers are the default ones. One should finalyze previous residue i.e. insert 'endres'entry
						os << _ComStr("endres") 
							<< strDelimiter << nThroughResNumber 
							<< std::endl;
						bResidueIsOpen = false;
						++nThroughResNumber;
					}
					nCurResSequenceNumber = curAtom.GetResSequenceNumber();
					strCurResName = curAtom.GetResName();
					cCurResInsertionCode = curAtom.GetResInsertionCode();
					cCurResChainID = curAtom.GetResChainID();
					//Analyze for whether one should insert 'res' entry for current atom [group]

					if ( nCurResSequenceNumber != nDefResSequenceNumber
						|| strCurResName != strDefResName
						|| (cCurResInsertionCode != cDefResInsertionCode && cCurResInsertionCode != cDefResInsertionCode2)
						|| (cCurResChainID != cDefResChainID && cCurResChainID != cDefResChainID2)
						)
					{	//Residue description in the atom is not the defaul one. One should insert 'res' entry
						os << _ComStr("res")
							<< strDelimiter << nThroughResNumber
							<< strDelimiter <<  (GetNameFromLine(strCurResName) == strEmptyString ? strStubString : GetNameFromLine(strCurResName))
							<< strDelimiter <<  ((cCurResInsertionCode == cEmptyChar || cCurResInsertionCode == cNullChar) ? cStubChar : cCurResInsertionCode)
							<< strDelimiter <<  ((cCurResChainID == cEmptyChar || cCurResChainID == cNullChar) ? cStubChar : cCurResChainID)
							<< std::endl;
						bResidueIsOpen = true;
					}
				} //Something has changed in residue description
				//Serialize atom information
				strAtomFlags = Common::ComString((curAtom.GetAtomFlags() & Common::eAtomOwnershipHetero) ? _ComStr("h") : _ComStr(""))
										 + Common::ComString((curAtom.GetAtomFlags() & Common::eAtomIsSelected) ? _ComStr("s") : _ComStr(""));
				strLabel = GetNameFromLine(Common::ComString(curAtom.GetLabel()));
				strElement = Common::g_PT[int(curAtom.GetElementID())].m_strElement;
				strFFType = curAtom.GetFFType();
				os << _ComStr("atom")
					<< strDelimiter << curAtom.GetSequenceNumber()
					<< strDelimiter << (strLabel == strEmptyString ? strStubString : strLabel)
					<< strDelimiter << strElement
					<< strDelimiter << (strFFType == strEmptyString ? strStubString : strFFType)
					<< strDelimiter << (strAtomFlags == strEmptyString ? strStubString : strAtomFlags)
					<< strDelimiter << curAtom.GetCharge()
					<< strDelimiter << curAtom.GetX()
					<< strDelimiter << curAtom.GetY()
					<< strDelimiter << curAtom.GetZ()
					<< strDelimiter;
#if 0
//Atom itself states which bonds it has. This could be wrong if it has bonds with atoms that do not belong to this molecule
				os << (nBonds = curAtom.GetBondsNumber());
				for (iBond = 0; iBond < nBonds; ++iBond)
				{
					const typename _Atom::Bond& bond = curAtom.GetBond(iBond);
					switch (bond.second & Common::eBondOrdersMask)
					{
					case Common::eBondSingle: cBondType = _ComStr('s'); break;
					case Common::eBondDouble: cBondType = _ComStr('d'); break;
					case Common::eBondTriple: cBondType = _ComStr('t'); break;
					case Common::eBondAromatic: cBondType = _ComStr('a'); break;
					default: cBondType = cStubChar; break;
					}
					os << strDelimiter << mol.GetAtom(bond.first).GetSequenceNumber()
						 << strDelimiter << cBondType;
				}
#else
//Molecule enumerates atom' bonds
				os << (nBonds = mol.GetOwnBondsNumber(iAtom));
				const _Atom* pAtom2 = NULL;
				Common::BondFlags bondFlags;
				for (iBond = 0; iBond < nBonds; ++iBond)
				{
					if (mol.GetOwnNeighbourAtom(iAtom, iBond, pAtom2, bondFlags))
					{
						switch (bondFlags & Common::eBondOrdersMask)
						{
						case Common::eBondSingle: cBondType = _ComStr('s'); break;
						case Common::eBondDouble: cBondType = _ComStr('d'); break;
						case Common::eBondTriple: cBondType = _ComStr('t'); break;
						case Common::eBondAromatic: cBondType = _ComStr('a'); break;
						default: cBondType = cStubChar; break;
						}
						//Additional check for SS-bond
						if (bondFlags & Common::eBondIsSS)
						{
							cBondType = _ComStr('s');
						}
						os << strDelimiter << pAtom2->GetSequenceNumber()
							<< strDelimiter << cBondType;
					}
					else
					{
						Common::CException::Throw(__FILE__, __LINE__, _ComStr("Failed to extract %d-th [own] neighbour for atom %d"), 
							iBond, curAtom.GetSequenceNumber());
					}
				}
#endif
				os << std::endl;

				//Atom velocities, if defined
				if ( curAtom.GetVx() != (typename _Atom::PositionNumeric)0.
					|| curAtom.GetVy() != (typename _Atom::PositionNumeric)0.
					|| curAtom.GetVz() != (typename _Atom::PositionNumeric)0.
					)
				{
					os << _ComStr("vel")
						<< strDelimiter << curAtom.GetSequenceNumber()
						<< strDelimiter << curAtom.GetVx()
						<< strDelimiter << curAtom.GetVy()
						<< strDelimiter << curAtom.GetVz()
						<< std::endl;
				}

				//Atom mass, if defined
				if (curAtom.GetMass() != (typename _Atom::EnergyNumeric)0.)
				{
					os << _ComStr("mass")
						<< strDelimiter << curAtom.GetSequenceNumber()
						<< strDelimiter << curAtom.GetMass()
						<< std::endl;
				}

			} //Cycle by atoms

				//Finalyze the molecule
			if (bResidueIsOpen)
			{	//Current resudue identifiers are the default ones. One should finalyze previous residue i.e. insert 'endres'entry
				os << _ComStr("endres")
					<< strDelimiter << nThroughResNumber
					<< std::endl;
				bResidueIsOpen = false;
				++nThroughResNumber;
			}
			os << _ComStr("endmol")
				<< strDelimiter << mol.GetID()
				<< std::endl;
		}
		catch(...)
		{
		}

		return true;
	}

	//Light-weight structire, shell on the molecule to allow its serializing to HIN-format stream using '<<' operator
	template <typename _Molecule>
	struct HINPrinterT
	{
		HINPrinterT(_Molecule& mol)
		{
			m_pMolecule = &mol;
		}
		_Molecule* m_pMolecule;
	};

	//Implementation of operator '<<' that populates HIN-format stream from the molecule
	template <typename _Molecule>
	std::ostream& operator << (std::ostream& os, HINPrinterT<_Molecule>& hinPrinter)
	{
		SaveMoleculeToHINStream(os, *hinPrinter.m_pMolecule);
		return os;
	}

	//Proxy to be used for more compact call of molecule serializing to HIN. An example scenario would be
	// [Container / stream to be populated with HIN data] << WriteHIN(molecule);
	template <typename _Molecule>
	HINPrinterT<_Molecule> WriteHIN(_Molecule& mol)
	{
		return HINPrinterT<_Molecule>(mol);
	}

	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

	template <typename _Molecule = CIOMolecule, typename _ContainerMol = std::vector<std::shared_ptr<_Molecule> >, typename _ContainerComments = std::vector<Common::ComString>, typename _ContainerPendingKeylines = std::vector<Common::ComString> >
	class CFileIOHINT : public CFileIOMolecularT<_Molecule, _ContainerMol, _ContainerComments, _ContainerPendingKeylines>
	{
	public:
		typedef CFileIOMolecularT<_Molecule, _ContainerMol, _ContainerComments, _ContainerPendingKeylines> _BaseType;
		virtual bool ReadMolecule(std::basic_istream<Common::ComChar>& is,
									_Molecule& mol, //Molecule structure to be populated
									_ContainerComments& contComments, //List of comments to be populated. Comments are detected only in the lines range from current input stream position to the end of molecule/stream
									_ContainerPendingKeylines& contPendingKeylines //List of lines starting from HIN keywords that are not handled within this function, or are related to the whole file. 
									)
		{
			return LoadMoleculeFromHINStream(is,
									is.tellg(), //std::ios::cur,
									mol,
									std::back_insert_iterator<_ContainerComments>(contComments),
									std::back_insert_iterator<_ContainerPendingKeylines>(contPendingKeylines) //List of lines starting from HIN keywords that are not handled within this function, or are related to the whole file. 
									);
		}

		virtual bool ApplyKeywords(_Molecule& mol, const _ContainerPendingKeylines& contPendingKeylines)
		{
			Common::ComString strField;
			for (typename _ContainerPendingKeylines::const_iterator it = contPendingKeylines.begin(), itEnd = contPendingKeylines.end(); it != itEnd; ++it)
			{
				std::basic_stringstream<Common::ComChar> ssBuffer(*it);
				ssBuffer >> strField;
				if (strField == _ComStr("forcefield")) //Stub. Do not check for leading spaces etc. ...
				{
					strField = GetNameFromRestOfLine(ssBuffer);
					mol.SetForceField(strField);
				}
			}
			return true;
		}

		virtual bool WriteMolecule(std::basic_ostream<Common::ComChar>& os,
									const _Molecule& mol, //Molecule structure to be serialized
									const _ContainerComments& contComments = _ContainerComments(), //List of comments to be serialized
									const _ContainerPendingKeylines& contPendingKeylines = _ContainerPendingKeylines() //List of lines with keywords to be serialized
									)
		{
			//Write comment lines
			WriteComments(os, contComments);
			//Write keylines
			WriteKeylines(os, contPendingKeylines);
			//Write molecule contents
			return SaveMoleculeToHINStream(os, mol);
		}

		virtual bool WriteComments(std::basic_ostream<Common::ComChar>& os,
									const _ContainerComments& contComments //List of comments to be serialized
									)
		{
			return _BaseType::writeLines(os, contComments, _ComStr(";"));
		}

		virtual bool WriteKeylines(std::basic_ostream<Common::ComChar>& os,
									const _ContainerPendingKeylines& contPendingKeylines //List of lines with keywords to be serialized
									)
		{
			return _BaseType::writeLines(os, contPendingKeylines, _ComStr(""));
		}

	};

	typedef CFileIOHINT<CIOMolecule, std::vector<std::shared_ptr<CIOMolecule> >, std::vector<Common::ComString>, std::vector<Common::ComString> > CFileIOHIN;

} //namespace FileIO
