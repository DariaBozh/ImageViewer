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

	ui->gbTriangle->setEnabled(false);
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
	bool hermiteSelected = ui->toolButtonHermite->isChecked();
	bool bezierSelected = ui->toolButtonBezier->isChecked();
	bool coonsSelected = ui->toolButtonCoonse->isChecked();
	
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
			w->setObjectType(ObjectType::Line);
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

	//Polygon (or triangle specifically)
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
			w->setObjectType(ObjectType::Polygon);
			w->update();

			const QVector<QPoint>& n = w->getTransformedPoints();
			if (n.size() == 3) {
				ui->gbTriangle->setEnabled(true);
				w->setObjectType(ObjectType::Triangle);
			}
		}
	}

	// Curve
	if (hermiteSelected) {

		if (e->button() == Qt::LeftButton) {
			if (!w->getDrawCurveActivated()) {
				w->clearObject();
				w->setDrawCurveActivated(true);
			}

			HermitePoint newP;
			newP.pos = e->pos();
			newP.length = 150;
			newP.angle = 0;
			w->addHermitePoint(newP);

			w->setPixel(e->pos().x(), e->pos().y(), globalColor);
			w->update();
			
		}

		else if (e->button() == Qt::RightButton) {
			int size = w->getHermitePoints().size();

			w->setDrawCurveActivated(false);
			w->setObjectType(ObjectType::HermiteCubic);

			updateCanvas(w);
			
			ui->spinBoxIndex->setRange(1, size);
			ui->spinBoxIndex->setValue(1);
		}
	}
	if (bezierSelected) {

		if (e->button() == Qt::LeftButton) {
			if (w->getObjectType() != ObjectType::None && !w->getDrawCurveActivated()) {
				return;
			}
			if (!w->getDrawCurveActivated()) {
				w->clearObject();
				w->setDrawCurveActivated(true);
				w->setObjectType(ObjectType::None);
			}

			w->addCurvePoint(e->pos());
			w->setPixel(e->pos().x(), e->pos().y(), globalColor);
			w->update();
		}

		else if (e->button() == Qt::RightButton) {
			if (w->getCurvePoints().size() > 1) {
				w->setDrawCurveActivated(false);
				w->setObjectType(ObjectType::BezierCurve);
				updateCanvas(w);
			}
		}
	}
	if (coonsSelected) {

		if (e->button() == Qt::LeftButton) {
			if (w->getObjectType() != ObjectType::None && !w->getDrawCurveActivated()) {
				return;
			}
			if (!w->getDrawCurveActivated()) {
				w->clearObject();
				w->setDrawCurveActivated(true);
				w->setObjectType(ObjectType::None);
			}

			w->addCurvePoint(e->pos());
			w->setPixel(e->pos().x(), e->pos().y(), globalColor);
			w->update();
		}

		else if (e->button() == Qt::RightButton) {
			if (w->getCurvePoints().size() > 3) {
				w->setDrawCurveActivated(false);
				w->setObjectType(ObjectType::CoonsBSpline);
				updateCanvas(w);
			}
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
	
	updateCanvas(w);
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
{ 
	w->clear();
	w->drawObject(globalColor, ui->comboBoxLineAlg->currentIndex()); 
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
	vW->setObjectType(ObjectType::None);
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
	vW->setObjectType(ObjectType::None);
	vW->update();
	ui->gbTriangle->setEnabled(false);
}
void ImageViewer::on_pushButtonFill_clicked()
{
	QVector<QPoint> points = vW->getTransformedPoints().isEmpty() ? vW->getPolygonPoints() : vW->getTransformedPoints();
	if (points.size() < 3) return;

	if (points.size() == 3) {
		TVertex v1 = { points[0], colorV1 };
		TVertex v2 = { points[1], colorV2 };
		TVertex v3 = { points[2], colorV3 };

		int interType = ui->comboBoxInterpol->currentIndex();

		vW->fillTriangle(v1, v2, v3, interType);
	}
	else {
		vW->scanLine(points, globalColor);
	}
	
	updateCanvas(vW);
}

void ImageViewer::on_pushButtonRotate_clicked()
{
	if (vW->getTransformedPoints().isEmpty()) return;
	double angle = ui->SpinBoxDegreesRotate->value();
	QPoint origin = vW->getTransformedPoints()[0];

	QVector<QPoint> polygonRotated = vW->rotation(vW->getTransformedPoints(), angle, origin);
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
void ImageViewer::on_pushButtonSymmetry_clicked()
{
	if (vW->getTransformedPoints().isEmpty()) return;

	QPoint A = vW->getTransformedPoints()[0];
	QPoint B = vW->getTransformedPoints()[1];

	QVector<QPoint> polygonSymmetric = vW->symmetry(A, B, vW->getTransformedPoints());
	vW->setTransformedPoints(polygonSymmetric);

	updateCanvas(vW);
}

void ImageViewer::on_pbColorVertex1_clicked()
{
	QColor c = QColorDialog::getColor(colorV1, this, "Select Color for Vertex 1");
	if (c.isValid()) {
		colorV1 = c;
		ui->pbColorVertex1->setStyleSheet(QString("background-color: %1;").arg(c.name()));
	}
}
void ImageViewer::on_pbColorVertex2_clicked()
{
	QColor c = QColorDialog::getColor(colorV2, this, "Select Color for Vertex 2");
	if (c.isValid()) {
		colorV2 = c;
		ui->pbColorVertex2->setStyleSheet(QString("background-color: %1;").arg(c.name()));
	}
}
void ImageViewer::on_pbColorVertex3_clicked()
{
	QColor c = QColorDialog::getColor(colorV3, this, "Select Color for Vertex 3");
	if (c.isValid()) {
		colorV3 = c;
		ui->pbColorVertex3->setStyleSheet(QString("background-color: %1;").arg(c.name()));
	}
}

void ImageViewer::on_spinBoxIndex_valueChanged(int i)
{
	if (i-1 >= 0 && i-1 < vW->getHermitePoints().size()) {
		HermitePoint p = vW->getHermitePoints()[i-1];

		ui->spinBoxAngle->blockSignals(true);
		ui->spinBoxAngle->setValue(p.angle);
		ui->spinBoxAngle->blockSignals(false);

		ui->dsbLength->blockSignals(true);
		ui->dsbLength->setValue(p.length);
		ui->dsbLength->blockSignals(false);
	}
}
void ImageViewer::on_spinBoxAngle_valueChanged(int a)
{
	int index = ui->spinBoxIndex->value() - 1;

	vW->setHermiteAngle(index, a);
	updateCanvas(vW); 
}
void ImageViewer::on_dsbLength_valueChanged(double l)
{
	int index = ui->spinBoxIndex->value() - 1;

	vW->setHermiteLength(index, l);
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