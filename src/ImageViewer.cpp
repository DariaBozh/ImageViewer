#include "ImageViewer.h"

ImageViewer::ImageViewer(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::ImageViewerClass)
{
	ui->setupUi(this);
	vW = new ViewerWidget(QSize(500, 500), ui->scrollArea /*Parent, do vnutra nieco vlozime*/);
	ui->scrollArea->setWidget(vW); //Vlozime platno

	ui->scrollArea->setBackgroundRole(QPalette::Dark); //Pozadie
	ui->scrollArea->setWidgetResizable(false);
	ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded); //Zobrazia sa vtedy, ked potrebne
	ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	vW->setObjectName("ViewerWidget");
	vW->installEventFilter(this);

	globalColor = Qt::blue;
	QString style_sheet = QString("background-color: %1;").arg(globalColor.name(QColor::HexRgb));
	ui->pushButtonSetColor->setStyleSheet(style_sheet);
}

// Event filters
bool ImageViewer::eventFilter(QObject* obj, QEvent* event) //vsetko zdedene s Qobjekt; 
{
	if (obj->objectName() == "ViewerWidget") { //berie vsetky iventy, ale nas zaujima iba v ramci platna
		return ViewerWidgetEventFilter(obj, event);
	}
	return QMainWindow::eventFilter(obj, event);
}

//ViewerWidget Events
bool ImageViewer::ViewerWidgetEventFilter(QObject* obj, QEvent* event)
{
	ViewerWidget* w = static_cast<ViewerWidget*>(obj); //kontrola

	if (!w) {
		return false;
	}

	if (event->type() == QEvent::MouseButtonPress) {
		ViewerWidgetMouseButtonPress(w, event);
	}
	else if (event->type() == QEvent::MouseButtonRelease) {
		ViewerWidgetMouseButtonRelease(w, event);
	}
	else if (event->type() == QEvent::MouseMove) {
		ViewerWidgetMouseMove(w, event);
	}
	else if (event->type() == QEvent::Leave) {
		ViewerWidgetLeave(w, event);
	}
	else if (event->type() == QEvent::Enter) {
		ViewerWidgetEnter(w, event);
	}
	else if (event->type() == QEvent::Wheel) {
		ViewerWidgetWheel(w, event);
	}

	return QObject::eventFilter(obj, event);
}
void ImageViewer::ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
	bool lineSelected = ui->toolButtonDrawLine->isChecked();
	bool circleSelected = ui->toolButtonDrawCircle->isChecked();
	bool polygonSelected = ui->toolButtonDrawPolygon->isChecked();
	
	//Dragging
	if (e->button() == Qt::LeftButton && !w->getDrawPolygonActivated() && !w->getDrawLineActivated() && !w->getTransformedPoints().isEmpty()) {
		isDragging = true;
		setLastMousePos(e->pos().x(), e->pos().y());
		return;
	}

	//Line or circle
	if (e->button() == Qt::LeftButton && (lineSelected || circleSelected )) //Corresponding button is pressed
	{
		if (w->getDrawLineActivated()) {
			if (lineSelected){
				w->drawLine(w->getDrawLineBegin(), e->pos(), globalColor, ui->comboBoxLineAlg->currentIndex());
				w->addPolygonPoint(e->pos());
			}
			else if (circleSelected) {
				w->drawCircle(w->getDrawLineBegin(), e->pos(), globalColor);
			}
			w->setDrawLineActivated(false);
		}
		else {
			w->setDrawLineBegin(e->pos()); //First point
			w->setDrawLineActivated(true);

			w->clearObject();
			w->addPolygonPoint(e->pos());

			w->setPixel(e->pos().x(), e->pos().y(), globalColor);
			w->update();
		}
	}

	//Polygon
	if (polygonSelected) {

		if (e->button() == Qt::LeftButton) {

			if (!w->getDrawPolygonActivated()) {
				w->clearObject();
				w->setDrawPolygonActivated(true);
			}

			w->addPolygonPoint(e->pos());
			int size = w->getPolygonPoints().size();

			if (size > 1) {
				w->drawLine(w->getTransformedPoints()[size - 2], e->pos(), globalColor, ui->comboBoxLineAlg->currentIndex());
			}
			else {
				w->setPixel(e->pos().x(), e->pos().y(), globalColor);
				w->update();
			}
		}

		else if (e->button() == Qt::RightButton) {
			w->setTransformedPoints(w->getPolygonPoints()); // Ініціалізуємо робочий вектор
			w->closePolygon(globalColor, ui->comboBoxLineAlg->currentIndex());
			w->setDrawPolygonActivated(false);
			w->update();
		}
	}

}
void ImageViewer::ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);

	if (e->button() == Qt::LeftButton) {
		isDragging = false;
	}

}
void ImageViewer::ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
	
	if (isDragging) {
		w->displacement(getLastMousePos(), e->pos(), w->getTransformedPoints());
		setLastMousePos(e->pos().x(), e->pos().y());

		updateCanvas(w);
	}

}
void ImageViewer::ViewerWidgetLeave(ViewerWidget* w, QEvent* event)
{
}
void ImageViewer::ViewerWidgetEnter(ViewerWidget* w, QEvent* event)
{
}
void ImageViewer::ViewerWidgetWheel(ViewerWidget* w, QEvent* event)
{
	QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event); // returns angleDelta() with QPoint type
	if (w->getTransformedPoints().isEmpty()) return;

	double scaleFactor = 1;
	if (wheelEvent->angleDelta().y() > 0) {
		scaleFactor *= 1.25; //upward
	}
	else {
		scaleFactor *= 0.75; //downward
	}
	
	QVector<QPoint> scaled = w->scale(w->getTransformedPoints(), scaleFactor, scaleFactor);
	w->setTransformedPoints(scaled);
	
	w->clear();
	w->drawPolygon(scaled, globalColor, ui->comboBoxLineAlg->currentIndex());
	w->update();
}

//ImageViewer Events
void ImageViewer::closeEvent(QCloseEvent* event)
{
	if (QMessageBox::Yes == QMessageBox::question(this, "Close Confirmation", "Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::No))
	{
		event->accept();
	}
	else {
		event->ignore();
	}
}

//Image functions
bool ImageViewer::openImage(QString filename)
{
	QImage loadedImg(filename);
	if (!loadedImg.isNull()) {
		return vW->setImage(loadedImg);
	}
	return false;
}
bool ImageViewer::saveImage(QString filename)
{
	QFileInfo fi(filename);
	QString extension = fi.completeSuffix();

	QImage* img = vW->getImage();
	return img->save(filename, extension.toStdString().c_str());
}

void ImageViewer::updateCanvas(ViewerWidget* w)
{ // Draws new polygon(or line) when we moving it or 
	w->clear();
	w->drawPolygon(w->getTransformedPoints(), globalColor, ui->comboBoxLineAlg->currentIndex());
	w->update();
}

//Slots
void ImageViewer::on_actionOpen_triggered()
{
	QString folder = settings.value("folder_img_load_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm);;All files (*)";
	QString fileName = QFileDialog::getOpenFileName(this, "Load image", folder, fileFilter);
	if (fileName.isEmpty()) { return; }

	QFileInfo fi(fileName);
	settings.setValue("folder_img_load_path", fi.absoluteDir().absolutePath()); //cata a zapisuje do registrov

	if (!openImage(fileName)) {
		msgBox.setText("Unable to open image.");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
	}
}
void ImageViewer::on_actionSave_as_triggered()
{
	QString folder = settings.value("folder_img_save_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm);;All files (*)";
	QString fileName = QFileDialog::getSaveFileName(this, "Save image", folder, fileFilter);
	if (!fileName.isEmpty()) {
		QFileInfo fi(fileName);
		settings.setValue("folder_img_save_path", fi.absoluteDir().absolutePath());

		if (!saveImage(fileName)) {
			msgBox.setText("Unable to save image.");
			msgBox.setIcon(QMessageBox::Warning);
		}
		else {
			msgBox.setText(QString("File %1 saved.").arg(fileName));
			msgBox.setIcon(QMessageBox::Information);
		}
		msgBox.exec();
	}
}
void ImageViewer::on_actionClear_triggered()
{
	vW->clear();
}
void ImageViewer::on_actionExit_triggered()
{
	this->close();
}

//Added for reseting active state (selecting another drawing option)
void ImageViewer::on_toolButtonDrawLine_clicked()
{
	vW->setDrawLineActivated(false);
}
void ImageViewer::on_toolButtonDrawCircle_clicked()
{
	vW->setDrawLineActivated(false);
}

void ImageViewer::on_pushButtonClearObject_clicked()
{
	vW->clearObject();
	vW->update();
}
void ImageViewer::on_pushButtonRotate_clicked()
{
	if (vW->getTransformedPoints().isEmpty()) return;
	double angle = ui->SpinBoxDegreesRotate->value();
	
	QVector<QPoint> polygonRotated = vW->rotation(vW->getTransformedPoints(), angle);
	vW->setTransformedPoints(polygonRotated);
	
	updateCanvas(vW);

}
void ImageViewer::on_pushButtonScale_clicked()
{
	if (vW->getTransformedPoints().isEmpty()) return;

	double dx = ui->dsb_xScale->value();
	double dy = ui->dsb_yScale->value();

	QVector<QPoint> polygonScaled = vW->scale(vW->getTransformedPoints(), dx, dy);
	vW->setTransformedPoints(polygonScaled);
	
	updateCanvas(vW);

}
void ImageViewer::on_pushButtonSlope_clicked()
{
	if (vW->getTransformedPoints().isEmpty()) return;

	double d = ui->dsb_Slope->value();
	
	QVector<QPoint> polygonSloped = vW->share(vW->getTransformedPoints(), d);
	vW->setTransformedPoints(polygonSloped);
	
	updateCanvas(vW);

}

void ImageViewer::on_pushButtonSetColor_clicked()
{
	QColor newColor = QColorDialog::getColor(globalColor, this);
	if (newColor.isValid()) {
		QString style_sheet = QString("background-color: %1;").arg(newColor.name(QColor::HexRgb));
		ui->pushButtonSetColor->setStyleSheet(style_sheet);
		globalColor = newColor;
	}
}