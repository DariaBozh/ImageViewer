#pragma once
#include <QtWidgets>
#include <math.h>
#include <algorithm>
#include <limits>
#include "Object3D.h"

enum class ObjectType {
	None, Line, Polygon, Triangle, Circle, HermiteCubic, BezierCurve, CoonsBSpline
};

enum class DrawState {
	Ready, InProgress, Finished
};

struct TVertex { //for triangle
	QPointF point;
	QColor color;
};

struct Edge { //for filling
	int yMin;
	int yMax;
	double x; //intersection with the current line
	double w; // w = 1/m
};

struct HermitePoint {
	QPointF pos;     
	double angle;  
	double length;  
};

class ViewerWidget :public QWidget {
	Q_OBJECT
private:
	QSize areaSize = QSize(0, 0);
	QImage* img = nullptr; //has a structure containing an index with bytes representing pixels; stored in a 1D array 
	uchar* data = nullptr; //for access

	QPoint drawLineBegin = QPoint(0, 0);

	QVector<QPoint> polygonPoints;
	QVector<QPoint> transformedPoints;
	QVector<QPointF> curvePoints;
	QVector<HermitePoint> hermitePoints;

	ObjectType currentObjectType = ObjectType::None; 
	DrawState currentDrawState = DrawState::Ready;

	bool isFilled = false;
	QColor fillColor;

	TVertex v1, v2, v3; 
	bool isTriangleFilled = false;
	int currentInterType = 0;

	//3D object
	//std::vector<std::vector<double>> zBuffer;

public:

	ViewerWidget(QSize imgSize, QWidget* parent = Q_NULLPTR);
	~ViewerWidget();
	void resizeWidget(QSize size);

	//Image functions
	bool setImage(const QImage& inputImg);
	QImage* getImage() { return img; };
	bool isEmpty();
	bool changeSize(int width, int height);

	void setPixel(int x, int y, int r, int g, int b, int a = 255);
	void setPixel(int x, int y, double valR, double valG, double valB, double valA = 1.);
	void setPixel(int x, int y, const QColor& color);
	bool isInside(int x, int y);

	//Draw functions
	void drawLine(QPoint start, QPoint end, QColor color, int algType = 0);
	void drawCircle(QPoint center, QPoint radiusLen, QColor color);
	void drawPolygon(QVector<QPoint> points, QColor color, int algType = 0);

	void setDrawLineBegin(QPoint begin) { drawLineBegin = begin; }
	QPoint getDrawLineBegin() { return drawLineBegin; }

	void addPolygonPoint(QPoint point);
	void setPolygonPoints(QVector<QPoint> points) { polygonPoints = points; };
	void setTransformedPoints(const QVector<QPoint>& tpoints) { transformedPoints = tpoints; }
	QVector<QPoint> getPolygonPoints() { return polygonPoints; }
	QVector<QPoint> getTransformedPoints() { return transformedPoints; }
		
	void closePolygon(QColor color, int algType = 0);

	//Curves
	void drawHermiteCubic(QVector<HermitePoint> controlPoints, QColor color);
	void drawTangeantVectors(QColor color);
	void drawBezierCurve(QVector<QPointF> controlPoints, QColor color);
	void drawCoonsBSpline(QVector<QPointF> controlPoints, QColor color);

	void addHermitePoint(HermitePoint point) { hermitePoints.append(point); };
	void setHermiteAngle(int index, double angle);
	void setHermiteLength(int index, double length);
	QVector<HermitePoint>& getHermitePoints() { return hermitePoints; }
	
	void addCurvePoint(QPointF point) { curvePoints.append(point); };
	QVector<QPointF> getCurvePoints() { return curvePoints; }
	
	//Filling functions
	void scanLine(const QVector<QPoint>& points, QColor color);
	QVector<Edge> createEdgeTable(const QVector<QPoint>& points, int& yMin, int& yMax);
	void fillTriangle(TVertex T0, TVertex T1, TVertex T2, int interType = 0);
	void fillBaseTriangle(TVertex T0, TVertex T1, TVertex T2, TVertex orig0, TVertex orig1, TVertex orig2, int interType, bool excludeEnd = false);

	//For object type
	void setObjectType(ObjectType type) { currentObjectType = type; }
	ObjectType getObjectType() { return currentObjectType; }
	void setDrawState(DrawState state) { currentDrawState = state; }
	DrawState getDrawState() { return currentDrawState; }

	void drawObject(QColor color, int algType);
	void clearObject();

	//Transformations
	QVector<QPoint> rotation(const QVector<QPoint>& points, double a, QPoint origin = QPoint(0,0));
	QVector<QPoint> scale(const QVector<QPoint>& points, double dx, double dy);
	QVector<QPoint> share(const QVector<QPoint>& points, double d);
	QVector<QPoint> symmetry(QPoint A, QPoint B, const QVector<QPoint>& points);
	QVector<QPoint> displacement(QPoint origin, QPoint newP, const QVector<QPoint>& points);
	
	QVector<QPoint> clipLine(QPoint P1, QPoint P2);
	QVector<QPoint> clipPolygon(const QVector<QPoint>& points);
	QVector<QPoint> clipWithEdge(const QVector<QPoint>& points, double xmin);

	//Get/Set functions
	uchar* getData() { return data; }
	void setDataPtr() { data = img ? img->bits() : nullptr; } //takes the data and stores it in the pointer
	int getImgWidth() { return img ? img->width() : 0; };
	int getImgHeight() { return img ? img->height() : 0; };

	void clear();

	//Algorithms
	void drawLineDDA(QPoint start, QPoint end, QColor color);
	void drawLineBresenham(QPoint start, QPoint end, QColor color);
	QColor getNearestNeighborColor(int x, int y, TVertex T0, TVertex T1, TVertex T2);
	QColor getBarycentricColor(int x, int y, TVertex T0, TVertex T1, TVertex T2);

	//Camera and 3D
	void draw3DObject(const Object3D& object, double theta, double phi, double rho, int projection_type, int representation);
	QPoint projectPoint(const QVector3D& V, int projection_type);
	void renderEdge(QVector3D P1, QVector3D P2, int projection_type, double near);
	
	void zBufferAlg(QPoint p0, double d0, QPoint p1, double d1, QPoint p2, double d2,
		QColor color, QVector<QVector<double>>& zbuf);

public slots:
	void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
};