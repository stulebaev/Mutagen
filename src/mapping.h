// mapping.h
#ifndef MAPPING_H
#define MAPPING_H

#include "molecular.h"

#include <QtCore/QPair>
#include <QtCore/QList>
#include <QtCore/QString>

typedef QPair<int,int> AtomIndex;

class MutationMap {
public:
  QList<AtomIndex> mappingList;
  bool save(const QString& filename);
  void processAtom(const AtomIndex& atomIndex);
};

extern MutationMap mutationMap;

#endif // MAPPING_H
