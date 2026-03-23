#pragma once //predstavuje hlavne okno

#include <QtWidgets/QMainWindow>
#include <QtWidgets>
#include "ui_ImageViewer.h"
#include "ViewerWidget.h"

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

private slots:
	void on_actionOpen_triggered();
	void on_actionSave_as_triggered();
	void on_actionClear_triggered();
	void on_actionExit_triggered();

	void on_pushButtonClearObject_clicked();
	void on_pushButtonFill_clicked();
	void on_pushButtonRotate_clicked();
	void on_pushButtonScale_clicked();
	void on_pushButtonSlope_clicked();
	void on_pushButtonSymmetry_clicked();

	void on_pbColorVertex1_clicked();
	void on_pbColorVertex2_clicked();
	void on_pbColorVertex3_clicked();

	//Added for reseting active state (selecting another drawing option)
	void on_toolButtonDrawLine_clicked();
	void on_toolButtonDrawCircle_clicked();

	//Tools slots
	void on_pushButtonSetColor_clicked();
};
