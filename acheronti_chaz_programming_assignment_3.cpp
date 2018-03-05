#include <GL/glut.h>

#include <iostream>
#include <string>
#include <vector>
#include <math.h>

#include <fstream>

using namespace std;

/* Chaz Acheronti */

// struct to hold x y and z coordinates of vertex
struct vertex { double x, y, z; };

// a struct for one face
using face = vector <vertex>;

// Constants
const int MAX_CHARS_PER_VALUE = 32;
vector <vertex> vertices;
vector <face> faces;
bool wireframe = false;
bool modeSet = true;

double vertex_scalar = 10.0;

struct camera {
	float pos_x, pos_y, pos_z;
	float radius;
	float angle_degrees, angle_radians;
	float zoom_speed, rot_speed;
};

camera cam {
	0.0f, 0.0f, 0.0f,	// position
	2.0f,				// radius
	0.0f, 0.0f,			// angle in degr and rad
	0.025f, 2.5f		// speeds
};

// given a .obj file, parse it and save the verices and faces
bool read_file(string file_name) {
	if (!file_name.find(".obj") || !file_name.find(".OBJ")) {
		cout << "This program can only parse .obj files!\n";
		return false;
	}

	ifstream obj_file(file_name); // open the file
	// if the file is not good, return false
	if (!obj_file.good())
		return false;
	
	// variables for saving pare information
	char ch;
	char value[MAX_CHARS_PER_VALUE];
	// flags for determining the current line operations to be performed
	bool commentLine = false, vertLine = false, faceLine = false;
	
	// temp storage for vertecies
	cout << "Parsing . . . ";
	while (obj_file >> noskipws >> ch) {
		if (ch == '#' || ch == 'g' || ch == 'm' || ch == 'u' || ch == 's') { // ignoring comments, g, use, s, and m
			while (obj_file >> noskipws >> ch) { if (ch == '\n') break; }
		}
		if (ch == 'v') { // parsing the vertex type
			string float_string;
			float temp_vertex[3]{ 0.0f, 0.0f, 0.0f }; // store parsed values of x,y,z 
			int id = 0; // index of vertex (x,y,z)
			
			obj_file >> noskipws >> ch; // skip the whitespace
			// create the vertex
			while (obj_file >> noskipws >> ch) {				
				if (ch == '\n') { // we hit a new line
					double value = stod(float_string); // get the float value of the string
					temp_vertex[id] = value; // insert this last value
					// insert a vertex struct to the vertices vector!
					vertices.push_back({temp_vertex[0], temp_vertex[1], temp_vertex[2]});
					break; // end this vertex loop
				}
				if (ch == ' ') { // we hit a whitespace
					double value = stod(float_string); // get the float value of the string
					temp_vertex[id] = value; // insert the value into the temp_vector
					id++; // increment the index of temp_vertex
					float_string.clear(); // clear the temp string
				}
				else { float_string.push_back(ch); /* append the float string with current char */ }
			}
		}
		if (ch == 'f') {
			string int_string;
			face new_face; // store parsed values until ready to push_back into face vector
			bool inserted = false;

			obj_file >> noskipws >> ch; // skip the whitespace
			while (!inserted && obj_file >> noskipws >> ch) {
				if (ch == ' ') { // this is an intermidiary value, save in temp_face
					int value = stoi(int_string);
					new_face.push_back(vertices[value - 1]);
					int_string.clear();
				}
				if (ch == '\n') { // this is the last value, push the face to the vector
					int value = stoi(int_string); // get vert - 1 index
					new_face.push_back(vertices[value - 1]); // insert parsed value to face_verts
					faces.push_back( new_face );
					inserted = true;
				}
				else { int_string.push_back(ch); /* this is a char (int) value, append to int_string*/ }
			}
			cout << "added face " << faces.size() - 1 << endl;
		}
	} cout << "done!\n";
	obj_file.close();
	return true;
}

// draw a single vertex
void draw_vertex(const vertex& vert) {
	glVertex3f(vert.x * vertex_scalar, vert.y * vertex_scalar, vert.z * vertex_scalar);
}

// draw all faces
void draw_faces() {
	for (int f = 0; f < faces.size(); f++) {
		glBegin(GL_POLYGON);
		for (int v = 0; v < faces[f].size(); v++) {
			draw_vertex(faces[f][v]);
		}
		glEnd();
	}
}

// drawing the mesh
void draw_obj() {
	// set the drawing mode on a modeSet event
	if (!modeSet) {
		if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // set polygon mode to unfilled
		else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // set polygon mode to unfilled
		modeSet = true;
	}
	draw_faces();
}

// init gl and lighting
void init(void) {
	glShadeModel(GL_SMOOTH);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_COLOR_MATERIAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	GLfloat lightPos[4] = { -1.0, 1.0, 0.5, 0.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, (GLfloat *)&lightPos);
	glEnable(GL_LIGHT1);
	GLfloat lightAmbient1[4] = { 0.0, 0.0,  0.0, 0.0 };
	GLfloat lightPos1[4] = { 1.0, 0.0, -0.2, 0.0 };
	GLfloat lightDiffuse1[4] = { 0.5, 0.5,  0.3, 0.0 };
	glLightfv(GL_LIGHT1, GL_POSITION, (GLfloat *)&lightPos1);
	glLightfv(GL_LIGHT1, GL_AMBIENT, (GLfloat *)&lightAmbient1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, (GLfloat *)&lightDiffuse1);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

}

// display the current frame
void display(void) {	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	// calculate position
	cam.angle_radians = cam.angle_degrees * 3.14 / 180.0f;
	cam.pos_x = cam.radius * cos(cam.angle_radians);
	cam.pos_y = vertex_scalar * 0.1f; // height of camera is limited to scale of object
	cam.pos_z = cam.radius * sin(cam.angle_radians);
	
	gluLookAt(cam.pos_x, cam.pos_y, cam.pos_z, 0.0f, cam.pos_y, 0.0f, 0.0f, 1.0f, 0.0f);

	draw_obj();

	glutSwapBuffers();
	glutPostRedisplay();
}

// reshape the window
void reshape(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (h == 0) {
		gluPerspective(80, (float)w, 1.0, 5000.0);
	}
	else {
		gluPerspective(80, (float)w / (float)h, 1.0, 5000.0);
	}
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// keyboard event handlers
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
		case 'w':
			if (wireframe) cout << "Already in wireframe mode.\n";
			else {
				wireframe = true;
				modeSet = false;
				cout << "Changing to wireframe mode! \n";
			}
			break;
		case 's':
			if (wireframe) {
				wireframe = false;
				modeSet = false;
				cout << "Changing to solid mode! \n";
			}
			else cout << "Already in solid mode! \n";
			break;
		case 'q':
			cout << "Scaling the object up!\n";
			vertex_scalar += 0.5f;
			break;
		case 'e':
			cout << "Scaling the object down!\n";
			vertex_scalar -= 0.5f;
			break;
		default:
			break;
	}
}

// arrow key event handlers
void arrow_keys(int key, int x, int y) {
	switch (key) {
		case GLUT_KEY_UP:
			cout << "Radius: " << cam.radius << endl;
			cam.radius -= cam.zoom_speed;
			break;
		case GLUT_KEY_DOWN:
			cout << "Radius: " << cam.radius << endl;
			cam.radius += cam.zoom_speed;
			break;
		case GLUT_KEY_LEFT:
			cout << "Angle: " << cam.angle_degrees << endl;
			cam.angle_degrees += cam.rot_speed;
			if (cam.angle_degrees > 360.0f) cam.angle_degrees = 0.0f;
			break;
		case GLUT_KEY_RIGHT:
			cout << "Angle: " << cam.angle_degrees << endl;
			cam.angle_degrees -= cam.rot_speed;
			if (cam.angle_degrees < 0.0f) cam.angle_degrees = 360.0f;
			break;
		default:
			break;
	}
}

// get input form console or run the bunny obj by default
int main(int argc, char *argv[]) {
	int window;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowSize(1280, 720);
	glutInitWindowPosition(160, 160);
	window = glutCreateWindow("OBJ PARSER & VISUALIZER      C h a z  A c h e r o n t i");
	init();

	// getting the input from command line
	string file_name;
	if (argc > 1) {
		file_name = argv[1];
		cout << "Using " << file_name << " . . . \n";
	}
	else {
		cout << "Usage: acheronti_chaz_..._3 <file_name>\n";
		cout << "Since no file name was specified, using Stanfrod Bunny (bunny.obj) instead! \n";
		file_name = "bunny.obj";
	}

	// Output on the success of file read
	if (read_file(file_name)) cout << "Sucess!\n";
	else cout << "Failed to read file!\n";

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(arrow_keys);

	glutMainLoop();
}