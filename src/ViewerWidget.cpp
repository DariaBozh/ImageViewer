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
void ViewerWidget::clearObject()
{
	polygonPoints.clear();
	transformedPoints.clear();
	drawPolygonActivated = false;
	clear();
}

void ViewerWidget::drawObject(QColor color, int algType)
{
	if (transformedPoints.isEmpty()) return;

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
			drawPolygon(clipped, color, algType);
		}
		break;
	}
	default: break;
	}

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
	update();

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

//Slots
void ViewerWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	if (!img || img->isNull()) return;

	QRect area = event->rect();
	painter.drawImage(area, *img, area); //aktualny stav vykreslime
}