#pragma once
#include <QtWidgets>
#include <math.h>

enum class ObjectType {
	None, Line, Polygon, Circle //Curve
};

class ViewerWidget :public QWidget {
	Q_OBJECT
private:
	QSize areaSize = QSize(0, 0);
	QImage* img = nullptr; //ma strukturu, v ktorom je smernik s baitami, reprezentujucimi pixeli; ulozene v 1D pole 
	uchar* data = nullptr; //pre pristup

	bool drawLineActivated = false;
	bool drawPolygonActivated = false;
	QPoint drawLineBegin = QPoint(0, 0);

	QVector<QPoint> polygonPoints;
	QVector<QPoint> transformedPoints;

	ObjectType currentObjectType = ObjectType::None; 

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
	void setDrawLineActivated(bool state) { drawLineActivated = state; }
	bool getDrawLineActivated() { return drawLineActivated; }

	void addPolygonPoint(QPoint point);
	void setPolygonPoints(QVector<QPoint> points) { polygonPoints = points; };
	void setTransformedPoints(const QVector<QPoint>& tpoints) { transformedPoints = tpoints; }
	QVector<QPoint> getPolygonPoints() { return polygonPoints; }
	QVector<QPoint> getTransformedPoints() { return transformedPoints; }
		
	void setDrawPolygonActivated(bool state) { drawPolygonActivated = state; }
	bool getDrawPolygonActivated() { return drawPolygonActivated; }
	void closePolygon(QColor color, int algType = 0);
	void clearObject();

	void setObjectType(ObjectType type) { currentObjectType = type; }
	void drawObject(QColor color, int algType);

	//Transformation functions
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
	void setDataPtr() { data = img ? img->bits() : nullptr; } //zoberie data a ulozi do smernika
	int getImgWidth() { return img ? img->width() : 0; };
	int getImgHeight() { return img ? img->height() : 0; };

	void clear();

	//Algorithms
	void drawLineDDA(QPoint start, QPoint end, QColor color);
	void drawLineBresenham(QPoint start, QPoint end, QColor color);

public slots:
	void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
};