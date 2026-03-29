#include   "ViewerWidget.h"

ViewerWidget::ViewerWidget(QSize imgSize, QWidget* parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_StaticContents);
	setMouseTracking(true);
	if (imgSize != QSize(0, 0)) { //ci ma velkost/existuje
		img = new QImage(imgSize, QImage::Format_ARGB32);
		img->fill(Qt::white);
		resizeWidget(img->size()); 
		setDataPtr();
	}
}
ViewerWidget::~ViewerWidget()
{
	delete img;
	img = nullptr;
	data = nullptr;
}
void ViewerWidget::resizeWidget(QSize size)
{
	this->resize(size);
	this->setMinimumSize(size);
	this->setMaximumSize(size);
}

//Image functions
bool ViewerWidget::setImage(const QImage& inputImg)
{
	if (img) {
		delete img;
		img = nullptr;
		data = nullptr;
	}
	img = new QImage(inputImg.convertToFormat(QImage::Format_ARGB32));
	if (!img || img->isNull()) {
		return false;
	}
	resizeWidget(img->size());
	setDataPtr();
	update();

	return true;
}
bool ViewerWidget::isEmpty()
{
	if (img == nullptr) {
		return true;
	}

	if (img->size() == QSize(0, 0)) {
		return true;
	}
	return false;
}

bool ViewerWidget::changeSize(int width, int height)
{
	QSize newSize(width, height);

	if (newSize != QSize(0, 0)) {
		if (img != nullptr) {
			delete img;
		}

		img = new QImage(newSize, QImage::Format_ARGB32);
		if (!img || img->isNull()) {
			return false;
		}
		img->fill(Qt::white);
		resizeWidget(img->size());
		setDataPtr();
		update();
	}

	return true;
}

void ViewerWidget::setPixel(int x, int y, int r, int g, int b, int a)
{
	if (!img || !data) return;
	if (!isInside(x, y)) return;

	r = r > 255 ? 255 : (r < 0 ? 0 : r);
	g = g > 255 ? 255 : (g < 0 ? 0 : g);
	b = b > 255 ? 255 : (b < 0 ? 0 : b);
	a = a > 255 ? 255 : (a < 0 ? 0 : a);

	size_t startbyte = y * img->bytesPerLine() + x * 4; //kazdy pixel zakodovany 4 baitami!
	data[startbyte] = static_cast<uchar>(b);
	data[startbyte + 1] = static_cast<uchar>(g);
	data[startbyte + 2] = static_cast<uchar>(r);
	data[startbyte + 3] = static_cast<uchar>(a);
}
void ViewerWidget::setPixel(int x, int y, double valR, double valG, double valB, double valA) //dostane cislo od 0 do 1
{
	valR = valR > 1 ? 1 : (valR < 0 ? 0 : valR);
	valG = valG > 1 ? 1 : (valG < 0 ? 0 : valG);
	valB = valB > 1 ? 1 : (valB < 0 ? 0 : valB);
	valA = valA > 1 ? 1 : (valA < 0 ? 0 : valA);

	setPixel(x, y, static_cast<int>(255 * valR + 0.5), static_cast<int>(255 * valG + 0.5), static_cast<int>(255 * valB + 0.5), static_cast<int>(255 * valA + 0.5));
}
void ViewerWidget::setPixel(int x, int y, const QColor& color)
{
	if (color.isValid()) {
		setPixel(x, y, color.red(), color.green(), color.blue(), color.alpha()); //alfa je presvetnost
	}
}

bool ViewerWidget::isInside(int x, int y)
{
	return img && x >= 0 && y >= 0 && x < img->width() && y < img->height();
}

//Draw functions
void ViewerWidget::drawLine(QPoint start, QPoint end, QColor color, int algType)
{
	if (!img || !data) return;

	if (algType == 0) {
		drawLineDDA(start, end, color);
	}
	else {
		drawLineBresenham(start, end, color);
	}
	update();

	//Po implementovani drawLineDDA a drawLineBresenham treba vymazat
	/*QPainter painter(img);
	painter.setPen(QPen(color));
	painter.drawLine(start, end);
	update();*/
}
void ViewerWidget::drawCircle(QPoint center, QPoint radiusLen, QColor color) //Сelociselny
{ 
	if (!img || !data) return;

	int radius = static_cast<int>(sqrt(pow(radiusLen.x() - center.x(), 2) + pow(radiusLen.y() - center.y(), 2)));

	int p = 1 - radius;
	int x = 0;
	int y = radius;
	int twoX = 3;
	int twoY = 2 * radius - 2;

	int x0 = center.x();
	int y0 = center.y();

	while (x <= y) { // 45 stupnov
		setPixel(x0 + x,y0 + y, color);
		setPixel(x0 + x, y0 - y, color);
		setPixel(x0 - x, y0 + y, color);
		setPixel(x0 - x, y0 - y, color);
		setPixel(x0 + y, y0 + x, color);
		setPixel(x0 + y, y0 - x, color);
		setPixel(x0 - y, y0 + x, color);
		setPixel(x0 - y, y0 - x, color);

		if (p > 0) {
			p -= twoY;
			y--;
			twoY -= 2;
		}
		p += twoX;
		twoX += 2;
		x++;
	}
	update();
}
void ViewerWidget::drawPolygon(QVector<QPoint> points, QColor color, int algType)
{
	//qDebug() << "Drawing points count:" << points.size();
	if (points.size() < 2) return;

	for (int i = 0; i < points.size() - 1; i++) { // до передостанньої точки 
		drawLine(points[i], points[i + 1], color, algType);
	}

	if (points.size() > 2) {
		drawLine(points[points.size() - 1], points[0], color, algType); //abo last-first
	}
}

void ViewerWidget::addPolygonPoint(QPoint point)
{
	polygonPoints.append(point);
	transformedPoints = polygonPoints; //synchronization
}
void ViewerWidget::closePolygon(QColor color, int algType)
{
	if (polygonPoints.size() < 3) return;

	QPoint endpoint = polygonPoints.last();
	QPoint startpoint = polygonPoints.first();
	drawLine(endpoint, startpoint, color, algType);
}

void ViewerWidget::drawHermiteCubic(QVector<HermitePoint> controlPoints, QColor color)
{
	int n = controlPoints.size();
	if (n < 2) return;

	int N = 50; // Because I want so
	double dt = 1.0 / N;

	for (int i = 1; i < n; i++) {
		HermitePoint P0 = controlPoints[i-1]; // P{i-1}
		HermitePoint P1 = controlPoints[i]; // Pi

		QPointF dP0 (P0.length * cos(P0.angle * M_PI / 180.0), P0.length * sin(P0.angle * M_PI / 180.0));
		QPointF dP1 (P1.length * cos(P1.angle * M_PI / 180.0), P1.length * sin(P1.angle * M_PI / 180.0));

		QPointF Q0 = P0.pos;
		double Ft0, Ft1, Ft2, Ft3; //Hermitian cubic polynomials
		double t = dt;

		while (t <= 1) {
			double t2 = t * t;
			double t3 = t2 * t;
			Ft0 = 2 * t3 - 3 * t2 + 1;
			Ft1 = -2 * t3 + 3 * t2;
			Ft2 = t3 - 2 * t2 + t;
			Ft3 = t3 - t2;
			QPointF Q1 = P0.pos * Ft0 + P1.pos * Ft1 + dP0 * Ft2 + dP1 * Ft3;

			drawLine(Q0.toPoint(), Q1.toPoint(), color);
			Q0 = Q1;
			t += dt;
		}
		drawLine(Q0.toPoint(), P1.pos.toPoint(), color);
	}
}
void ViewerWidget::drawTangeantVectors(QColor color) {
	for (const auto& p : hermitePoints) {
		// Finding the end of the vector
		double rad = p.angle * M_PI / 180.0;
		QPoint target(
			p.pos.x() + p.length * cos(rad),
			p.pos.y() + p.length * sin(rad)
		);
		
		drawLine(p.pos.toPoint(), target, color);
	}
}
void ViewerWidget::drawBezierCurve(QVector<QPointF> controlPoints, QColor color)
{// De Casteljau's algorithm

	int n = controlPoints.size();
	if (n < 2) return;

	QPointF** P = new QPointF*[n];
	for (int i = 0; i < n; i++) {
		P[i] = new QPointF[n-i];
	}

	for (int j = 0; j < n; j++) {
		P[0][j] = controlPoints[j];
	}
	
	int N = 100;
	double dt = 1.0 / N;
	double t = dt;

	QPointF Q0 = controlPoints.first();

	while (t <= 1.001) { // to account for the double precision error
		for (int i = 1; i < n; i++) {
			for (int j = 0; j < n - i; j++) {
				P[i][j] = (1 - t) * P[i-1][j] + t * P[i-1][j+1];
			}
		}

		QPointF Q1 = P[n - 1][0];
		drawLine(Q0.toPoint(), Q1.toPoint(), color);
		Q0 = Q1;
		t += dt;
	}
	drawLine(Q0.toPoint(), controlPoints.last().toPoint(), color);

	for (int i = 0; i < n; i++) {
		delete[] P[i];
	}
	delete[] P;

	//// Or make it easier:
	//while (t <= 1.001) {
	//	QVector<QPointF> temp = controlPoints; 
	//	while (temp.size() > 1) {
	//		for (int i = 0; i < temp.size() - 1; i++) {
	//			temp[i] = (1 - t) * temp[i] + t * temp[i + 1];
	//		}
	//		temp.pop_back(); // "Сплющуємо" піраміду
	//	}
	//	QPointF Q1 = temp[0];
	//	drawLine(Q0.toPoint(), Q1.toPoint(), color);
	//	Q0 = Q1;
	//	t += dt;
	//}
	////...
}
void ViewerWidget::drawCoonsBSpline(QVector<QPointF> controlPoints, QColor color)
{
	int n = controlPoints.size();
	if (n < 4) return;

	int N = 100;
	double dt = 1.0 / N;

	for (int i = 3; i < n; i++) {
		double t = 0;
		// for t=0: Bt0 = 1/6, Bt1 = 4/6, Bt2 = 1/6, Bt3 = 0
		QPointF Q0 = controlPoints[i - 3] * (1.0/6) + controlPoints[i - 2] * (4.0 / 6) + controlPoints[i - 1] * (1.0 / 6);
	
		while (t < 1) {
			t += dt;
			double t2 = t * t;
			double t3 = t * t2;

			double Bt0 = -(1.0 / 6) * t3 + (1.0 / 2) * t2 - (1.0 / 2) * t + (1.0 / 6);
			double Bt1 = (1.0 / 2) * t3 - t2 +(2.0 / 3);
			double Bt2 = -(1.0 / 2) * t3 + (1.0 / 2) * t2 + (1.0 / 2) * t + (1.0 / 6);
			double Bt3 = (1.0 / 6) * t3;

			QPointF Q1 = controlPoints[i - 3] * Bt0 + controlPoints[i - 2] * Bt1 + controlPoints[i - 1] * Bt2 + controlPoints[i] * Bt3;
			drawLine(Q0.toPoint(), Q1.toPoint(), color);
			Q0 = Q1;
		}
	}
}

void ViewerWidget::setHermiteAngle(int index, double angle) {
	if (index >= 0 && index < hermitePoints.size()) {
		hermitePoints[index].angle = angle;
	}
}
void ViewerWidget::setHermiteLength(int index, double length) {
	if (index >= 0 && index < hermitePoints.size()) {
		hermitePoints[index].length = length;
	}
}

//Filling functions
void ViewerWidget::scanLine(const QVector<QPoint>& points, QColor color)
{
	int yMin, yMax;
	QVector<Edge> ET = createEdgeTable(points, yMin, yMax);
	if (ET.isEmpty()) return;

	QList<Edge> AET; 

	for (int y = yMin; y <= yMax; ++y) {
		// Move edges that START at this scan line into AET
		for (int i = 0; i < ET.size(); ) {
			if (ET[i].yMin == y) {
				AET.append(ET[i]);
				ET.removeAt(i); // We don't increment i because the list has shifted
			}
			else {
				i++;
			}
		}

		// Sorting by x
		std::sort(AET.begin(), AET.end(), [](const Edge& a, const Edge& b) {
			return a.x < b.x;
		});

		// Fill between pairs
		for (int j = 0; j + 1 < AET.size(); j += 2) {
			int xStart = static_cast<int>(std::ceil(AET[j].x));
			int xEnd = static_cast<int>(std::floor(AET[j + 1].x));

			if (xStart <= xEnd) {
				for (int x = xStart; x <= xEnd; x++) {
					setPixel(x, y, color);
				}
			}
		}

		// Remove finished edges (Δy == 0 after decrement), update x
		for (int k = 0; k < AET.size(); ) {
			Edge& edge = AET[k];
			edge.x += edge.w;
			// yMax was already shortened by 1, so remove when y reaches yMax
			if (y >= edge.yMax) {
				AET.removeAt(k);
			}
			else {
				k++;
			}
		}
	}
	update();

	// Save fill state so drawObject can re-apply it after transformations
	isFilled = true;
	fillColor = color;
}
QVector<Edge> ViewerWidget::createEdgeTable(const QVector<QPoint>& points, int& yMin, int& yMax)
{
	QVector<Edge> edgeTable;
	int n = points.size();

	yMin = points[0].y();
    yMax = points[0].y();

	for (int i = 0; i < n; i++) {
		QPoint p1 = points[i];
		if (p1.y() < yMin) yMin = p1.y();
		if (p1.y() > yMax) yMax = p1.y();
		QPoint p2 = points[(i + 1) % n];

		if (p1.y() == p2.y()) continue;

		Edge e;
		if (p1.y() < p2.y()) {
			e.yMin = p1.y();
			e.yMax = p2.y();
			e.yMax -= 1;
			e.x = p1.x(); 
		}
		else {
			e.yMin = p2.y();
			e.yMax = p1.y();
			e.yMax -= 1;
			e.x = p2.x();
		}

		e.w = static_cast<double>(p2.x() - p1.x()) / (p2.y() - p1.y());

		edgeTable.append(e);
	}

	std::sort(edgeTable.begin(), edgeTable.end(), [](const Edge& a, const Edge& b) {
		if (a.yMin == b.yMin) return a.x < b.x; 
		return a.yMin < b.yMin; // sorting by yMin
	});
	
	return edgeTable;
}

void ViewerWidget::fillTriangle(TVertex T0, TVertex T1, TVertex T2, int interType)
{
	v1 = T0;
	v2 = T1;
	v3 = T2;
	currentInterType = interType;
	isTriangleFilled = true;

	//Sorting
	auto compare = [](const TVertex& a, const TVertex& b) {
		if (a.point.y() != b.point.y()) return a.point.y() > b.point.y();
		return a.point.x() > b.point.x();
	};

	if (compare(T0, T1)) std::swap(T0, T1);
	if (compare(T1, T2)) std::swap(T1, T2);
	if (compare(T0, T1)) std::swap(T0, T1);

	TVertex t0 = T0, t1 = T1, t2 = T2;

	if (T1.point.y() == T2.point.y()) { // Flat-bottom 
		fillBaseTriangle(T0, T1, T2, t0, t1, t2, interType);
	}
	else if (T0.point.y() == T1.point.y()) { // Flat-top
		fillBaseTriangle(T2, T0, T1, t0, t1, t2, interType);
	}
	else {// Split
		TVertex P;
		double t = static_cast<double>(T1.point.y() - T0.point.y()) / (T2.point.y() - T0.point.y());
		P.point = QPoint(qRound(T0.point.x() + t * (T2.point.x() - T0.point.x())), T1.point.y());

		fillBaseTriangle(T0, T1, P, t0, t1, t2, interType, true);  // exclude split scanline
		fillBaseTriangle(T2, T1, P, t0, t1, t2, interType, false); // owns the split scanline 
	}
	update();
}
void ViewerWidget::fillBaseTriangle(TVertex t0, TVertex t1, TVertex t2, TVertex orig0, TVertex orig1, TVertex orig2, int interType, bool excludeEnd)
{
	//t0, t1, t2 - vertices of the current subtriangle
	//orig0, orig1, orig2 - used for calculating pixel color
	if (t0.point.y() == t1.point.y()) return;

	double w1 = static_cast<double>(t1.point.x() - t0.point.x()) / (t1.point.y() - t0.point.y());
	double w2 = static_cast<double>(t2.point.x() - t0.point.x()) / (t2.point.y() - t0.point.y());

	double x1 = t0.point.x();
	double x2 = t0.point.x();

	//Direction
	int stepY = (t0.point.y() < t1.point.y()) ? 1 : -1;
	int yStart = t0.point.y();
	int yEnd = t1.point.y();

	//for (int y = yStart; (stepY > 0 ? y <= yEnd : y >= yEnd); y += stepY) {
	for (int y = yStart; (stepY > 0 ? (excludeEnd ? y < yEnd : y <= yEnd) : y >= yEnd); y += stepY) {
		//drawing line between x1 and x2
		int startX = std::ceil(std::min(x1, x2));
		int endX = std::floor(std::max(x1, x2));
		
		for (int x = startX; x <= endX; x++) {
			QColor pixelColor;
			if (interType == 0) {
				pixelColor = getNearestNeighborColor(x, y, orig0, orig1, orig2);
			}
			else {
				pixelColor = getBarycentricColor(x, y, orig0, orig1, orig2);
			}
			setPixel(x, y, pixelColor);
		}

		x1 += w1 * stepY;
		x2 += w2 * stepY;
	}
}

void ViewerWidget::drawObject(QColor color, int algType)
{
	if (transformedPoints.isEmpty() && hermitePoints.isEmpty() && curvePoints.isEmpty()) return;

	switch (currentObjectType) {
	case ObjectType::Line: {
		if (transformedPoints.size() >= 2) {
			QVector<QPoint> clipped = clipLine(transformedPoints[0], transformedPoints[1]);
			if (!clipped.isEmpty()) {
				drawLine(clipped[0], clipped[1], color, algType);
			}
		}
		break;
	}
	case ObjectType::Polygon: {
		QVector<QPoint> clipped = clipPolygon(transformedPoints);
		if (!clipped.isEmpty()) {
			if (isFilled) {
				scanLine(clipped, fillColor);
			}
			drawPolygon(clipped, color, algType);
		}
		break;
	}
	case ObjectType::Triangle: {
		QVector<QPoint> clipped = clipPolygon(transformedPoints);
		if (!clipped.isEmpty()) {
			drawPolygon(clipped, color, algType);
		}

		if (isTriangleFilled && transformedPoints.size() >= 3) {
			TVertex currentV1 = { transformedPoints[0], v1.color };
			TVertex currentV2 = { transformedPoints[1], v2.color };
			TVertex currentV3 = { transformedPoints[2], v3.color };
			fillTriangle(currentV1, currentV2, currentV3, currentInterType);
		}
		break;
	}
	case ObjectType::HermiteCubic: {
		if (hermitePoints.size() > 1) {
			drawHermiteCubic(hermitePoints, color);
			drawTangeantVectors(Qt::red);

			// Створюємо маляра, який малюватиме по нашому QImage
			QPainter painter(img);
			painter.setPen(Qt::black);
			painter.setFont(QFont("Arial", 10, QFont::Medium));

			int i = 0;
			for (const auto& p : hermitePoints) {
				setPixel(p.pos.x(), p.pos.y(), color);

				QString label = QString::number(i + 1);
				painter.drawText(p.pos.x() + 10, p.pos.y() - 10, label);
				i++;
			}
		}
		break;
	}
	case ObjectType::BezierCurve: {
		if (curvePoints.size() > 1) {
			drawBezierCurve(curvePoints, color);

			int i = 0;
			for (const auto& p : curvePoints) {
				setPixel(p.x(), p.y(), color);
			}
		}
		break;
	}
	case ObjectType::CoonsBSpline: {
		if (curvePoints.size() > 3) {
			drawCoonsBSpline(curvePoints, color);

			int i = 0;
			for (const auto& p : curvePoints) {
				setPixel(p.x(), p.y(), color);
			}
		}
		break;
	}
	default: break;
	}

}
void ViewerWidget::clearObject()
{
	clear();
	polygonPoints.clear();
	transformedPoints.clear();
	hermitePoints.clear();
	curvePoints.clear();

	isFilled = false;
	isTriangleFilled = false;

	currentDrawState = DrawState::Ready;
} 

QVector<QPoint> ViewerWidget::rotation(const QVector<QPoint>& points, double a, QPoint origin)
{
	if (points.isEmpty()) return points;

	QVector<QPoint> newPoints;
	//QPoint origin = points[0];

	double rad = a * M_PI / 180.0;
	double cosA = cos(rad);
	double sinA = sin(rad);

	double x0 = origin.x();
	double y0 = origin.y();

	for (int i = 0; i < points.size(); i++) {
		double x = points[i].x();
		double y = points[i].y();

		int xNew = qRound((x - x0) * cosA - (y - y0) * sinA + x0);
		int yNew = qRound((x - x0) * sinA + (y - y0) * cosA + y0);
		
		newPoints.append(QPoint(xNew, yNew));
	}
	return newPoints;
}
QVector<QPoint> ViewerWidget::scale(const QVector<QPoint>& points, double dx, double dy)
{
	if (points.isEmpty()) return points;

	QVector<QPoint> newPoints;
	QPoint center = points[0];
	double x0 = center.x();
	double y0 = center.y();

	for (int i = 0; i < points.size(); i++) {
		double x = points[i].x() - x0;
		double y = points[i].y() - y0;

		int xNew = qRound(x0 + dx * x);
		int yNew = qRound(y0 + dy * y);
		newPoints.append(QPoint(xNew, yNew));
	}

	return newPoints;
}
QVector<QPoint> ViewerWidget::share(const QVector<QPoint>& points, double d)
{
	if (points.isEmpty()) return points;
	QVector<QPoint> newPoints;

	QPoint center = points[0];
	double y0 = center.y();

	for (int i = 0; i < points.size(); i++) {
		double x = points[i].x();
		double y = points[i].y();

		int xNew = qRound(x + d * (y - y0));
		int yNew = y;
		newPoints.append(QPoint(xNew, yNew));
	}
	return newPoints;
}
QVector<QPoint> ViewerWidget::symmetry(QPoint A, QPoint B, const QVector<QPoint>& points)
{
	if (points.isEmpty()) return points;
	QVector<QPoint> newPoints;

	double u = B.x() - A.x();
	double v = B.y() - A.y();
	double a = v;
	double b = -u;
	double c = (-a * A.x()) - (b * A.y());

	double denominator = (a * a) + (b * b);
	if (denominator == 0) return points; //dividing by 0 

	for (int i = 0; i < points.size(); i++) {
		double px = points[i].x();
		double py = points[i].y();

		double factor = (a * px + b * py + c) / denominator;

		double newX = px - 2 * a * factor;
		double newY = py - 2 * b * factor;

		newPoints.append(QPoint(qRound(newX), qRound(newY)));
	}

	return newPoints;
}
QVector<QPoint> ViewerWidget::displacement(QPoint origin, QPoint newP, const QVector<QPoint>& points)
{
	if (points.isEmpty()) return points;
	QVector<QPoint> newPoints;

	double dx = newP.x() - origin.x();
	double dy = newP.y() - origin.y();

	for (int i = 0; i < points.size(); i++) {
		double newX = points[i].x() + dx;
		double newY = points[i].y() + dy;
		newPoints.append(QPoint(newX, newY));
	}

	setTransformedPoints(newPoints);
	setPolygonPoints(newPoints);

	return newPoints;
}

QVector<QPoint> ViewerWidget::clipLine(QPoint P1, QPoint P2)
{ //Cyrus-Beck algorithm
	QVector<QPoint> newPoints;

	double tLow = 0, tUpp = 1;
	QPoint d = P2 - P1; //Direction vector for P1P2 
	QPoint borderPoints[4] = {QPoint(0,0), QPoint(0, getImgHeight()-1), QPoint(getImgWidth()-1, getImgHeight()-1), QPoint(getImgWidth()-1,0)};

	for (int i = 0; i < 4; i++) {
		double uE, vE;
		if (i == 3) {
			uE = borderPoints[0].x() - borderPoints[i].x();
			vE = borderPoints[0].y() - borderPoints[i].y();
		}
		else {
			uE = borderPoints[i + 1].x() - borderPoints[i].x();
			vE = borderPoints[i + 1].y() - borderPoints[i].y();
		}

		QPoint w = P1 - borderPoints[i]; //Direction vector from edge to begining of P1P2
		QPoint n(vE, -uE);
		double dn = d.x() * n.x() + d.y() * n.y();
		double wn = w.x() * n.x() + w.y() * n.y();

		if (dn != 0) {
			double t = -wn / dn;
			if (dn > 0) { //The line is directed inward (всередину)
				tLow = std::max(t, tLow);
			}
			else {
				tUpp = std::min(t, tUpp);
			}
		}
		else {//If line is paralel and outside
			if (wn < 0) return QVector<QPoint>();
		}
		
	}

	if (tLow <= tUpp) {
		QPoint P1new(qRound(P1.x() + tLow * d.x()), qRound(P1.y() + tLow * d.y()));
		QPoint P2new(qRound(P1.x() + tUpp * d.x()), qRound(P1.y() + tUpp * d.y()));

		newPoints.append(P1new);
		newPoints.append(P2new);
	}

	return newPoints;
}
QVector<QPoint> ViewerWidget::clipPolygon(const QVector<QPoint>& points)
{ // Sutherland-Hodgman algorithm

	if (points.isEmpty()) return points;
	QVector<QPoint> results = points;

	double xmins[4] = { 0, 0, -(double)(getImgWidth()-1), -(double)(getImgHeight() - 1) };

	for (int i = 0; i < 4; i++) {
		results = clipWithEdge(results, xmins[i]);
		results = rotation(results, -90);
	}

	return results;
}
QVector<QPoint> ViewerWidget::clipWithEdge(const QVector<QPoint>& points, double xmin)
{
	QVector<QPoint> W;
	if (points.isEmpty()) return W; // Захист від порожнього масиву

	QPoint S = points.last();

	for (int i = 0; i < points.size(); i++) {
		QPoint V = points[i];
		QPoint P;
		double Py;

		if (V.x() >= xmin) {
			if (S.x() >= xmin) {
				W.append(V);
			}
			else {
				Py = S.y() + (xmin - S.x()) / (double)(V.x() - S.x()) * (V.y() - S.y());
				P.setX(qRound(xmin));
				P.setY(qRound(Py));
				W.append(P);
				W.append(V);
			}
		}
		else if (S.x() >= xmin) {
			Py = S.y() + (xmin - S.x()) / (V.x() - S.x()) * (V.y() - S.y());
			P.setX(qRound(xmin));
			P.setY(qRound(Py));
			W.append(P);
		}
		S = V;
	}

	return W;
}

void ViewerWidget::clear()
{
	if (!img) return;
	img->fill(Qt::white);
	update();
}

void ViewerWidget::drawLineDDA(QPoint start, QPoint end, QColor color)
{
	double x0 = start.x();
	double y0 = start.y();
	double x1 = end.x();
	double y1 = end.y();

	double dx = x1 - x0;
	double dy = y1 - y0;
	double steps = std::max(abs(dx), abs(dy));
	double xInk, yInk;

	if (steps != 0) { //double m = dy / dx;
		xInk = dx / steps;
		yInk = dy / steps;
	}
	else {
		setPixel(x0, y0, color);
		return;
	}

	for (int i = 0; i < steps; i++) {
		setPixel((int)(x0 + 0.5), (int)(y0 + 0.5), color);
		//pripadne nieco z toho urcite je 1
		x0 += xInk;
		y0 += yInk;
	}

	setPixel((int)(x1 + 0.5), (int)(y1 + 0.5), color); //posledny
	
}
void ViewerWidget::drawLineBresenham(QPoint start, QPoint end, QColor color)
{
	int x0 = start.x();
	int y0 = start.y();
	int x1 = end.x();
	int y1 = end.y();

	int adx = abs(x1 - x0);
	int ady = abs(y1 - y0);

	int k1, k2, p; // p je chyba

	int stepX = (x1 > x0) ? 1 : -1;
	int stepY = (y1 > y0) ? 1 : -1;

	if (adx >= ady) { //hlavna je os x
		k1 = 2 * ady;
		k2 = k1 - 2 * adx;
		p = k1 - adx;

		for (int i = 0; i <= adx; i++) {
			setPixel(x0, y0, color);

			if (p > 0) {
				y0 += stepY;
				p += k2;
			}
			else {
				p += k1;
			}
			x0 += stepX;
		}
	}
	else if (ady > adx) { //hlavna je os y
		k1 = 2 * adx;
		k2 = k1 - 2 * ady;
		p = k1 - ady;

		for (int i = 0; i <= ady; i++) {
			setPixel(x0, y0, color);
			
			if (p > 0) {
				x0 += stepX;
				p += k2;
			}
			else {
				p += k1;
			}
			y0 += stepY;
		}
	}

}

QColor ViewerWidget::getNearestNeighborColor(int x, int y, TVertex T0, TVertex T1, TVertex T2)
{
	QColor color;

	double d0 = (x - T0.point.x()) *(x - T0.point.x()) + (y - T0.point.y()) * (y - T0.point.y());
	double d1 = (x - T1.point.x()) * (x - T1.point.x()) + (y - T1.point.y()) * (y - T1.point.y());
	double d2 = (x - T2.point.x()) * (x - T2.point.x()) + (y - T2.point.y()) * (y - T2.point.y());

	if (d0 < d1 && d0 < d2) {
		color = T0.color;
	}else if (d1 < d0 && d1 < d2) {
		color = T1.color;
	}else if(d2 < d0 && d2 < d1) {
		color = T2.color;
	}else { color = T2.color; }

	return color;
}
QColor ViewerWidget::getBarycentricColor(int x, int y, TVertex T0, TVertex T1, TVertex T2)
{
	double ux = T1.point.x() - T0.point.x();
	double uy = T1.point.y() - T0.point.y();
	double vx = T2.point.x() - T0.point.x();
	double vy = T2.point.y() - T0.point.y();

	double nz = ux * vy - uy * vx;

	double A = abs(nz) / 2.0;
	if (A < 0.0001) return T0.color; //if they are on 1 line
	double A0 = abs((T1.point.x() - x)*(T2.point.y() - y) - (T1.point.y() - y)*(T2.point.x() - x)) / 2.0; //P-T1-T2
	double A1 = abs((T0.point.x() - x) * (T2.point.y() - y) - (T0.point.y() - y) * (T2.point.x() - x)) / 2.0; //P-T0-T2

	double l0 = A0 / A;
	double l1 = A1 / A;
	double l2 = 1.0 - l0 - l1;

	int r = qBound(0, (int)(l0 * T0.color.red() + l1 * T1.color.red() + l2 * T2.color.red()), 255);
	int g = qBound(0, (int)(l0 * T0.color.green() + l1 * T1.color.green() + l2 * T2.color.green()), 255);
	int b = qBound(0, (int)(l0 * T0.color.blue() + l1 * T1.color.blue() + l2 * T2.color.blue()), 255);

	return QColor(r, g, b);
}

//Slots
void ViewerWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	if (!img || img->isNull()) return;

	QRect area = event->rect();
	painter.drawImage(area, *img, area); //aktualny stav vykreslime
}