#pragma once
#include <QtWidgets>
#include <math.h>

struct H_edge; //I'll use half-edge representation

struct Vertex {
	double x, y, z;
	H_edge* edge;
	unsigned int id;
};
struct Face { //plocha (trojuholnik)
	H_edge* edge;
};
struct H_edge { //orientovana polohrana
	Vertex* vert_origin;
	Face* face;
	H_edge* edge_prev, * edge_next;
	H_edge* pair = nullptr;
};

class Object3D //class for a model
{
private:
	QVector<Vertex*> vertices;
	QVector<H_edge*> halfEdges;
	QVector<Face*> faces;

public:
	~Object3D() {
		for (auto v : vertices) delete v;
		for (auto h : halfEdges) delete h;
		for (auto f : faces) delete f;
	}

	void generateCube(double size);
	void generateUVSphere(int meridians, int parallels, double R);

	void triangulateFace(int idx1, int idx2, int idx3);
	void pairing();

	void saveToVTK(const QString& filename);
	void loadFromVTK(QString filename);

	void clear();
};

