#pragma once

#include "Typedefs.h"

namespace Common
{ //namespace Common

	enum EPTElements
	{
		ePTEUndefined = -1, //Should be the first and equal to -1
		LP = 0, //Lone [electronic] pair
		H,																	He,
		Li,	Be,											B,	C,	N,	O,	F,	Ne,
		Na,	Mg,											Al,	Si,	P,	S,	Cl,	Ar,
		K,	Ca,	Sc,	Ti,	V,	Cr,	Mn,	Fe,	Co,	Ni,	Cu,	Zn,	Ga,	Ge,	As,	Se,	Br,	Kr,
		Rb,	Sr,	Y,	Zr,	Nb,	Mo,	Tc,	Ru,	Rh,	Pd,	Ag,	Cd,	In,	Sn,	Sb,	Te,	I,	Xe,
		Cs,	Ba,																	La,	Ce,	Pr, Nd,	Pm,	Sm,	Eu,	Gd,	Tb,	Dy,	Ho,	Er,	Tm,	Yb,
				Lu,	Hf,	Ta,	W,	Re,	Os,	Ir,	Pt,	Au,	Hg,	Tl,	Pb,	Bi,	Po,	At,	Rn,
		Fr,	Ra,																	Ac,	Th,	Pa,	U,	Np,	Pu,	Am,	Cm,	Bk,	Cf,	Es,	Fm,	Md,	No,
				Lr,	Rf,	Db,	Sg,	Bh,	Hs,	Mt,	Ds,	Rg,	Uub,Uut,Uuq,Uup,Uuh,Uus,Uuo,
		ePTENumberOfElements, //Should be the last
		ePTEUnknown = ePTENumberOfElements
	};

	//Periodic table of chemical elements
	class CPeriodicTable
	{
	public:

		class Element
		{
		public:
			Element(int nNumber, enum EPTElements eSymbol, int nPeriod, int nGroup, Common::ComChar cBlock, double dAtomicWeight, const Common::ComChar* szFullName, const Common::ComChar* szElement, double dAtomicRadius, double dIonicRadius,	double dCovalentRadius,	double dVdWRadius,	double dCrystalRadius, double dElectronegativity)
			{
				set(nNumber, eSymbol, nPeriod, nGroup, cBlock, dAtomicWeight, szFullName, szElement, dAtomicRadius, dIonicRadius,	dCovalentRadius,	dVdWRadius,	dCrystalRadius, dElectronegativity);
			}

		protected:
			void set(int nNumber, enum EPTElements eSymbol, int nPeriod, int nGroup, Common::ComChar cBlock, double dAtomicWeight, const Common::ComChar* szFullName, const Common::ComChar* szElement, double dAtomicRadius, double dIonicRadius,	double dCovalentRadius,	double dVdWRadius,	double dCrystalRadius, double dElectronegativity)
			{
				m_nNumber = nNumber;
				m_eSymbol = eSymbol;
				m_nPeriod = nPeriod;
				m_nGroup = nGroup;
				m_cBlock = cBlock;
				m_dAtomicWeight = dAtomicWeight;
				m_strFullName = szFullName; //Name of chemical element
				m_strElement = szElement; //'Symbol' of chemical element
				m_dAtomicRadius = dAtomicRadius;
				m_dIonicRadius = dIonicRadius;
				m_dCovalentRadius = dCovalentRadius;
				m_dVdWRadius = dVdWRadius;
				m_dCrystalRadius = dCrystalRadius;
				m_dElectronegativity = dElectronegativity;
			}

		public:
			int m_nNumber;
			enum EPTElements m_eSymbol;
			int m_nPeriod;
			int m_nGroup;
			Common::ComChar m_cBlock;
			double m_dAtomicWeight;
			Common::ComString m_strFullName; //Name of chemical element
			Common::ComString m_strElement; //'Symbol' of chemical element
			double m_dAtomicRadius;
			double m_dIonicRadius;
			double m_dCovalentRadius;
			double m_dVdWRadius;
			double m_dCrystalRadius;
			double m_dElectronegativity;
		};

		Element& operator[] (int nNumber) const
		{
			if ( nNumber <  int(LP)
				|| nNumber >= int(ePTENumberOfElements)
				)
				nNumber = (int)ePTEUnknown;
			return m_Elements[nNumber];
		}

		Element& operator[] (EPTElements eSymbol) const
		{
			if ( int(eSymbol) <  int(LP)
				|| int(eSymbol) >= int(ePTENumberOfElements)
				)
				eSymbol = ePTEUnknown;
			return m_Elements[int(eSymbol)];
		}

		Element& operator[] (const Common::ComString& strElement) const
		{
			EPTElements eSymbol = ePTEUnknown;

			for (int i = int(LP); i < int (ePTENumberOfElements); ++i)
			{
#if defined( __GNUC__ ) || defined (__linux__)
	#if defined _ONEBYTECHARACTERS
				if (!strcasecmp(strElement.c_str(), m_Elements[EPTElements(i)].m_strElement.c_str()))
	#elif defined _TWOBYTECHARACTERS
				if (!wcscasecmp(strElement.c_str(), m_Elements[EPTElements(i)].m_strElement.c_str()))
	#endif
#else
	#if defined _ONEBYTECHARACTERS
				if (!_stricmp(strElement.c_str(), m_Elements[EPTElements(i)].m_strElement.c_str()))
	#elif defined _TWOBYTECHARACTERS
				if (!_wcsicmp(strElement.c_str(), m_Elements[EPTElements(i)].m_strElement.c_str()))
	#endif
#endif
				{
					eSymbol = EPTElements(i);
					break;
				}
			}
			return m_Elements[eSymbol];
		}

	private:
		static Element m_Elements[ePTENumberOfElements + 1];
	public:

	};

	extern CPeriodicTable g_PT;

	inline std::basic_istream<Common::ComChar>& operator >> (std::basic_istream<Common::ComChar>& is, EPTElements& eSymbol)
	{
		Common::ComString strBuffer;
		is >> strBuffer;
		eSymbol = g_PT[strBuffer].m_eSymbol;
		return is;
	}
	inline std::basic_ostream<Common::ComChar>& operator << (std::basic_ostream<Common::ComChar>& os, const EPTElements& eSymbol)
	{
		os << (g_PT[eSymbol].m_strElement.c_str());
		return os;
	}

} //namespace Common
