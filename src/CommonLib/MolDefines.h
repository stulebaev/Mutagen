#pragma once

#include "Typedefs.h"

namespace Common
{ //namespace Common

//#ifndef IndexType
//	typedef Common::IndexType AtomIndexType; //Type of variable used for indexing atoms
//#endif

#ifndef BondIndexType
	typedef unsigned char BondIndexType; //Type of variable used for indexing bonds of particular atom
#endif

#ifndef AtomFlags
	typedef unsigned char AtomFlags; //Set of [bitwise] flags characterizing atom
#endif

#ifndef BondFlags
	typedef unsigned char BondFlags; //Set of [bitwise] flags characterizing bond
#endif

#define MaxAtomBondsNumber	12 //8 //HIN file format allows up to 12 covalent bonds
#define MaxAtomLinksNumber	MaxAtomBondsNumber //12

#define UndefinedMolSubstitutionCode	_ComChar('\0') //Symbol for molecule substitution that indicates that molecule substitution (other molecule(s) with the same sequential number etc.) was not provided
#define UndefinedChainLabel						_ComChar('\0') //Symbol for chain label that indicates that chain identifier was not provided
#define UndefinedResInsertionCode			_ComChar('\0') //Symbol for residue insertion code that indicates that residue insertion code was not provided

#define MinValidResidueNumber		-9999  //Minimal value of residue number that can be treated as valid one (negative residue numbers can be still valid e.g. if they indicate extra residues when compared to 'sample' PDB structure
#define MaxInvalidResidueNumber	-10000 //Maxinal value of residue number that is treated as invalid valid one (negative residue numbers can be still valid e.g. if they indicate extra residues when compared to 'sample' PDB structure

	enum EBondDefinitions
	{
		eBondNoncovalent= 0x00, //0x00000000
		eBondDummy			= 0x00, //0x00000000
		eBondSingle			= 0x01, //0x00000001
		eBondDouble			= 0x02, //0x00000010
		eBondTriple			= 0x03, //0x00000011
		eBondAromatic		= 0x04, //0x00000100
		eBondOrdersMask	= 0x07, //0x00000111 //Mask for all covalent types of bonds
		eLinkHydrogen		= 0x08, //0x00001000
		eLinkIonic			= 0x10, //0x00010000
		eBondIsSS				= 0x20, //0x00100000 //Flag for the bond is SS (in addition to the formal bond order)
		eBondIsAmide		= 0x40, //0x01000000 //Flag for the bond is amide (in addition to formal bond order)
		eBondIsAromatic	= 0x80, //0x10000000 //Flag that the bond is aromatic (in addition to formal bond order)
		eBondIsRotatable= 0x01, //0x00000001 //Does not work //Flag that there can be [formal] rotations of the parts of the molecule around this bond
		eBondAllFlags		= 0xff  //0x11111111
	};

	inline Common::ComString BondOrderName(BondFlags bondFlags)
	{
		switch (bondFlags & eBondOrdersMask)
		{
			case eBondSingle: return Common::ComString(_ComStr("BondSingle"));
			case eBondDouble: return Common::ComString(_ComStr("BondDouble"));
			case eBondTriple: return Common::ComString(_ComStr("BondTriple"));
			case eBondAromatic: return Common::ComString(_ComStr("BondAromatic"));
			default: return Common::ComString(_ComStr("UnknownBondOrder"));
		};
	}

	////////////////////////////////////////////////////////////////////////////

	enum EAtomDefinitions
	{
		eAtomHybridizationUndefined	= 0x00, //0x00000000
		eAtomHybridizationSP3		= 0x01, //0x00000001
		eAtomHybridizationSP2		= 0x02, //0x00000010
		eAtomHybridizationSP		= 0x03, //0x00000011
		eAtomHybridizationsMask	= 0x03, //0x00000011
		eAtomOwnershipUndefined	= 0x00, //0x00000000
		eAtomOwnershipHetero		= 0x04, //0x00000100
		eAtomOwnershipProtein		= 0x08, //0x00001000
		eAtomOwnershipBackbone	= 0x0c, //0x00001100
		eAtomOwnershipsMask			= 0x0c, //0x00001100
		eAtomIsSelected					= 0x10, //0x00010000
		eAtomIsHidden						= 0x20, //0x00100000
		eAtomIsInRing						= 0x40, //0x01000000 //Auxiliary informational flag, typically is a result of molecule processing
		eAtomIsAromatic					= 0x80, //0x10000000 //Auxiliary informational flag, typically is a result of molecule processing
		eAtomAllFlags						= 0xff  //0x11111111
	};

	////////////////////////////////////////////////////////////////////////////

	enum EBondInsertionPolicies
	{
		eDoNotCheckForDuplication,
		eDuplicateIfExists = eDoNotCheckForDuplication,
		eOverrideIfExists,
		eSkipIfExists,
		eThrowIfExists
	};

	////////////////////////////////////////////////////////////////////////////

	enum EGroupDefinitions
	{
		eGroupUnknown = -1,
		eGroupHetero,
		eGroupResidue,
		eGroupChain
	};

	////////////////////////////////////////////////////////////////////////////

	enum EMoleculeDefinitions
	{
		eMoleculeUnknown = -1,
		eMoleculeHetero,
		eMoleculeProtein
	};

	////////////////////////////////////////////////////////////////////////////

	enum EChargeGroupDefinitionStrategies
	{
		eCGUndefined = -1,
		eCGAutoGenerate, //Charge groups are generated and assigned 'automatically' by appropriate algorithm 
		eCGTakeFromFile, //Index of charge group is taken from atom definition (the index is valid only within residue the atom belongs to!)		
		eCGAssignPerAtom, //For every atom, there is assigned its unique charge group (i.e. one atom - one charge group)
		eCGAssignPerResidue, //For every residue (or its subdivision on primary groups), there is assigned its unique charge group (i.e. actually one primary group - one charge group)
	};

	inline EChargeGroupDefinitionStrategies ChargeGroupDefinitionStrategy(const Common::ComString& strStrategy)
	{
		if (strStrategy == _ComStr("AUTO"))
		{
			return eCGAutoGenerate;
		}
		else if (strStrategy == _ComStr("FILE"))
		{
			return eCGTakeFromFile;
		}
		else if (strStrategy == _ComStr("ATOM"))
		{
			return eCGAssignPerAtom;
		}
		else if (strStrategy == _ComStr("RESIDUE"))
		{
			return eCGAssignPerResidue;
		}
		return eCGUndefined;
	}

	inline Common::ComString ChargeGroupDefinitionStrategyName(Common::EChargeGroupDefinitionStrategies strategy)
	{
		switch (strategy)
		{
			case eCGAutoGenerate:			return Common::ComString(_ComStr("AUTO")); break;
			case eCGTakeFromFile:			return Common::ComString(_ComStr("FILE")); break;
			case eCGAssignPerAtom:		return Common::ComString(_ComStr("ATOM")); break;
			case eCGAssignPerResidue:	return Common::ComString(_ComStr("RESIDUE")); break;
			default:									return Common::ComString(_ComStr("UNKNOWN")); break;
		};
	}

	////////////////////////////////////////////////////////////////////////////

	enum EFunctionalGroupTypes
	{
		eFuncGroupTypeUnknown = -1,
		eUNITED,
		eINTERMOLEC
	};

	inline EFunctionalGroupTypes FunctionalGroupType(const Common::ComString& strGroupType)
	{
		if (strGroupType == _ComStr("UNITED"))
		{
			return eUNITED;
		}
		else if (strGroupType == _ComStr("INTER-MOLEC"))
		{
			return eINTERMOLEC;
		}
		return eFuncGroupTypeUnknown;
	}

	inline Common::ComString FunctionalGroupTypeName(Common::EFunctionalGroupTypes groupType)
	{
		switch (groupType)
		{
			case eUNITED:			return Common::ComString(_ComStr("UNITED")); break;
			case eINTERMOLEC:	return Common::ComString(_ComStr("INTER-MOLEC")); break;
			default:					return Common::ComString(_ComStr("UNKNOWN")); break;
		};
	}

	////////////////////////////////////////////////////////////////////////////

	enum EFunctionalGroupSetIDs
	{
		eFuncSetIDUnknown = -1,
		eSetA,
		eSetB
	};

	inline EFunctionalGroupSetIDs FunctionalSetID(const Common::ComString& strSetID)
	{
		if (strSetID == _ComStr("A"))
		{
			return eSetA;
		}
		else if (strSetID == _ComStr("B"))
		{
			return eSetB;
		}
		return eFuncSetIDUnknown;
	}

	inline Common::ComString FunctionalSetIDName(Common::EFunctionalGroupSetIDs setID)
	{
		switch (setID)
		{
			case eSetA:	return Common::ComString(_ComStr("A")); break;
			case eSetB:	return Common::ComString(_ComStr("B")); break;
			default:		return Common::ComString(_ComStr("UNKNOWN")); break;
		};
	}

	/////////////////////////////////////////////////////////////////////////////

	enum EPolarProperty //1 for donor, 2 for acceptor, and 0 for n/a
	{
		eNotDA		= 0,
		eDonor		= 1,
		eAcceptor	= 2,
		eUnknownDA	= 3
	};

} //namespace Common
