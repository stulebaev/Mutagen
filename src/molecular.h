// molecular.h
#ifndef MOLECULAR_H
#define MOLECULAR_H

#include <QtCore/QVector>
#include <QtCore/QString>
#include <QtGui/QVector3D>
#include "FileIOLib/FileIOHIN.h"

typedef FileIO::CIOAtom Atom;

struct Molecule {
  Molecule() {
    minimum = QVector3D(+FLT_MAX, +FLT_MAX, +FLT_MAX);
    maximum = QVector3D(-FLT_MAX, -FLT_MAX, -FLT_MAX);
  }
  QString name;
  QVector3D minimum, maximum;
  QVector<Atom> vecAtoms;
};

class MolecularSystem {
public:
  MolecularSystem() { representation = LINE; }
  ~MolecularSystem() { clear(); }
  void clear();
  void changeRepresentation();
  bool load(const QString& filename);
  void calculateScope();
public:
  int numAtoms;
  double diameter;
  QVector3D center;
  enum { INVISIBLE = 0, LINE, STICK, BALL_AND_STICK, VDW, SES, BACKBONE } representation;
  QVector<Molecule> vecMolecules;
};

extern MolecularSystem molecularSystem;

#endif // MOLECULAR_H
