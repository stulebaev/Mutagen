// scene3D.cpp
#include "scene3D.h"
#include "CommonLib/PeriodicTable.h"
#include <QtGui/QMouseEvent>

void drawMapLinks();

Scene3D::Scene3D(QWidget* parent) : QGLWidget(parent) {
  projectionType = Perspective;
  showAxes = true;
  atomLabelFont = QFont("Helvetica", 10); //"Arial", "Times", "Courier"
  setInitialViewPoint();
}

void Scene3D::initializeGL()
{
  //glClearColor(0.0, 0.0, 0.0, 0.0);
  glClearColor(0.3f, 0.3f, 0.3f, 1.0f); // gray background
  GLfloat lightAmbient[] = {0.5f, 0.5f, 0.5f, 1.0f};
  GLfloat lightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
  GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
  GLfloat lightPosition[] = {0.0f, 1.0f, 0.0f, 0.0f};
  glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glShadeModel(GL_SMOOTH);
  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
}

void Scene3D::resizeGL(int width, int height) {
  if (height < 1) height = 1;
  glViewport(0, 0, width, height);
  applyProjection();
}

void Scene3D::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  applyModelview();
  drawScene();
  if (!mutationMap.mappingList.isEmpty()) drawMapLinks();
  if (showAxes) drawÑoordinateAxes();
}

void Scene3D::wheelEvent(QWheelEvent* event) {
  zoom(-MOUSE_WHEEL_SPEED*event->delta());
  updateGL();
}

void Scene3D::mouseMoveEvent(QMouseEvent* event) {
  Qt::MouseButtons state = event->buttons();
  if (state & Qt::LeftButton) {
    double deltaX = (double)(event->x() - lastMousePosition.x());
    double deltaY = (double)(event->y() - lastMousePosition.y());
    // for interactive use, we should switch the X and Y axes
    rotate(deltaY, deltaX, 0);
  }
  if (state & Qt::RightButton) {
    int indx = pointedMolecule(event->pos());
    if (indx > -1) {
      // translate the molecule following mouse movement
      double z = project(viewPoint).z();
      QVector3D toPos = unproject(event->x(), event->y(), z);
      QVector3D fromPos = unproject(lastMousePosition.x(), lastMousePosition.y(), z);
      QVector3D shift = toPos - fromPos;
      for (auto& atom : molecularSystem.vecMolecules[indx].vecAtoms) {
        atom.SetX(atom.GetX() + shift.x());
        atom.SetY(atom.GetY() + shift.y());
        atom.SetZ(atom.GetZ() + shift.z());
      }
      molecularSystem.vecMolecules[indx].minimum += shift;
      molecularSystem.vecMolecules[indx].maximum += shift;
    }
  }
  lastMousePosition = event->pos();
  updateGL();
}

void Scene3D::mousePressEvent(QMouseEvent* event) {
  Qt::MouseButtons state = event->buttons();
  /*if (state & Qt::LeftButton) {
    AtomIndex atomIndex = pickAtom(event->pos());
    if (atomIndex != AtomIndex(-1,-1)) {
      Atom& atom = molecularSystem.vecMolecules[atomIndex.first].vecAtoms[atomIndex.second];
      auto flags = atom.GetAtomFlags();
      flags ^= Common::eAtomIsSelected;
      atom.SetAtomFlags(flags);
      updateGL();
    }
  }*/
  if (state & Qt::RightButton) {
    setCursor(Qt::SizeAllCursor);
  }
  if (state == Qt::MiddleButton) {
    molecularSystem.changeRepresentation();
    updateGL();
  }
  lastMousePosition = event->pos();
}

void Scene3D::mouseReleaseEvent(QMouseEvent* event) {
  Q_UNUSED(event)
  setCursor(Qt::ArrowCursor);
}

void Scene3D::mouseDoubleClickEvent(QMouseEvent* event) {
  if (event->buttons() == Qt::LeftButton) {
    if (molecularSystem.vecMolecules.size() == 2) {
      AtomIndex atomIndex = pickAtom(event->pos());
      if (atomIndex != AtomIndex(-1,-1)) {
        mutationMap.processAtom(atomIndex);
        updateGL();
      }
    }
  }
}

void Scene3D::keyPressEvent(QKeyEvent* event) {
  switch (event->key()) {
    case Qt::Key_Up:
      if (event->modifiers() & Qt::ShiftModifier)
        translate(0, -5.0);
      else
        rotate(-5.0, 0, 0);
      break;
    case Qt::Key_Down:
      if (event->modifiers() & Qt::ShiftModifier)
        translate(0, 5.0);
      else
        rotate(5.0, 0, 0);
      break;
    case Qt::Key_Right:
      if (event->modifiers() & Qt::ShiftModifier)
        translate(5.0, 0);
      else
        rotate(0, 5.0, 0);
      break;
    case Qt::Key_Left:
      if (event->modifiers() & Qt::ShiftModifier)
        translate(-5.0, 0);
      else
        rotate(0, -5.0, 0);
      break;
    case Qt::Key_PageUp:
      zoom(ZOOM_STEP);
      break;
    case Qt::Key_PageDown:
      zoom(-ZOOM_STEP);
      break;
    case Qt::Key_Home:
      setInitialViewPoint();
      break;
    case Qt::Key_Asterisk:
      molecularSystem.changeRepresentation();
      break;
  }
  updateGL();
}

void Scene3D::applyModelview() {
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(modelviewMatrix.data());
}

void Scene3D::applyProjection() {
  double aspectRatio = (double)width()/height();
  double range = qMax(molecularSystem.diameter, 256*ZOOM_STEP);
  double distanceToCenter = (modelviewMatrix*viewPoint).length();
  double zNear = qMax(CAMERA_NEAR_DISTANCE, distanceToCenter - range);
  double zFar = distanceToCenter + range;

  projectionMatrix.setToIdentity();
  if (projectionType == Perspective) {
    projectionMatrix.perspective(angleOfView, aspectRatio, zNear, zFar);
  }
  if (projectionType == Orthographic) {
    const double halfHeight = orthoScale * range;
    const double halfWidth = halfHeight * aspectRatio;
    projectionMatrix.ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, zNear, zFar);
  }
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(projectionMatrix.data());
}

void Scene3D::zoom(double delta) {
  QVector3D transformedCenter = modelviewMatrix * viewPoint;
  double distanceToCenter = transformedCenter.length();
  double t = delta;
  double u = 2*CAMERA_NEAR_DISTANCE/distanceToCenter - 1.0;
  if (t < u) t = u;
  if (projectionType == Perspective) {
    QMatrix4x4 translation;
    translation.setColumn(3, QVector4D((transformedCenter * t), 1.0));
    modelviewMatrix = translation * modelviewMatrix;
  }
  if (projectionType == Orthographic) {
    orthoScale *= (1.0 + t);
  }
}

void Scene3D::translate(double deltaX, double deltaY) {
  double z = project(viewPoint).z();
  QVector3D fromPos = unproject(0, 0, z);
  QVector3D toPos = unproject(deltaX, deltaY, z);
  modelviewMatrix.translate(toPos - fromPos);
}

void Scene3D::rotate(double angleX, double angleY, double angleZ) {
  modelviewMatrix.translate(viewPoint);
  QVector3D xAxis = modelviewMatrix.row(0).toVector3D();
  QVector3D yAxis = modelviewMatrix.row(1).toVector3D();
  QVector3D zAxis = modelviewMatrix.row(2).toVector3D();
  modelviewMatrix.rotate(angleX*ROTATION_SPEED, xAxis);
  modelviewMatrix.rotate(angleY*ROTATION_SPEED, yAxis);
  modelviewMatrix.rotate(angleZ*ROTATION_SPEED, zAxis);
  modelviewMatrix.translate(-viewPoint);
}

QVector3D Scene3D::project(const QVector3D& v) {
  QVector4D p(v);
  p.setW(1.0);
  p = (projectionMatrix*modelviewMatrix*p).toVector3DAffine();
  p.setX(width()*(p.x()+1)/2);
  p.setY(height()*(p.y()+1)/2);
  p.setZ((p.z()+1)/2);
  return QVector3D(p);
}

QVector3D Scene3D::unproject(double x, double y, double z) {
  QVector4D v;
  v.setW(1.0);
  v.setX(2.0*x/width() - 1.0);
  v.setY(1.0 - 2.0*y/height()); // simultaneously invert Y axis
  v.setZ(2.0*z - 1.0);
  return ((projectionMatrix*modelviewMatrix).inverted()*v).toVector3DAffine();
}

AtomIndex Scene3D::pickAtom(const QPoint& pos) {
  QVector3D vEnd = unproject(pos.x(), pos.y(), -1.0/*far plane*/);
  QVector3D vOrg = unproject(pos.x(), pos.y(), +1.0/*near plane*/);
  QVector3D vDir = (vEnd - vOrg).normalized();
  QVector3D v;

  double radiusSquared = ATOM_CHECK_INTERVAL*ATOM_CHECK_INTERVAL;
  bool atomRadiusVariable = (molecularSystem.representation != molecularSystem.LINE);
  double tCurrent = 0.0;
  AtomIndex picked(-1,-1);
  int moleculeIndex = -1;
  for (auto& molecule : molecularSystem.vecMolecules) {
    moleculeIndex++;
    int atomIndex = -1;
    for (auto& atom : molecule.vecAtoms) {
      atomIndex++;
      if (atomRadiusVariable) {
        radiusSquared = Common::g_PT[atom.GetElementID()].m_dVdWRadius;
        radiusSquared = radiusSquared*radiusSquared;
      }
      v = vOrg - QVector3D(atom.GetX(), atom.GetY(), atom.GetZ());
      double B = QVector3D::dotProduct(vDir, v);
      double det;
      if ( (B < 0) // åñëè îáúåêò íàõîäèòñÿ çà ïåðåäíåé âèäîâîé ïëîñêîñòüþ (near)
        && ((det = (B*B - v.lengthSquared()) + radiusSquared) > 0) // if distance from ray to sphere is less than radius
         )
      {
        double t = -B + sqrt(det);
        if (t > tCurrent) {
          tCurrent = t;
          picked = qMakePair(moleculeIndex, atomIndex);
        }
      }
    }
  }
  return picked;
}

bool intersectBox(const QVector3D& org, const QVector3D& ray, const QVector3D& minimum, const QVector3D& maximum, double& t) {
  double t1 = (minimum.x() - org.x())/ray.x();
  double t2 = (maximum.x() - org.x())/ray.x();
  double t3 = (minimum.y() - org.y())/ray.y();
  double t4 = (maximum.y() - org.y())/ray.y();
  double t5 = (minimum.z() - org.z())/ray.z();
  double t6 = (maximum.z() - org.z())/ray.z();

  double tmin = qMax(qMax(qMin(t1, t2), qMin(t3, t4)), qMin(t5, t6));
  double tmax = qMin(qMin(qMax(t1, t2), qMax(t3, t4)), qMax(t5, t6));

  // if tmax < 0, ray is intersecting box, but whole box is behing us
  if (tmax < 0) {
    t = tmax;
    return false;
  }
  // if tmin > tmax, ray doesn't intersect box
  if (tmin > tmax) {
    t = tmax;
    return false;
  }
  t = tmin;
  return true;
}

int Scene3D::pointedMolecule(const QPoint& pos) {
  QVector3D vEnd = unproject(pos.x(), pos.y(), -1.0/*far plane*/);
  QVector3D vOrg = unproject(pos.x(), pos.y(), +1.0/*near plane*/);
  QVector3D vDir = (vEnd - vOrg).normalized();

  int pointed = -1;
  int moleculeIndex = -1;
  double tCurrent = 0.0;
  for (auto& molecule : molecularSystem.vecMolecules) {
    moleculeIndex++;
    double t;
    if (intersectBox(vOrg, vDir, molecule.minimum, molecule.maximum, t)) {
      if (t > tCurrent) {
        tCurrent = t;
        pointed = moleculeIndex;
      }
    }
  }
  return pointed;
}

void Scene3D::setInitialViewPoint() {
  angleOfView = 40.0;
  orthoScale = 1.0;
  viewPoint = molecularSystem.center;

  modelviewMatrix.setToIdentity();
  // if the molecule is empty, we want to look at its center
  // from some distance (here 20.0) -- this gives us some room to work
  if (molecularSystem.numAtoms < 2) {
    modelviewMatrix.translate(-viewPoint - QVector3D(0,0,20.0));
  }
  else {
    // If we're here, the molecule is not empty, i.e. has atoms.
    // Now we want to move backwards, in order to view the molecule from a distance, not from inside it.
    QMatrix4x4 translation;
    translation.setColumn(3, QVector4D(((molecularSystem.diameter + CAMERA_NEAR_DISTANCE) * QVector3D(0,0,-1.0)), 1.0));
    modelviewMatrix = translation * modelviewMatrix;
    modelviewMatrix.translate(-viewPoint);
  }
  applyProjection();
}

void Scene3D::drawScene() {
  if (molecularSystem.vecMolecules.isEmpty()) return;
  if (molecularSystem.representation == molecularSystem.LINE) drawLines();
  if (molecularSystem.representation == molecularSystem.VDW) drawVDW();
}

void Scene3D::drawÑoordinateAxes() {
  // Render x, y, z axes as an overlay on the widget
  QFont labelFont("Courier", 10);
  glDisable(GL_LIGHTING);
  // Save the OpenGL projection matrix and set up an orthogonal projection
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  // Ensure the axes are of the same length
  double aspectRatio = (double)width()/height();
  glOrtho(0, aspectRatio, 0, 1, 0, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  // Set the origin and calculate the positions of the axes
  QVector3D origin(0.07f, 0.07f, -0.07f);
  QVector3D axisX = modelviewMatrix.column(0).toVector3D() * 0.06f + origin;
  QVector3D axisY = modelviewMatrix.column(1).toVector3D() * 0.06f + origin;
  QVector3D axisZ = modelviewMatrix.column(2).toVector3D() * 0.06f + origin;
  glLineWidth(1.5f);
  glColor3f(1.0f, 0.0f, 0.0f); // red
  glBegin(GL_LINES); // OX axis
    glVertex3f(origin.x(), origin.y(), origin.z());
    glVertex3f(axisX.x(), axisX.y(), axisX.z());
  glEnd();
  renderText(axisX.x(), axisX.y(), axisX.z(), "x", labelFont);
  glColor3f(0.0f, 0.5f, 0.0f); // green
  glBegin(GL_LINES); // OY axis
    glVertex3f(origin.x(), origin.y(), origin.z());
    glVertex3f(axisY.x(), axisY.y(), axisY.z());
  glEnd();
  renderText(axisY.x(), axisY.y(), axisY.z(), "y", labelFont);
  glColor3f(0.0f, 0.0f, 1.0f); // blue
  glBegin(GL_LINES); // OZ axis
    glVertex3f(origin.x(), origin.y(), origin.z());
    glVertex3f(axisZ.x(), axisZ.y(), axisZ.z());
  glEnd();
  renderText(axisZ.x(), axisZ.y(), axisZ.z(), "z", labelFont);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_LIGHTING);
}

#include "atomColors.h"

void Scene3D::drawLines() {
  glDisable(GL_LIGHTING);
  for (auto& molecule : molecularSystem.vecMolecules) {
    for (auto& atom : molecule.vecAtoms) {
      int id = atom.GetElementID();
      bool selected = (atom.GetAtomFlags() & Common::eAtomIsSelected);
      glColor3ubv(selected ? selectColor : AtomColors[id]);
      double atomX=atom.GetX(), atomY=atom.GetY(), atomZ=atom.GetZ();
      renderText(atomX, atomY, atomZ, atom.GetLabel(), atomLabelFont);
      int nBonds = atom.GetBondsNumber();
      glLineWidth(1.4f);
      glBegin(GL_LINES);
      for (int i = 0; i < nBonds; ++i) {
        auto& neighbour = molecule.vecAtoms[(atom.GetBond(i).first)];
        double neighbourX=neighbour.GetX(), neighbourY=neighbour.GetY(), neighbourZ=neighbour.GetZ();
        double midpointX, midpointY, midpointZ;
        midpointX = (atomX + neighbourX)/2; midpointY = (atomY + neighbourY)/2; midpointZ = (atomZ + neighbourZ)/2;
        glColor3ubv(AtomColors[id]);
        glVertex3d(atomX, atomY, atomZ); glVertex3d(midpointX, midpointY, midpointZ);
        glColor3ubv(AtomColors[neighbour.GetElementID()]);
        glVertex3d(midpointX, midpointY, midpointZ); glVertex3d(neighbourX, neighbourY, neighbourZ);
      }
      glEnd();
    }
  }
}

inline void normalizeGLfloatv(GLfloat* v) {
  GLfloat d = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
  v[0] /= d; v[1] /= d; v[2] /= d;
}

void drawTriangle(GLfloat* a, GLfloat* b, GLfloat* c, GLfloat r, int depth) {
  if (depth == 0) {
    glNormal3fv(a); glVertex3f(a[0]*r, a[1]*r, a[2]*r);
    glNormal3fv(b); glVertex3f(b[0]*r, b[1]*r, b[2]*r);
    glNormal3fv(c); glVertex3f(c[0]*r, c[1]*r, c[2]*r);
    return;
  }
  GLfloat ab[3], ac[3], bc[3];
  /* calculate midpoints of each side */
  for (int i = 0; i < 3; i++) {
    ab[i] = a[i] + b[i];
    ac[i] = a[i] + c[i];
    bc[i] = b[i] + c[i];
  }
  /* extrude midpoints to lie on unit sphere */
  normalizeGLfloatv(ab); normalizeGLfloatv(ac); normalizeGLfloatv(bc);
  /* recursively subdivide new triangles */
  drawTriangle(a, ab, ac, r, depth-1);
  drawTriangle(b, bc, ab, r, depth-1);
  drawTriangle(c, ac, bc, r, depth-1);
  drawTriangle(ab, bc, ac, r, depth-1);
}

void drawSphere(GLfloat radius, int ndiv=3) {
  #define SPHERE_X 0.525731112119133606f
  #define SPHERE_Z 0.850650808352039932f
  static GLfloat icosaederVertices[12][3] = {
    {-SPHERE_X, 0.0, SPHERE_Z},
    { SPHERE_X, 0.0, SPHERE_Z},
    {-SPHERE_X, 0.0,-SPHERE_Z},
    { SPHERE_X, 0.0,-SPHERE_Z},
    { 0.0, SPHERE_Z, SPHERE_X},
    { 0.0, SPHERE_Z,-SPHERE_X},
    { 0.0,-SPHERE_Z, SPHERE_X},
    { 0.0,-SPHERE_Z,-SPHERE_X},
    { SPHERE_Z, SPHERE_X, 0.0},
    {-SPHERE_Z, SPHERE_X, 0.0},
    { SPHERE_Z,-SPHERE_X, 0.0},
    {-SPHERE_Z,-SPHERE_X, 0.0}
  };
  static GLuint icosaederIndices[20][3] = {
    {0, 4, 1}, {0, 9, 4}, {9, 5, 4}, { 4, 5, 8}, {4, 8 ,1},
    {8,10, 1}, {8, 3,10}, {5, 3, 8}, { 5, 2, 3}, {2, 7, 3},
    {7,10, 3}, {7, 6,10}, {7,11, 6}, {11, 0, 6}, {0, 1, 6},
    {6, 1,10}, {9, 0,11}, {9,11, 2}, { 9, 2, 5}, {7, 2,11}
  };
  glBegin(GL_TRIANGLES);
  for (int i = 0; i < 20; i++) {
    drawTriangle(
      icosaederVertices[icosaederIndices[i][0]],
      icosaederVertices[icosaederIndices[i][1]],
      icosaederVertices[icosaederIndices[i][2]],
      radius, ndiv);
  }
  glEnd();
}

void Scene3D::drawVDW() {
  glEnable(GL_LIGHTING);
  for (auto& molecule : molecularSystem.vecMolecules) {
    for (auto& atom : molecule.vecAtoms) {
      int id = atom.GetElementID();
      bool selected = (atom.GetAtomFlags() & Common::eAtomIsSelected);
      glColor3ubv(selected ? selectColor : AtomColors[id]);
      glPushMatrix();
      glTranslated(atom.GetX(), atom.GetY(), atom.GetZ());
      drawSphere((GLfloat)Common::g_PT[id].m_dVdWRadius/*, 2*/);
      glPopMatrix();
    }
  }
}

void drawMapLinks() {
  glColor3ub(255, 255, 255);
  glLineWidth(1.0f);
  glEnable(GL_LINE_STIPPLE);
  glLineStipple(1, 0x0101); // (3,0xAAAA) (1,0x00FF)
  glBegin(GL_LINES);
  int size = mutationMap.mappingList.size();
  for (int i = 0; i < (size/2)*2; i += 2) {
    AtomIndex& indexA = mutationMap.mappingList[i];
    AtomIndex& indexB = mutationMap.mappingList[i+1];
    const Atom& atomA = molecularSystem.vecMolecules[indexA.first].vecAtoms[indexA.second];
    const Atom& atomB = molecularSystem.vecMolecules[indexB.first].vecAtoms[indexB.second];
    glVertex3d(atomA.GetX(), atomA.GetY(), atomA.GetZ());
    glVertex3d(atomB.GetX(), atomB.GetY(), atomB.GetZ());
  }
  if (size % 2) {
    AtomIndex& index = mutationMap.mappingList.last();
    const Atom& atom = molecularSystem.vecMolecules[index.first].vecAtoms[index.second];
    glVertex3d(atom.GetX(), atom.GetY(), atom.GetZ());
    const Molecule& alterMolecule = molecularSystem.vecMolecules[!index.first];
    QVector3D midPoint = (alterMolecule.minimum + alterMolecule.maximum)/2;
    glVertex3d(midPoint.x(), midPoint.y(), midPoint.z());
  }
  glEnd();
  glDisable(GL_LINE_STIPPLE);
}
