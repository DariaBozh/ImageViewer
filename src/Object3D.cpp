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

	pairing();

}
void Object3D::generateUVSphere(int M, int P, double R) // M-meridians(vertices), P-parallels(circles), R-radius
{
	clear();

	double theta = M_PI / P;
	double phi = 2 * M_PI / M;
	int idx = 0;

	// Creating verticies
	for (int j = 0; j <= P; j++) { //parallels, for theta
		for (int i = 0; i <= M; i++) { //meridians, for phi
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

	// Store data in parallel bands
	for (int j = 0; j < P; j++) { 
		for (int i = 0; i < M; i++) { //triangles share v1-v3 edge
			Vertex* v1 = vertices[j * (M + 1) + i]; //upper left
			Vertex* v2 = vertices[(j + 1) * (M + 1) + i]; //lower left
			Vertex* v3 = vertices[(j + 1) * (M + 1) + (i + 1)]; //lower right
			Vertex* v4 = vertices[j * (M + 1) + (i + 1)]; //upper right

			triangulateFace(v1->id, v2->id, v3->id);
			triangulateFace(v3->id, v4->id, v1->id); 
		}
	}

	pairing();
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

	file.close();
}
void Object3D::clear() {
	for (auto v : vertices) delete v;
	vertices.clear();

	for (auto h : halfEdges) delete h;
	halfEdges.clear();

	for (auto f : faces) delete f;
	faces.clear();
}
