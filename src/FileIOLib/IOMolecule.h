#pragma once

#include "IOAtom.h"

#include <vector>

namespace FileIO
{ //FileIO

	template <typename _Atom>
	class CIOMoleculeT
	{
		typedef CIOMoleculeT<_Atom> _ThisType;
	public:
		typedef _Atom Atom;
		typedef typename _Atom::Bond Bond;
		typedef typename std::vector<_Atom>::value_type ValueType;
	public:
		CIOMoleculeT()
			: m_eMoleculeType(Common::eMoleculeUnknown),
			m_nID(-1),
			m_strName(""),
			m_strForceField(""),
			m_bContentsChanged(false)
		{
		}
		virtual ~CIOMoleculeT()
		{
		}

		bool IsUndefined() const
		{
			return m_nID == -1
				&& m_eMoleculeType == Common::eMoleculeUnknown
				&& m_strName.empty()
				&& m_strForceField.empty()
				&& m_vecAtoms.empty()
				//&& m_vecComments.empty() //Do not treat molecule as 'defined' if it has only comments lines. Comments lines themselves are [duplicated] in molecular storage and so can be extracted from it, if necessary.
				//&& !m_bContentsChanged
				;
		}

		bool IsEmpty() const
		{
			return GetAtoms().empty();
		}

		void Clear()
		{
			m_nID = -1;
			m_eMoleculeType = Common::eMoleculeUnknown;
			m_strName.clear();
			m_strForceField.clear();
			m_vecComments.clear();
			m_vecAtoms.clear();
			m_bContentsChanged = true;
		}

		void Swap(_ThisType& otherMolecule)
		{
			std::swap(m_eMoleculeType, otherMolecule.m_eMoleculeType);
			std::swap(m_nID, otherMolecule.m_nID);
			m_strName.swap(otherMolecule.m_strName);
			m_strForceField.swap(otherMolecule.m_strForceField);
			m_vecAtoms.swap(otherMolecule.m_vecAtoms);
			m_vecComments.swap(otherMolecule.m_vecComments);
			std::swap(m_bContentsChanged, otherMolecule.m_bContentsChanged);
		}

		Common::EMoleculeDefinitions GetType() const
		{
			return m_eMoleculeType;
		}
		void SetType(Common::EMoleculeDefinitions type)
		{
			m_eMoleculeType = type;
		}

		int GetID() const
		{
			return m_nID;
		}
		void SetID(int nID)
		{
			m_nID = nID;
		}

		Common::ComString GetName() const
		{
			return m_strName;
		}
		void SetName(const Common::ComString& strName)
		{
			m_strName = strName;
		}

		Common::ComString GetForceField() const
		{
			return m_strForceField;
		}
		void SetForceField(const Common::ComString& strForceField)
		{
			m_strForceField = strForceField;
		}

		void AddComment(const Common::ComString& strCommentLine)
		{
			m_vecComments.push_back(strCommentLine);
		}
		template <typename _Container>
		void AddComments(const _Container& contComments)
		{
			m_vecComments.insert(m_vecComments.end(), contComments.begin(), contComments.end());
		}
		const std::vector<Common::ComString>& GetComments() const
		{
			return m_vecComments;
		}
		void ClearComments()
		{
			m_vecComments.clear();
		}

		typename Atom::EnergyNumeric GetMass() const
		{
			typename Atom::EnergyNumeric dMass = (typename Atom::EnergyNumeric)0.;
			for (typename std::vector<_Atom>::const_iterator ita = GetAtoms().begin(), itaEnd = GetAtoms().end(); ita != itaEnd; ++ita)
			{
				dMass += (*ita).GetMass();
			}
			return dMass;
		}

		std::vector<Common::ComString>& GetContainingMolGroups() //Get array of titles of molecular groups this molecule belongs to
		{
			return m_vecMolecularGroupNames;
		}
		const std::vector<Common::ComString>& GetContainingMolGroups() const
		{
			return m_vecMolecularGroupNames;
		}

		std::vector<_Atom>& GetAtoms() //Returns reference on internal array of atoms. Do not use it for hacks that can break atoms/bonds consistency!
		{
			return m_vecAtoms;
		}
		const std::vector<_Atom>& GetAtoms() const
		{
			return m_vecAtoms;
		}

		Common::IndexType GetAtomsNumber() const
		{
			return (Common::IndexType)m_vecAtoms.size();
		}

		_Atom& GetAtom(Common::IndexType nIndex) //Does not check for bounds...
		{
			return GetAtoms().at(nIndex); //Throws if index is out of range
		}
		const _Atom& GetAtom(Common::IndexType nIndex) const //Does not check for bounds...
		{
			return GetAtoms().at(nIndex); //Throws if index is out of range
		}

		Common::IndexType AddAtom(const _Atom& atom) //Returns index of newly inserted atom
		{
			Common::IndexType nIndex = (Common::IndexType)m_vecAtoms.size();
			m_vecAtoms.push_back(atom);
			m_vecAtoms.back().SetID(nIndex);
			m_bContentsChanged = true;
			return nIndex;
		}
		
		void RemoveAtom(Common::IndexType nAtomID)
		{
#ifdef _DEBUG
			if (nAtomID >= GetAtomsNumber())
			{
				Common::CException::Throw(__FILE__, __LINE__, _ComStr("CIOMolecule::RemoveAtom(): atom index %d is out of atoms range of %d"), nAtomID, GetAtomsNumber());
			}
#endif //_DEBUG
			Common::IndexType nID;
			Common::BondIndexType iBond, nBonds;
			//Remove bonds of atom to be deleted
			_Atom& atom = m_vecAtoms[nAtomID];
			for (iBond = atom.GetLinksNumber(); iBond; )
			{
				m_vecAtoms[atom.GetBond(--iBond).first].removeBondWith(nAtomID);
			}
			//Remove atom itself
			m_vecAtoms.erase(m_vecAtoms.begin() + nAtomID);
			//Correct atom IDs and bonded atoms indices for remaining atoms
			for (typename std::vector<_Atom>::iterator it = m_vecAtoms.begin(), itEnd = m_vecAtoms.end(); it < itEnd; ++it)
			{
				if ((nID = it->GetID()) > nAtomID) //There must be no equality of nID == nAtomID
				{
					it->SetID(--nID);
				}
				for (iBond = it->GetLinksNumber(); iBond; )
				{
					const Bond& bond= it->GetBond(--iBond);
					if (bond.first > nAtomID) //There must be no equality of bond.first == nAtomID
					{
						--(const_cast<Bond&>(bond).first);
					}
				}
			}
			m_bContentsChanged = true;
		}

		bool AreBonded(Common::IndexType nAtomID1, Common::IndexType nAtomID2, Common::BondFlags& bondFlags = Common::BondFlags()) const
		{
#ifdef _DEBUG
			Common::IndexType nAtomsNumber = GetAtomsNumber();
			if (nAtomID1 >= nAtomsNumber)
			{
				Common::CException::Throw(__FILE__, __LINE__, _ComStr("CIOMolecule::AreBonded(): atom index1 %d is out of atoms range of %d"), nAtomID1, GetAtomsNumber());
			}
			if (nAtomID2 >= nAtomsNumber)
			{
				Common::CException::Throw(__FILE__, __LINE__, _ComStr("CIOMolecule::AreBonded(): atom index2 %d is out of atoms range of %d"), nAtomID2, GetAtomsNumber());
			}
#endif //_DEBUG
			return m_vecAtoms.at(nAtomID1).IsBonded(nAtomID2, bondFlags);
		}

		Common::BondFlags operator () (Common::IndexType nAtomID1, Common::IndexType nAtomID2) const //Returns bond type between atoms nAtomID1 and nAtomID2 if they are bonded, and eBondDummy otherwise
		{
#ifdef _DEBUG
			Common::IndexType nAtomsNumber = GetAtomsNumber();
			if (nAtomID1 < 0 || nAtomID1 >= nAtomsNumber)
			{
				Common::CException::Throw(__FILE__, __LINE__, _ComStr("CIOMolecule::AreBonded(): atom index1 %d is out of atoms range of %d"), nAtomID1, GetAtomsNumber());
			}
			if (nAtomID2 < 0 || nAtomID2 >= nAtomsNumber)
			{
				Common::CException::Throw(__FILE__, __LINE__, _ComStr("CIOMolecule::AreBonded(): atom index2 %d is out of atoms range of %d"), nAtomID2, GetAtomsNumber());
			}
#endif //_DEBUG
			Common::BondFlags bondFlags = Common::eBondDummy;
			m_vecAtoms[nAtomID1].IsBonded(nAtomID2, bondFlags);
			return bondFlags;
		}

		Common::BondIndexType GetAllBondsNumber(Common::IndexType nAtomID) const //Returns the total number of bonds of given atom with atoms including bonds that are with atoms out of this molecule
		{
			if (nAtomID < 0 || nAtomID >= GetAtomsNumber())
			{
				return (Common::BondIndexType)(-1);
			}
			return GetAtom(nAtomID).GetBondsNumber();
		}

		Common::BondIndexType GetOwnBondsNumber(Common::IndexType nAtomID) const //Returns the number of bonds of given atom with atoms that belong to this molecule
		{
			Common::BondIndexType nOwnBonds(0);
			Common::IndexType nAtoms = GetAtomsNumber();
			if (nAtomID < 0 || nAtomID >= nAtoms)
			{
				return (Common::BondIndexType)(-1);
			}
			const Atom& atom = GetAtom(nAtomID); // Throws if index is out of range
			for (Common::BondIndexType iBond = 0, nBonds = atom.GetBondsNumber(); iBond < nBonds; ++iBond)
			{
				if (atom.Neighbour(iBond) >= 0 && atom.Neighbour(iBond) < nAtoms)
				{
					++nOwnBonds;
				}
			}
			return nOwnBonds;
		}

		bool GetOwnNeighbourAtom(Common::IndexType nAtomID, Common::BondIndexType bondIndex, Atom* &pAtom, Common::BondFlags& bondFlags) //Returns atom which is bondIndex-th connection of atom with index of nAtomID that belongs to this molecule
		{
			pAtom = NULL;
			bondFlags = Common::eBondDummy;
			Common::BondIndexType nOwnBondsCounter(0);
			Common::IndexType nAtoms = GetAtomsNumber();
			if (nAtomID < 0 || nAtomID >= nAtoms)
			{
				return false;
			}
			Atom& atom = GetAtom(nAtomID); // Throws if index is out of range
			for (Common::BondIndexType iBond = 0, nBonds = atom.GetBondsNumber(); iBond < nBonds; ++iBond)
			{
				if (atom.Neighbour(iBond) >= 0 && atom.Neighbour(iBond) < nAtoms)
				{
					if (nOwnBondsCounter == bondIndex)
					{
						pAtom = &m_vecAtoms[atom.Neighbour(iBond)];
						bondFlags = atom.BondFlags(iBond);
						return true;
					}
					else
					{
						++nOwnBondsCounter;
					}
				}
			}
			return false;
		}

		bool GetOwnNeighbourAtom(Common::IndexType nAtomID, Common::BondIndexType bondIndex, const Atom* &pAtom, Common::BondFlags& bondFlags) const //Returns atom which is bondIndex-th connection of atom with index of nAtomID that belongs to this molecule
		{
			pAtom = NULL;
			bondFlags = Common::eBondDummy;
			Common::BondIndexType nOwnBondsCounter(0);
			Common::IndexType nAtoms = GetAtomsNumber();
			if (nAtomID < 0 || nAtomID >= nAtoms)
			{
				return false;
			}
			const Atom& atom = GetAtom(nAtomID); // Throws if index is out of range
			for (Common::BondIndexType iBond = 0, nBonds = atom.GetBondsNumber(); iBond < nBonds; ++iBond)
			{
				if (atom.Neighbour(iBond) >= 0 && atom.Neighbour(iBond) < nAtoms)
				{
					if (nOwnBondsCounter == bondIndex)
					{
						pAtom = &m_vecAtoms[atom.Neighbour(iBond)];
						bondFlags = atom.BondFlags(iBond);
						return true;
					}
					else
					{
						++nOwnBondsCounter;
					}
				}
			}
			return false;
		}

		Common::IndexType GetAllNeighbourAtomIndexInMol(Common::IndexType nAtomID, Common::BondIndexType bondIndex) const //Returns index in the molecule of atom which is bondIndex-th connection of atom with index of nAtomID (including that with atom out of this molecule)
		{
#ifdef _DEBUG
			if (nAtomID < 0 || nAtomID >= GetAtomsNumber())
			{
				Common::CException::Throw(__FILE__, __LINE__, _ComStr("CIOMolecule::GetNeighbourAtomIndex(): atom index %d is out of atoms range of %d"), nAtomID, GetAtomsNumber());
			}
			Common::BondIndexType nBonds = m_vecAtoms.at(nAtomID).GetBondsNumber();
			if (bondIndex >= nBonds)
			{
				Common::CException::Throw(__FILE__, __LINE__, _ComStr("CIOMolecule::GetNeighbourAtomIndex(): for atom %d, bond index %d is out of bonds range of %d"), nAtomID, bondIndex, nBonds);
			}
#endif //_DEBUG
			return m_vecAtoms.at(nAtomID).BondAt(bondIndex).first;
		}

		bool AddBond(Common::IndexType nAtomID1, Common::IndexType nAtomID2, Common::BondFlags bondFlags, Common::EBondInsertionPolicies insertionPolicy = Common::eThrowIfExists)
		{
#ifdef _DEBUG
			Common::IndexType nAtomsNumber = GetAtomsNumber();
			if (nAtomID1 >= nAtomsNumber)
			{
				Common::CException::Throw(__FILE__, __LINE__, _ComStr("CIOMolecule::AddBond(): atom index1 %d is out of atoms range of %d"), nAtomID1, GetAtomsNumber());
			}
			if (nAtomID2 >= nAtomsNumber)
			{
				Common::CException::Throw(__FILE__, __LINE__, _ComStr("CIOMolecule::AddBond(): atom index2 %d is out of atoms range of %d"), nAtomID2, GetAtomsNumber());
			}
#endif //_DEBUG
			if ( m_vecAtoms[nAtomID1].addBond(nAtomID2, bondFlags, insertionPolicy)
				&& m_vecAtoms[nAtomID2].addBond(nAtomID1, bondFlags, insertionPolicy) )
			{
				m_bContentsChanged = true;
				return true;
			}
			else //Stub. Does not indicate if one atom added the bond but another one did not
			{
				return false;
			}
		}

		bool RemoveBond(Common::IndexType nAtomID1, Common::IndexType nAtomID2)
		{
#ifdef _DEBUG
			IndexType nAtomsNumber = GetAtomsNumber();
			if (nAtomID1 >= nAtomsNumber)
			{
				Common::CException::Throw(__FILE__, __LINE__, _ComStr("CIOMolecule::RemoveBond(): atom index1 %d is out of atoms range of %d"), nAtomID1, GetAtomsNumber());
			}
			if (nAtomID2 >= nAtomsNumber)
			{
				Common::CException::Throw(__FILE__, __LINE__, _ComStr("CIOMolecule::RemoveBond(): atom index2 %d is out of atoms range of %d"), nAtomID2, GetAtomsNumber());
			}
#endif //_DEBUG
			if ( m_vecAtoms[nAtomID1].removeBond(nAtomID2)
				&& m_vecAtoms[nAtomID2].removeBond(nAtomID1) )
			{
				m_bContentsChanged = true;
				return true;
			}
			else //Stub. Does not indicate if one atom removed the bond but another did not
			{
				return false;
			}
		}

	protected:
		virtual bool reviseContents() const //Internal function to revise and repair inconsistences, if any
		{
			m_bContentsChanged = false; //Stub
			return true;
		}

	protected:
		Common::EMoleculeDefinitions m_eMoleculeType;
		int m_nID;
		Common::ComString m_strName;
		Common::ComString m_strForceField; //String with ForceField name is it comes from file (if applicable). Is it necessary here?
	protected:
		std::vector<_Atom> m_vecAtoms;
		std::vector<Common::ComString> m_vecComments;
		std::vector<Common::ComString> m_vecMolecularGroupNames; //Array of names of molecular groups this molecule belongs to
		mutable bool m_bContentsChanged; //Internal flag for whether something has changed in contents (atoms/bonds) since the last [revision]
	};

	typedef CIOMoleculeT<CIOAtom> CIOMolecule;

} //namespace FileIO
