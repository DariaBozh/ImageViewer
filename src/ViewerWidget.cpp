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
void ViewerWidget::drawCircle(QPoint start, QPoint end, QColor color) //Ñelociselny
{ 
	if (!img || !data) return;

	int radius = static_cast<int>(sqrt(pow(end.x() - start.x(), 2) + pow(end.y() - start.y(), 2)));

	int p = 1 - radius;
	int x = 0;
	int y = radius;
	int twoX = 3;
	int twoY = 2 * radius - 2;

	int x0 = start.x();
	int y0 = start.y();

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