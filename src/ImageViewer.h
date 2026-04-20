#pragma once //predstavuje hlavne okno

#include <QtWidgets/QMainWindow>
#include <QtWidgets>
#include "ui_ImageViewer.h"
#include "ViewerWidget.h"
#include "Object3D.h"

class ImageViewer : public QMainWindow
{
	Q_OBJECT

public:
	ImageViewer(QWidget* parent = Q_NULLPTR);
	~ImageViewer() { delete ui; }

	void setLastMousePos(int newX, int newY) { lastMousePos.setX(newX); lastMousePos.setY(newY);  }
	QPoint getLastMousePos() { return lastMousePos; }
private:
	Ui::ImageViewerClass* ui;
	ViewerWidget* vW; //Kazdy imViewer obsahuje jeden viewerWidget

	QColor globalColor;
	QSettings settings;
	QMessageBox msgBox;

	//Triangle
	QColor colorV1 = Qt::red;
	QColor colorV2 = Qt::blue;
	QColor colorV3 = Qt::green;

	QPoint lastMousePos;
	bool isDragging = false;

	Object3D* currentObject = nullptr; 

	//Event filters
	bool eventFilter(QObject* obj, QEvent* event);

	//ViewerWidget Events, chceme zachytit vstup od uzivatiela
	bool ViewerWidgetEventFilter(QObject* obj, QEvent* event);
	void ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event);
	void ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event);
	void ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event);
	void ViewerWidgetLeave(ViewerWidget* w, QEvent* event);
	void ViewerWidgetEnter(ViewerWidget* w, QEvent* event);
	void ViewerWidgetWheel(ViewerWidget* w, QEvent* event);

	//ImageViewer Events
	void closeEvent(QCloseEvent* event);

	//Image functions
	bool openImage(QString filename);
	bool saveImage(QString filename);

	void updateCanvas(ViewerWidget* w);

	//3D
	void render3D();

private slots:
	void on_actionOpen_triggered();
	void on_actionSave_as_triggered();
	void on_actionClear_triggered();
	void on_actionExit_triggered();

	void on_pushButtonClearObject_clicked();
	void on_pushButtonFill_clicked();

	//Transformations
	void on_pushButtonRotate_clicked();
	void on_pushButtonScale_clicked();
	void on_pushButtonSlope_clicked();
	void on_pushButtonSymmetry_clicked();

	//Triangle settings
	void on_pbColorVertex1_clicked();
	void on_pbColorVertex2_clicked();
	void on_pbColorVertex3_clicked();

	//Curve
	void on_spinBoxIndex_valueChanged(int i);
	void on_spinBoxAngle_valueChanged(int angle);
	void on_dsbLength_valueChanged(double l);

	//Added for reseting active state 
	void on_toolButtonDrawLine_clicked();
	void on_toolButtonDrawPolygon_clicked();
	void on_toolButtonDrawCircle_clicked();
	void on_toolButtonHermite_clicked();
	void on_toolButtonBezier_clicked();
	void on_toolButtonCoonse_clicked();

	//3D
	void on_pbCubeSave_clicked();
	void on_pbSphereSave_clicked();

	void on_pbOpenVTK_clicked();

	void on_sliderThetaZenit_valueChanged(int value) { render3D(); };
	void on_sliderPhiAzimuth_valueChanged(int value) { render3D(); };
	void on_dsbDistance_valueChanged(double value) { render3D(); };

	//Tools slots
	void on_pushButtonSetColor_clicked();

};
