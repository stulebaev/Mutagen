// scene3D.h
#ifndef SCENE3D_H
#define SCENE3D_H

#include "mapping.h"
#include <QtGui/QMatrix4x4>
#include <QtOpenGL/QGLWidget>

#define ROTATION_SPEED     0.3
#define MOUSE_WHEEL_SPEED  0.002
#define ZOOM_STEP          0.2
#define CAMERA_NEAR_DISTANCE 2.0

#define ATOM_CHECK_INTERVAL 0.2

enum ProjectionType {
  Perspective  = 1,
  Orthographic = 2
};

class Scene3D : public QGLWidget {
  Q_OBJECT
public:
  Scene3D(QWidget* parent = 0);
  void setInitialViewPoint();
  ProjectionType projectionType;
  bool showAxes;
private:
  double orthoScale;
  double angleOfView;
  QVector3D viewPoint; /* position of the reference point */
  QMatrix4x4 modelviewMatrix; /* modelview matrix for OpenGL */
  QMatrix4x4 projectionMatrix; /* projection matrix for OpenGL */
  QFont atomLabelFont;
  QPoint lastMousePosition;
protected:
  void initializeGL();
  void resizeGL(int width, int height);
  void paintGL();
  void wheelEvent(QWheelEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
  void mousePressEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void mouseDoubleClickEvent(QMouseEvent* event);
  void keyPressEvent(QKeyEvent* event);
private:
  void drawScene();
  void drawCoordinateAxes();
  void drawVDW();
  void drawLines();
  AtomIndex pickAtom(const QPoint& pos);
  int pointedMolecule(const QPoint& pos);
  void applyModelview();
  void applyProjection();
  void zoom(double delta);
  void translate(double deltaX, double deltaY);
  void rotate(double angleX, double angleY, double angleZ);
  QVector3D project(const QVector3D& v);
  QVector3D unproject(double x, double y, double z);
};

#endif // SCENE3D_H
