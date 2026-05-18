#include "Object3D.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

void Object3D::generateCube(double size)
{
	// Creating verticies
	double size2 = size / 2;
	unsigned int idx = 0;

	for (int k = 0; k < 2; k++) {
		for (int j = 0; j < 2; j++) {
			for (int i = 0; i < 2; i++) {
				Vertex* v = new Vertex();
				v->x = (i == 0) ? size2 : -size2;
				v->y = (j == 0) ? size2 : -size2;
				v->z = (k == 0) ? size2 : -size2;
				v->id = idx;
				vertices.push_back(v);
				idx++;
			}
		}
	}

	// Forming faces (triangles)
	int triangleVert[12][3] = {
		{0,1,2}, {2,1,3}, {4,0,6}, {6,0,2},
		{5,4,7}, {7,4,6}, {1,5,3}, {3,5,7},
		{4,5,0}, {0,5,1}, {2,3,6}, {6,3,7}
	};

	for (int i = 0; i < 12; i++) {
		triangulateFace(triangleVert[i][0], triangleVert[i][1], triangleVert[i][2]);
	}

	//pairing();

}
void Object3D::generateUVSphere(int M, int P, double R) // M-meridians(vertices), P-parallels(circles), R-radius
{
	clear();

	double theta = M_PI / (P + 1); //P=5 -> 6 oblasti medzi nimi a polami (poly neberieme do P)
	double phi = 2 * M_PI / M;
	int idx = 0;

	// Creating verticies
	Vertex* northPole = new Vertex();
	northPole->x = 0; northPole->y = R; northPole->z = 0;
	northPole->id = idx;
	vertices.push_back(northPole);
	idx++;

	for (int j = 1; j <= P; j++) { //parallels, for theta
		for (int i = 0; i < M; i++) { //meridians, for phi
			double currTheta = j * theta;
			double currPhi = i * phi;
			Vertex* v = new Vertex();

			v->x = R * sin(currTheta) * cos(currPhi);
			v->y = R * cos(currTheta);
			v->z = R * sin(currTheta) * sin(currPhi);
			v->id = idx;

			vertices.push_back(v);
			idx++;
		}
	}

	Vertex* southPole = new Vertex();
	southPole->x = 0; southPole->y = -R; southPole->z = 0;
	southPole->id = idx;
	vertices.push_back(southPole);
	idx++;

	// Triangulation - store data in parallel bands
	int northId = 0;
	for (int i = 0; i < M; i++) {
		int curr = 1 + i;
		int next = 1 + (1 + i) % M;

		triangulateFace(next, northId, curr); //CCW
	}

	for (int j = 0; j < P - 1; j++) { 
		for (int i = 0; i < M; i++) { //triangles share v1-v3 edge
			int v1 = 1 + j * M + i; //upper left
			int v2 =  1 + (j + 1) * M + i; //lower left
			int v3 = 1 + (j + 1) * M + (i + 1) % M; //lower right
			int v4 = 1 + j * M + (i + 1) % M; //upper right

			triangulateFace(v3, v1, v2);
			triangulateFace(v1, v3, v4); 
		}
	}

	int southId = vertices.size() - 1;
	int lastRingStart = southId - M; //idx v 1D vektori
	for (int i = 0; i < M; i++) {
		int curr = lastRingStart + i;
		int next = lastRingStart + (i + 1) % M;

		triangulateFace(next, curr, southId); 
	}

	//pairing();
	computeVertexNormals();
}

void Object3D::triangulateFace(int idx1, int idx2, int idx3)
{
	Face* f = new Face;
	H_edge* e1 = new H_edge;
	H_edge* e2 = new H_edge;
	H_edge* e3 = new H_edge;

	e1->edge_next = e2;
	e1->edge_prev = e3;
	e1->vert_origin = vertices[idx1];
	e1->face = f;
	halfEdges.push_back(e1);
	vertices[idx1]->edge = e1;

	e2->edge_next = e3;
	e2->edge_prev = e1;
	e2->vert_origin = vertices[idx2];
	e2->face = f;
	halfEdges.push_back(e2);
	vertices[idx2]->edge = e2;

	e3->edge_next = e1;
	e3->edge_prev = e2;
	e3->vert_origin = vertices[idx3];
	e3->face = f;
	halfEdges.push_back(e3);
	vertices[idx3]->edge = e3;

	f->edge = e1;
	faces.push_back(f);
}
void Object3D::pairing()
{
	int total = halfEdges.size();
	for (int j = 0; j < total; j++) { //36 halfEdges in total
		H_edge* e1 = halfEdges[j];
		if (e1->pair != nullptr) continue;

		for (int i = 0; i < total; i++) {
			H_edge* e2 = halfEdges[i];

			if (i == j || e2->pair != nullptr) continue;
			else if (e1->vert_origin == e2->edge_next->vert_origin && e1->edge_next->vert_origin == e2->vert_origin) {
				e1->pair = e2;
				e2->pair = e1;
				break;
			}
		}
	}

	int pairedCount = 0;
	for (auto e : halfEdges) {
		if (e->pair != nullptr) pairedCount++;
	}
	qDebug() << "Total edges:" << halfEdges.size() << "Paired:" << pairedCount;
}

void Object3D::saveToVTK(const QString& filename)
{
	//QFile file(filename);
	//if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
	//	QTextStream out(&file);
	//	out << "# vtk DataFile Version 3.0\n";

	std::ofstream file(filename.toStdString());
	if (!file.is_open()) return;

	file << "# vtk DataFile Version 3.0\n";
	file << "Object3D Export\n";
	file << "ASCII\n";
	file << "DATASET POLYDATA\n";

	file << "POINTS " << vertices.size() << " double\n";
	for (Vertex* v : vertices) {
		file << v->x << " " << v->y << " " << v->z << "\n";
	}
	
	file << "POLYGONS " << faces.size() << " " << faces.size() * 4 << "\n";
	//for cube: 12 triangles, 4 nums each, "3" is the size, followed by vertex indices
	for (Face* f : faces) {
		H_edge* e1 = f->edge;
		H_edge* e2 = e1->edge_next;
		H_edge* e3 = e2->edge_next;

		file << "3 " << e1->vert_origin->id << " " 
			<< e2->vert_origin->id << " " 
			<< e3->vert_origin->id << "\n";
	}

	file.close();
}
void Object3D::loadFromVTK(QString filename)
{
	std::ifstream file(filename.toStdString());
	if (!file.is_open()) {
		throw std::runtime_error("Could not open file: " + filename.toStdString());
	}

	clear();
	std::string word;

	while (file >> word) {
		if (word.empty()) continue;

		if (word == "POINTS") {
			int count;
			std::string type;
			file >> count >> type; //reads one by one

			for (int i = 0; i < count; ++i) {
				double x, y, z;
				if (!(file >> x >> y >> z)) break;

				Vertex* v = new Vertex;
				v->id = i;
				v->x = x;
				v->y = y;
				v->z = z;
				vertices.push_back(v);
			}
		}

		else if (word == "POLYGONS") {
			int numPolygons, totalData;
			file >> numPolygons >> totalData;

			for (int i = 0; i < numPolygons; ++i) {
				int n, idx1, idx2, idx3; // n is 3 for triangle 
				
				if (file >> n >> idx1 >> idx2 >> idx3) {
					if (idx1 >= vertices.size() || idx2 >= vertices.size() || idx3 >= vertices.size()) {
						throw std::runtime_error("Corrupted VTK file: vertex index out of bounds");
					}
					triangulateFace(idx1, idx2, idx3);
				}
			}
		}
	}

	pairing();
	computeVertexNormals();

	if (halfEdges.size() == 36) {
		type = Object3DType::Cube;
		qDebug() << "It's a cube";
	}
	else if (halfEdges.size() > 36) {
		type = Object3DType::Sphere;
		qDebug() << "It's probably a sphere";
	}

	file.close();
}
void Object3D::computeVertexNormals()
{	//1 normal for triangle face! 

	//Reset all normals to zero
	for (Vertex* v : vertices) {
		v->N = QVector3D(0, 0, 0);
	}

	//Compute face normal, add it to each of its 3 vertices
	for (Face* f : faces) {
		H_edge* e1 = f->edge;
		H_edge* e2 = e1->edge_next;
		H_edge* e3 = e2->edge_next;

		Vertex* va = e1->vert_origin;
		Vertex* vb = e2->vert_origin;
		Vertex* vc = e3->vert_origin;

		QVector3D A(va->x, va->y, va->z);
		QVector3D B(vb->x, vb->y, vb->z);
		QVector3D C(vc->x, vc->y, vc->z);

		//Face normal weighted by area (cross product len. = 2*area)
		QVector3D faceN = QVector3D::crossProduct(C - A, B - A);
		//faceN is NOT normalized here; longer = bigger triangle = more weight

		va->N += faceN;
		vb->N += faceN;
		vc->N += faceN;
	}

	//Normalize each vertex normal
	for (Vertex* v : vertices) {
		v->N = v->N.normalized();
	}
}

void Object3D::clear() {
	for (auto v : vertices) delete v;
	vertices.clear();

	for (auto h : halfEdges) delete h;
	halfEdges.clear();

	for (auto f : faces) delete f;
	faces.clear();

	type = Object3DType::Generic;
}
