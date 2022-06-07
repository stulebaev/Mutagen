// molecular.cpp
#include "molecular.h"

MolecularSystem molecularSystem;

void MolecularSystem::clear() {
  vecMolecules.clear();
  representation = LINE;
  diameter = 0.0;
  center = QVector3D();
}

void MolecularSystem::changeRepresentation() {
  if (representation == LINE) { representation = VDW; return; }
  if (representation == VDW) { representation = LINE; return; }
}

bool MolecularSystem::load(const QString& filename) {
  FileIO::CFileIOHIN hinReader;
  bool loaded = hinReader.ReadContents(filename.toStdString());
  if (!loaded) return false;
  for (auto& hinMolecule : hinReader.GetMolecules()) {
    Molecule molecule;
    molecule.name = hinMolecule.get()->GetName().c_str();
    for (auto& atom : hinMolecule.get()->GetAtoms()) {
      molecule.vecAtoms.push_back(atom);
      double x = atom.GetX();
      double y = atom.GetY();
      double z = atom.GetZ();
      if (molecule.minimum.x() > x) molecule.minimum.setX(x);
      if (molecule.minimum.y() > y) molecule.minimum.setY(y);
      if (molecule.minimum.z() > z) molecule.minimum.setZ(z);
      if (molecule.maximum.x() < x) molecule.maximum.setX(x);
      if (molecule.maximum.y() < y) molecule.maximum.setY(y);
      if (molecule.maximum.z() < z) molecule.maximum.setZ(z);
    }
    vecMolecules.push_back(molecule);
  }
  return loaded;
}

void MolecularSystem::calculateScope() {
  numAtoms = 0;
  double sysMinX, sysMinY, sysMinZ;
  double sysMaxX, sysMaxY, sysMaxZ;
  sysMinX = sysMinY = sysMinZ = +DBL_MAX;
  sysMaxX = sysMaxY = sysMaxZ = -DBL_MAX;
  for (auto& molecule : molecularSystem.vecMolecules) {
    if (sysMinX > molecule.minimum.x()) sysMinX = molecule.minimum.x();
    if (sysMinY > molecule.minimum.y()) sysMinY = molecule.minimum.y();
    if (sysMinZ > molecule.minimum.z()) sysMinZ = molecule.minimum.z();
    if (sysMaxX < molecule.maximum.x()) sysMaxX = molecule.maximum.x();
    if (sysMaxY < molecule.maximum.y()) sysMaxY = molecule.maximum.y();
    if (sysMaxZ < molecule.maximum.z()) sysMaxZ = molecule.maximum.z();
    numAtoms += molecule.vecAtoms.count();
  }
  center.setX((sysMaxX + sysMinX)/2);
  center.setY((sysMaxY + sysMinY)/2);
  center.setZ((sysMaxZ + sysMinZ)/2);
  double edgeX = sysMaxX - sysMinX;
  double edgeY = sysMaxY - sysMinY;
  double edgeZ = sysMaxZ - sysMinZ;
  diameter = sqrt(edgeX*edgeX + edgeY*edgeY + edgeZ*edgeZ);
}
