#pragma once

#include "CommonLib/Exception.h"
#include "CommonLib/MolDefines.h" // definition of IndexType, BondIndexType

#include <cstring> //Definition of memset

namespace FileIO
{ //FileIO

	// CIOAtom
	// --------------------------------------------------------------------------
	// Class to represent information about molecule' atom stored in disk file.
	//
	//
	// Класс представляет информацию об атоме молекулы, загруженной из дискового файла.
	// Класс содержит агрегацию наборов полей, встречающихся в файлах поддерживаемых форматов (HIN, MOL, PDB, RTP)
	// Если в файле конкретного формата какое-то поле не встречается, при считывании файла оно остается незаполненным (заполненным по умолчанию).
	// Структура атома предусматривает возможность постадийного добавления информациии, в т.ч. 'слияния' данных об одном и том же атоме, загруженных из нескольких файлов.
	// Управление подобными действиями производится на уровне классов, представляющих молекулу или молекулярный файл.

	template <typename _PositionNumeric, typename _EnergyNumeric>
	class CIOAtomT
	{
		typedef CIOAtomT<_PositionNumeric, _EnergyNumeric> _ThisType;
		template <typename _ThisType> friend class CIOMoleculeT;
	public:
		typedef _PositionNumeric	PositionNumeric;
		typedef _EnergyNumeric		EnergyNumeric;
	public:
		typedef std::pair<Common::IndexType, Common::BondFlags> Bond;
	public:
		CIOAtomT()
		{
			Clear();
		}

		void Clear()
		{
			m_nID = (Common::IndexType)(-1);
			m_nSequenceNumber = (Common::IndexType)(-1);
			m_nElementID = (-1);
			m_nResSequenceNumber = MaxInvalidResidueNumber;
			m_cResInsertionCode = _ComStr('\0'); //' ';
			m_cResChainID = UndefinedChainLabel; //' ';
			m_x = m_y = m_z = (PositionNumeric)0.;
			m_vx = m_vy = m_vz = (PositionNumeric)0.;
			m_fx = m_fy = m_fz = (EnergyNumeric)0.;
			m_dCharge = (EnergyNumeric)0.;
			m_dFinalCharge = (EnergyNumeric)0.;
			m_dTemperature = (EnergyNumeric)0.;
			m_dMass = (EnergyNumeric)0.;
			m_cAtomFlags = (Common::AtomFlags)0;
			m_nLocalChargeGroupID = -1;
			m_nBondsNumber = (Common::BondIndexType)0;
			m_nLinksNumber = (Common::BondIndexType)0;
			memset(m_szFFType, 0, sizeof(m_szFFType));
			memset(m_szLabel, 0, sizeof(m_szLabel));
			memset(m_szResName, 0, sizeof(m_szResName));
		}

		Common::IndexType GetID() const 
		{
			return m_nID;
		}
		void SetID(Common::IndexType nAtomIndex)
		{
			m_nID = nAtomIndex;
		}
		Common::IndexType GetSequenceNumber() const
		{
			return m_nSequenceNumber;
		}
		void SetSequenceNumber(Common::IndexType nSequenceNumber)
		{
			m_nSequenceNumber = nSequenceNumber;
		}
		int GetElementID() const
		{
			return m_nElementID;
		}
		void SetElementID(int nElementID)
		{
			m_nElementID = nElementID;
		}
		const Common::ComChar* GetFFType() const
		{
			return m_szFFType;
		}
		void SetFFType(const Common::ComString& strFFType)
		{
			size_t len = (strFFType.length() + 1) * sizeof(Common::ComChar); //Include terminating '\0'
			memcpy(m_szFFType, strFFType.c_str(), len <= sizeof(m_szFFType) ? len : sizeof(m_szFFType));
		}
		void SetFFType(const Common::ComChar* szFFType) //Null-terminated C-string is expected
		{
			size_t len = (strlen(szFFType) + 1) * sizeof(Common::ComChar); //Include terminating '\0'
			memcpy(m_szFFType, szFFType, len <= sizeof(m_szFFType) ? len : sizeof(m_szFFType));
		}
		const Common::ComChar* GetLabel() const
		{
			return m_szLabel;
		}
		void SetLabel(const Common::ComString& strLabel)
		{
			size_t len = (strLabel.length() + 1) * sizeof(Common::ComChar); //Include terminating '\0'
			memcpy(m_szLabel, strLabel.c_str(), len <= sizeof(m_szLabel) ? len : sizeof(m_szLabel));
		}
		void SetLabel(const Common::ComChar* szLabel) //Null-terminated C-string is expected
		{
			size_t len = (strlen(szLabel) + 1) * sizeof(Common::ComChar); //Include terminating '\0'
			memcpy(m_szLabel, szLabel, len <= sizeof(m_szLabel) ? len : sizeof(m_szLabel));
		}
		Common::IndexType GetResSequenceNumber() const
		{
			return m_nResSequenceNumber;
		}
		void SetResSequenceNumber(Common::IndexType nResSequenceNumber)
		{
			m_nResSequenceNumber = nResSequenceNumber;
		}
		const Common::ComChar* GetResName() const
		{
			return m_szResName;
		}
		void SetResName(const Common::ComString& strResName)
		{
			size_t len = (strResName.length() + 1) * sizeof(Common::ComChar); //Include terminating '\0'
			memcpy(m_szResName, strResName.c_str(), len <= sizeof(m_szResName) ? len : sizeof(m_szResName));
		}
		void SetResName(const Common::ComChar* szResName) //Null-terminated C-string is expected
		{
			size_t len = (strlen(szResName) + 1) * sizeof(Common::ComChar); //Include terminating '\0'
			memcpy(m_szResName, szResName, len <= sizeof(m_szResName) ? len : sizeof(m_szResName));
		}
		Common::ComChar GetResInsertionCode() const
		{
			return m_cResInsertionCode;
		}
		void SetResInsertionCode(Common::ComChar cResInsertionCode)
		{
			m_cResInsertionCode = cResInsertionCode;
		}
		Common::ComChar GetResChainID() const
		{
			return m_cResChainID;
		}
		void SetResChainID(Common::ComChar cResChainID)
		{
			m_cResChainID = cResChainID;
		}
		PositionNumeric GetX() const
		{
			return m_x;
		}
		PositionNumeric GetY() const
		{
			return m_y;
		}
		PositionNumeric GetZ() const
		{
			return m_z;
		}
		template <typename _Point>
		void GetPosition(_Point& position) const
		{
			position.x = m_x;
			position.y = m_y;
			position.z = m_z;
		}
		void SetX(PositionNumeric dX)
		{
			m_x = dX;
		}
		void SetY(PositionNumeric dY)
		{
			m_y = dY;
		}
		void SetZ(PositionNumeric dZ)
		{
			m_z = dZ;
		}
		PositionNumeric GetVx() const
		{
			return m_vx;
		}
		PositionNumeric GetVy() const
		{
			return m_vy;
		}
		PositionNumeric GetVz() const
		{
			return m_vz;
		}
		template <typename _Vector>
		void GetVelocity(_Vector& velocity) const
		{
			velocity.x = m_vx;
			velocity.y = m_vy;
			velocity.z = m_vz;
		}
		void SetVx(PositionNumeric dVx)
		{
			m_vx = dVx;
		}
		void SetVy(PositionNumeric dVy)
		{
			m_vy = dVy;
		}
		void SetVz(PositionNumeric dVz)
		{
			m_vz = dVz;
		}
		EnergyNumeric GetFx() const
		{
			return m_fx;
		}
		EnergyNumeric GetFy() const
		{
			return m_fy;
		}
		EnergyNumeric GetFz() const
		{
			return m_fz;
		}
		template <typename _Vector>
		void GetForce(_Vector& force) const
		{
			force.x = m_fx;
			force.y = m_fy;
			force.z = m_fz;
		}
		void SetFx(EnergyNumeric dFx)
		{
			m_fx = dFx;
		}
		void SetFy(EnergyNumeric dFy)
		{
			m_fy = dFy;
		}
		void SetFz(EnergyNumeric dFz)
		{
			m_fz = dFz;
		}
		EnergyNumeric GetCharge() const
		{
			return m_dCharge;
		}
		void SetCharge(EnergyNumeric dCharge)
		{
			m_dCharge = dCharge;
		}
		EnergyNumeric GetFinalCharge() const
		{
			return m_dFinalCharge;
		}
		void SetFinalCharge(EnergyNumeric dFinalCharge)
		{
			m_dFinalCharge = dFinalCharge;
		}
		EnergyNumeric GetTemperature() const
		{
			return m_dTemperature;
		}
		void SetTemperature(EnergyNumeric dTemperature)
		{
			m_dTemperature = dTemperature;
		}
		EnergyNumeric GetMass() const
		{
			return ( m_dMass == (EnergyNumeric)0.
					&& m_nElementID >= Common::EPTElements::LP
					&& m_nElementID < Common::EPTElements::ePTEUnknown
					)
					? (EnergyNumeric)Common::g_PT[m_nElementID].m_dAtomicWeight
					: m_dMass; //If mass is 'initialized' [from file etc.), or element is invalid, then return m_dmass regardless of its correctness
		}
		void SetMass(EnergyNumeric dMass)
		{
			m_dMass = dMass;
		}
		Common::AtomFlags GetAtomFlags() const
		{
			return m_cAtomFlags;
		}
		void SetAtomFlags(Common::AtomFlags cAtomFlags) //Replace existing flags with new ones
		{
			m_cAtomFlags = cAtomFlags;
		}
		void PutAtomFlags(Common::AtomFlags cAtomFlags) //Add new flags to existing ones
		{
			m_cAtomFlags |= cAtomFlags;
		}

		int GetLocalChargeGroupID() const
		{
			return m_nLocalChargeGroupID;
		}
		void SetLocalChargeGroupID(int nLocalChargeGroup)
		{
			m_nLocalChargeGroupID = nLocalChargeGroup;
		}

		Common::BondIndexType GetBondsNumber() const
		{
			return m_nBondsNumber;
		}
		Common::BondIndexType GetLinksNumber() const
		{
			return m_nLinksNumber;
		}
		const Bond& GetBond(Common::BondIndexType bondIndex) const //Does not check for bounds...
		{
			return m_Bonds[bondIndex];
		}
		const Bond& BondAt(Common::BondIndexType bondIndex) const //Analog for GetBond() but checks for bounds...
		{
			if (bondIndex >= m_nLinksNumber)
			{
				Common::CException::Throw(__FILE__, __LINE__, _ComStr("CIOAtom::BondAt(): for atom %d, bond index %d is out of bonds range of %d"), m_nID, bondIndex, m_nLinksNumber);
			}
			return m_Bonds[bondIndex];
		}
		Common::IndexType& Neighbour(Common::BondIndexType bondIndex) //Does not check for bounds...
		{
			return m_Bonds[bondIndex].first;
		}
		const Common::IndexType& Neighbour(Common::BondIndexType bondIndex) const //Does not check for bounds...
		{
			return m_Bonds[bondIndex].first;
		}
		Common::BondFlags& BondFlags(Common::BondIndexType bondIndex) //Does not check for bounds...
		{
			return m_Bonds[bondIndex].second;
		}
		const Common::BondFlags& BondFlags(Common::BondIndexType bondIndex) const //Does not check for bounds...
		{
			return m_Bonds[bondIndex].second;
		}
		Common::BondFlags GetBondOrder(Common::BondIndexType bondIndex) const //Does not check for bounds...
		{
			return m_Bonds[bondIndex].second & Common::eBondOrdersMask;
		}
		Common::BondFlags BondOrderAt(Common::BondIndexType bondIndex) const //Analog for GetBondOrder() but checks for bounds...
		{
			if (bondIndex >= m_nLinksNumber)
			{
				Common::CException::Throw(__FILE__, __LINE__, _ComStr("CIOAtom::BondOrderAt(): for atom %d, bond index %d is out of bonds range of %d"), m_nID, bondIndex, m_nLinksNumber);
			}
			return m_Bonds[bondIndex].second & Common::eBondOrdersMask;
		}
		bool IsBonded(Common::IndexType nAtomID) const
		{
			for (Common::BondIndexType i = m_nLinksNumber; i; )
			{
				if (m_Bonds[--i].first == nAtomID)
				{
					return true;
				}
			}
			return false;
		}
		bool IsBonded(Common::IndexType nAtomID, Common::BondFlags& bondFlags) const
		{
			for (Common::BondIndexType i = m_nLinksNumber; i; )
			{
				if (m_Bonds[--i].first == nAtomID)
				{
					bondFlags = m_Bonds[i].second;
					return true;
				}
			}
			return false;
		}

	protected:
		bool addBond(Common::IndexType nAtomID, Common::BondFlags bondFlags, Common::EBondInsertionPolicies insertionPolicy = Common::eThrowIfExists)
		{
			//Check for bond duplication
			if (insertionPolicy != Common::eDoNotCheckForDuplication)
			{
				for (Common::BondIndexType i = m_nLinksNumber; i;)
				{
					if (m_Bonds[--i].first == nAtomID)
					{
						switch (insertionPolicy)
						{
						case Common::eOverrideIfExists:
							m_Bonds[i].second = bondFlags;
							return true;
						case Common::eSkipIfExists:
							return false;
						case Common::eThrowIfExists:
							{
								Common::CException::Throw(__FILE__, __LINE__, _ComStr("CIOAtom::addBond(): bond with these prameters already exists"));
							}
							return false;
						default:
							break;
						}
					}
				}
			}
			//Check for max bonds number
			if (m_nLinksNumber == MaxAtomLinksNumber)
			{
				Common::CException::Throw(__FILE__, __LINE__, _ComStr("CIOAtom::addBond(): for atom %d, bonds number already has the limit of %d"), m_nID, MaxAtomLinksNumber);
			}
			if (bondFlags & Common::eBondOrdersMask) //This is 'real' covalent bond. Insert if at the end of bond storage (before links, if any)
			{
				memmove(m_Bonds + (m_nBondsNumber + 1), m_Bonds + m_nBondsNumber, (m_nLinksNumber - m_nBondsNumber) * sizeof(Bond)); //Is it correct for memory alignment?
				m_Bonds[m_nBondsNumber].first = nAtomID;
				m_Bonds[m_nBondsNumber].second = bondFlags;
				++m_nBondsNumber;
				++m_nLinksNumber;
			}
			else //This is 'link'. Put it at the end of bonds array
			{
				m_Bonds[m_nLinksNumber].first = nAtomID;
				m_Bonds[m_nLinksNumber].second = bondFlags;
				++m_nLinksNumber;
			}
			return true;
		}

		void removeBond(Common::BondIndexType bondIndex)
		{
			if (bondIndex >= m_nLinksNumber)
			{
				Common::CException::Throw(__FILE__, __LINE__, _ComStr("CIOAtom::removeBond(): for atom %d, bond index %d is out of bonds range of %d"), m_nID, bondIndex, m_nLinksNumber);
			}
			if (bondIndex < (--m_nLinksNumber))
			{
				memmove(m_Bonds + bondIndex, m_Bonds + (bondIndex+1), (m_nLinksNumber - bondIndex) * sizeof(Bond)); //Is it correct for memory alignment?
			}
			if (bondIndex < m_nBondsNumber)
			{
				--m_nBondsNumber;
			}
		}

		bool removeBondWith(Common::IndexType nAtomID)
		{
			bool bRes = false;
			for (Common::BondIndexType i = m_nLinksNumber; i; )
			{
				if (m_Bonds[--i].first == nAtomID)
				{
					removeBond(i); //Overhead is additional check for bonds array ranges
					bRes = true; //Do not return immediately because there could be duplicated bonds with this nAtomID
				}
			}
			return bRes;
		}

	protected:
		Common::IndexType m_nID; //External key - ID of atom as stored in some external data structure
		Common::IndexType m_nSequenceNumber; //Atom sequence number as it is given in file. It can be not sequential within the atoms array record and potentially can be not unique without some extra info (e.g. chain id)
		int m_nElementID; //ID of atom chemical element as given in periodic table (1-based)
		Common::ComChar m_szFFType[5]; //[Symbolic] name of FF-type of atom, up to 4 characters
		Common::ComChar m_szLabel[5]; //Symbolic atom label (atom name as given in file), up to 4 characters

		int m_nResSequenceNumber; //Atom group sequence number (e.g. amino acid residue etc.) as it is given in file
		Common::ComChar m_szResName[5]; //Symbolic name of atom group (e.g. amino acid residue name) as it is given in file, up to 4 characters
		Common::ComChar m_cResInsertionCode; //'Insertion code' of atom group, used mainly in PDB-files
		Common::ComChar m_cResChainID; //Chain identifier

		PositionNumeric m_x, m_y, m_z; //Atom position (cartesian coordinates)
		PositionNumeric m_vx, m_vy, m_vz; //Atom velocity
		EnergyNumeric m_fx, m_fy, m_fz; //Atom force
		EnergyNumeric m_dCharge; //Fractional atom charge as it is given in file
		EnergyNumeric m_dFinalCharge; //Atom charge after assigning charge transfers (if any)
		//EnergyNumeric m_dPartialCharge; //Fractional atom charge without any charge transfers etc. Either integer or fractional like 1/n for symmertic groups
		EnergyNumeric m_dTemperature; //Temperature factor of the atom, used mainly in PDB-files
		EnergyNumeric m_dMass; //Mass of the atom
		Common::AtomFlags m_cAtomFlags; //Set of atom flags. Some of flag sets can be bitwise combined
		Common::IndexType m_nLocalChargeGroupID; //Index of charge group that the atom belongs to. Typically in files these charge groups are represented on residue basis, so the charge group id is unique only within current residue

		Common::BondIndexType m_nBondsNumber; //Number of covalent bonds with other atoms (cannot be larger than MacAtomBondsNumber)
		Common::BondIndexType m_nLinksNumber; //Number of any bonds/links (including H, SS, Ionic, dummy) with other atoms (cannot be larger than MaxAtomLinksNumber)
			//Be careful with bonds insertion/deletion: in atom, [covalent] bonds should go before [non-covalent] links
		Bond m_Bonds[MaxAtomLinksNumber]; //Index (0-based) of second atom (which owner atom is bonded/linked to)
											//Bond type (see BOND_DEFINITIONS)

	};
	typedef CIOAtomT<Common::NUMERIC, Common::NUMERIC> CIOAtom; //Stub

	template <typename _Atom>
	Common::ComString BuildAtomFullName(const _Atom& atom)
	{	//Function builds a literal identifier of atom containing its label, residue name, residue number and chain id (residue insertion code is not included!)
		//Should be kept consistent at least with with IO atom and topology atom to enable atoms matching by 'full name'
		Common::ComString strFullName = atom.GetLabel() 
			+ Common::ComString(_ComStr(".")) 
			+ atom.GetResName() 
			+ Common::ComString(_ComStr("(")) 
			+ _ToString(atom.GetResSequenceNumber())
			+ Common::ComString(_ComStr(")")) 
			+ atom.GetResChainID() //ResChainID can be '\0'
			;
		return strFullName;
	}

} //namespace FileIO
