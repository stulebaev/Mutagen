// mapping.cpp
#include "mapping.h"
#include <QtXml/QtXml>

bool MutationMap::save(const QString& filename) {
  QXmlStreamWriter xmlWriter;
  QFile file(filename);
  if (file.open(QIODevice::WriteOnly)) {
    xmlWriter.setDevice(&file);
    xmlWriter.setCodec("Windows-1251");
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("MutationMap");
    QString nameResA = molecularSystem.vecMolecules[0].name;
    QString nameResB = molecularSystem.vecMolecules[1].name;
    xmlWriter.writeAttribute("Title", nameResA + "-}" + nameResB);
    xmlWriter.writeStartElement("ResA");
    xmlWriter.writeCharacters(nameResA);
    xmlWriter.writeEndElement();
    xmlWriter.writeStartElement("ResB");
    xmlWriter.writeCharacters(nameResB);
    xmlWriter.writeEndElement();
    xmlWriter.writeStartElement("AtomsMapping");
    int count = (mappingList.size()/2) * 2;
    for (int i = 0; i < count; i += 2) {
      AtomIndex& indexA = mappingList[i];
      AtomIndex& indexB = mappingList[i+1];
      if (indexA.first > indexB.first) {
        AtomIndex temp = indexA;
        indexA = indexB;
        indexB = temp;
      }
      xmlWriter.writeStartElement("Atoms");
      xmlWriter.writeAttribute("A", molecularSystem.vecMolecules[indexA.first].vecAtoms[indexA.second].GetLabel());
      xmlWriter.writeAttribute("B", molecularSystem.vecMolecules[indexB.first].vecAtoms[indexB.second].GetLabel());
      xmlWriter.writeEndElement();//<Atoms>
    }
    xmlWriter.writeEndElement();//</AtomsMapping>
    xmlWriter.writeEndElement();//</MutationMap>
    xmlWriter.writeEndDocument();
    return true;
  }
  return false;
}

void MutationMap::processAtom(const AtomIndex& atomIndex) {
  int indx = mappingList.indexOf(atomIndex);
  if (indx == -1) { // adding new atom
    if (mappingList.size() % 2) {
      if (atomIndex.first == mappingList.last().first) return; // linked atoms can not be in one molecule
    }
    mappingList.append(atomIndex);
  }
  else { // this atom is already in the mutation map
    indx = (indx/2)*2;
    mappingList.removeAt(indx);
    mappingList.removeAt(indx); // remove also the linked atom
  }
//for (auto& index : mappingList) { qDebug() << "(" << index.first << "," << index.second << ")"; }
}

MutationMap mutationMap;
