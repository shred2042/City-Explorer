//-------------------------------------------------------------------------------------------------
// Descriere: fisier main
//
// Autor: student
// Data: today
//-------------------------------------------------------------------------------------------------

//incarcator de meshe
#include "lab_mesh_loader.hpp"

//geometrie: drawSolidCube, drawWireTeapot...
#include "lab_geometry.hpp"

//incarcator de shadere
#include "lab_shader_loader.hpp"

//interfata cu glut, ne ofera fereastra, input, context opengl
#include "lab_glut.hpp"

//camera
#include "lab_camera.hpp"

#include "lab_texture_loader.hpp"

//time
#include <ctime>
#include <vector>

#define MAP_SIZE 5000
#define MIN_HEIGHT 50
#define MAX_HEIGHT 500
#define MIN_LENGTH 20
#define MAX_LENGTH 100
#define NEAR_WIDTH 200
#define NEAR_HEIGHT 100
#define FAR_WIDTH 4000
#define FAR_HEIGHT 2500
#define FAR_DIST 2000
#define NEAR_DIST 50
#define SHIP_DISTANCE 6
#define SHIP_HEIGHT 2
#define SHIP_WIDTH 1
#define PI 3.1415
#define SPOT_ANGLE 0.9

struct CamVertexFormat{ 
		glm::vec3 position, color; 
		CamVertexFormat(const glm::vec3 &p, const glm::vec3 &c){ 
			position=p; color=c;
		} 
};

struct SceneObject{
		glm::vec3 position;
		int length, height, width;
		SceneObject(const glm::vec3 &p, const int l, const int h, const int w) {
			position = p;
			length = l;
			height = h;
			width = w;
		}
};

class Laborator : public lab::glut::WindowListener{

//variabile
private:
	glm::mat4 model_matrix, projection_matrix;											//matrici 4x4 pt modelare vizualizare proiectie
	lab::Camera cam;
	unsigned int gl_program_shader;														//id-ul de opengl al obiectului de tip program shader
	unsigned int screen_width, screen_height;
	std::vector<struct SceneObject> scene_objects;
	std::vector<unsigned int> object_vao, object_vbo, object_ibo, object_num_indices;						//containere opengl pentru vertecsi, indecsi si stare	
	std::vector<unsigned int> tex_type;
	unsigned int texture_building[4];
	unsigned int ship_vao, ship_vbo, ship_ibo, ship_num_indices;
	glm::vec3 position, forward, up;
	std::vector<float> angleY, angleZ;
	unsigned int ground_vbo, ground_ibo, ground_vao, ground_num_indices;
	std::vector<CamVertexFormat> ground;
	glm::vec3 startEye, startCenter;
	glm::vec3 overviewEye;
	unsigned int curr_cam;
	unsigned int state;
	int in_transition;
	bool tps;
	bool construction_matrix[MAP_SIZE];
	float A1, A2, A3, A4, A5, A6, B1, B2, B3, B4, B5, B6, C1, C2, C3, C4, C5, C6, D1, D2, D3, D4, D5, D6;
	unsigned int texture_red, texture_green, texture_ground, texture_ship;

//metode
public:
	
	//constructor .. e apelat cand e instantiata clasa
	Laborator(){
		//setari camera default
		startEye = glm::vec3(MAP_SIZE/2, 500, 0);
		startCenter = glm::vec3(MAP_SIZE/4, 0, MAP_SIZE/2);
		cam.set(startEye, startCenter, glm::vec3(0, 1, 0));
		tps = true;		
		position = glm::vec3(0, 0, 0);
		overviewEye = glm::vec3(MAP_SIZE/2, MAP_SIZE, MAP_SIZE/2);
		//setari pentru desenare, clear color seteaza culoarea de clear pentru ecran (format R,G,B,A)
		glClearColor(0.2,0.2,0.2,1);
		glClearDepth(1);			//clear depth si depth test (nu le studiem momentan, dar avem nevoie de ele!)
		glEnable(GL_DEPTH_TEST);	//sunt folosite pentru a determina obiectele cele mai apropiate de camera (la curs: algoritmul pictorului, algoritmul zbuffer)
		
		//incarca un shader din fisiere si gaseste locatiile matricilor relativ la programul creat
		gl_program_shader = lab::loadShader("shadere\\shader_vertex.glsl", "shadere\\shader_fragment.glsl");

		texture_building[0] = lab::loadTextureBMP("resurse\\building1.bmp");
		texture_building[1] = lab::loadTextureBMP("resurse\\building2.bmp");
		texture_building[2] = lab::loadTextureBMP("resurse\\building3.bmp");
		texture_building[3] = lab::loadTextureBMP("resurse\\building4.bmp");

		texture_red = lab::loadTextureBMP("resurse\\red.bmp");
		texture_green = lab::loadTextureBMP("resurse\\green.bmp");
		texture_ground = lab::loadTextureBMP("resurse\\ground.bmp");
		texture_ship = lab::loadTextureBMP("resurse\\ship.bmp");

		//construieste scena
		buildScene(MAP_SIZE);
		//incarca un mesh		
		lab::loadObj("resurse\\SpaceShip.obj",ship_vao, ship_vbo, ship_ibo, ship_num_indices);	
		
		//construieste groundul
		ground.push_back(CamVertexFormat(glm::vec3(0, 0, 0), glm::vec3(0.18, 0.5, 1)));
		ground.push_back(CamVertexFormat(glm::vec3(MAP_SIZE, 0, 0), glm::vec3(0.18, 0.5, 1)));
		ground.push_back(CamVertexFormat(glm::vec3(MAP_SIZE, 0, MAP_SIZE), glm::vec3(0.18, 0.5, 1)));
		ground.push_back(CamVertexFormat(glm::vec3(MAP_SIZE, 0, MAP_SIZE), glm::vec3(0.18, 0.5, 1)));
		ground.push_back(CamVertexFormat(glm::vec3(0, 0, MAP_SIZE), glm::vec3(0.18, 0.5, 1)));
		ground.push_back(CamVertexFormat(glm::vec3(0, 0, 0), glm::vec3(0.18, 0.5, 1)));
				
		ground_num_indices = ground.size();

		//construieste geometria groundului
		glGenVertexArrays(1,&ground_vao);
		glBindVertexArray(ground_vao);
		glGenBuffers(1,&ground_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, ground_vbo);
		glGenBuffers(1,&ground_ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ground_ibo);
		std::vector<unsigned int> indices2; for(unsigned int i=0;i<ground.size();i++) indices2.push_back(i);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ground.size()*sizeof(unsigned int),&indices2[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(CamVertexFormat),(void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(CamVertexFormat),(void*)(sizeof(float)*3));
		glBindVertexArray(ground_vao);
		glBindBuffer(GL_ARRAY_BUFFER, ground_vbo);
		glBufferData(GL_ARRAY_BUFFER, ground.size()*sizeof(CamVertexFormat),&ground[0],GL_STATIC_DRAW);

		glBindVertexArray(0);

		//matrici de modelare si vizualizare
		model_matrix = glm::mat4(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);

		//desenare wireframe
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glLineWidth(5);
		glPointSize(5);
	}

	//destructor .. e apelat cand e distrusa clasa
	~Laborator(){
		//distruge shader
		glDeleteProgram(gl_program_shader);

		//distruge mesh incarcat
		for (int i = 0; i < object_vao.size(); i++) {
			glDeleteBuffers(1,&object_vbo[i]);
			glDeleteBuffers(1,&object_ibo[i]);
			glDeleteVertexArrays(1,&object_vao[i]);
		}
		glDeleteBuffers(1,&ground_vbo);
		glDeleteBuffers(1,&ground_ibo);
		glDeleteVertexArrays(1,&ground_vao);
		glDeleteTextures(1, &texture_building[0]);
		glDeleteTextures(1, &texture_building[1]);
		glDeleteTextures(1, &texture_building[2]);
		glDeleteTextures(1, &texture_building[3]);

		glDeleteTextures(1, &texture_red);
		glDeleteTextures(1, &texture_green);
	}

	void buildScene(unsigned int size) {
		
		std::vector<CamVertexFormat> object;
		for (int i = 1; i < MAP_SIZE; i=i + 100) {
			for (int j = 0; j < MAP_SIZE; j = j + 100) {
				srand(time(NULL)*i + j);
				int h = rand()% (MAX_HEIGHT - MIN_HEIGHT) + MIN_HEIGHT;

				//adauga la structura de verificare a frustrumului
				scene_objects.push_back(SceneObject(glm::vec3(i, 0, j), 50, 50, h));
				tex_type.push_back(h%4);
				//fata de jos
				object.push_back(CamVertexFormat(glm::vec3(i, 0, j), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i + 50, 0, j), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i + 50, 0, j + 50), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i + 50, 0, j + 50), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i, 0, j + 50), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i, 0, j), glm::vec3(1, 0, 0)));
				//fete laterale
				object.push_back(CamVertexFormat(glm::vec3(i, 0, j), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i, h, j), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i + 50, h, j), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i + 50, h, j), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i + 50, 0, j), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i, 0, j), glm::vec3(1, 0, 0)));

				object.push_back(CamVertexFormat(glm::vec3(i, 0, j + 50), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i, h, j + 50), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i + 50, h, j + 50), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i + 50, h, j + 50), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i + 50, 0, j + 50), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i, 0, j + 50), glm::vec3(1, 0, 0)));

				object.push_back(CamVertexFormat(glm::vec3(i, 0, j), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i, 0, j + 50), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i, h, j + 50), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i, h, j + 50), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i, h, j), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i, 0, j), glm::vec3(1, 0, 0)));

				object.push_back(CamVertexFormat(glm::vec3(i + 50, 0, j), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i + 50, 0, j + 50), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i + 50, h, j + 50), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i + 50, h, j + 50), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i + 50, h, j), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i + 50, 0, j), glm::vec3(1, 0, 0)));

				//fata superioara
				object.push_back(CamVertexFormat(glm::vec3(i, h, j), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i + 50, h, j), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i + 50, h, j + 50), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i + 50, h, j + 50), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i, h, j + 50), glm::vec3(1, 0, 0)));
				object.push_back(CamVertexFormat(glm::vec3(i, h, j), glm::vec3(1, 0, 0)));


				//construieste geometria
				object_num_indices.push_back(object.size());

				//construieste geometria objectului
				object_vao.push_back(i);
				glGenVertexArrays(1,&object_vao[object_num_indices.size()-1]);
				glBindVertexArray(object_vao[object_num_indices.size()-1]);
				object_vbo.push_back(i);
				glGenBuffers(1,&object_vbo[object_num_indices.size()-1]);
				glBindBuffer(GL_ARRAY_BUFFER, object_vbo[object_num_indices.size()-1]);
				object_ibo.push_back(i);
				glGenBuffers(1,&object_ibo[object_num_indices.size()-1]);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object_ibo[object_num_indices.size()-1]);
				std::vector<unsigned int> indices; for(unsigned int i=0;i<object.size();i++) indices.push_back(i);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, object.size()*sizeof(unsigned int),&indices[0], GL_STATIC_DRAW);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(CamVertexFormat),(void*)0);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(CamVertexFormat),(void*)(sizeof(float)*3));
				glBindVertexArray(object_vao[object_num_indices.size()-1]);
				glBindBuffer(GL_ARRAY_BUFFER, object_vbo[object_num_indices.size()-1]);
				glBufferData(GL_ARRAY_BUFFER, object.size()*sizeof(CamVertexFormat),&object[0],GL_STATIC_DRAW);
				object.clear();
			}
		}
	}
	
	//--------------------------------------------------------------------------------------------
	//functii de cadru ---------------------------------------------------------------------------

	//functie chemata inainte de a incepe cadrul de desenare, o folosim ca sa updatam situatia scenei ( modelam/simulam scena)
	void notifyBeginFrame(){
		//rotatie - comentati daca o implementati in VS
		static float angle=0;
		angle = 0.1f;
	}
	//functia de afisare (lucram cu banda grafica)
	void notifyDisplayFrame(){
		
		//pe tot ecranul
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		
		
		//cadru normal
		{
			glViewport(0,0, screen_width, screen_height);
			
			//foloseste shaderul
			glUseProgram(gl_program_shader);
				
			//trimite variabile uniforme la shader
			glUniformMatrix4fv(glGetUniformLocation(gl_program_shader,"model_matrix"),1,false,glm::value_ptr(glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)));
			glUniformMatrix4fv(glGetUniformLocation(gl_program_shader,"view_matrix"),1,false,glm::value_ptr(cam.getViewMatrix()));
			glUniformMatrix4fv(glGetUniformLocation(gl_program_shader,"projection_matrix"),1,false,glm::value_ptr(projection_matrix));
			glUniform1i(glGetUniformLocation(gl_program_shader,"flag"), 0);
		
			

			if (tps) {
				position = cam.getPosition();
				forward = cam.getForward();
				up = cam.getUp();
			}			

			//determina cele 6 planuri ale frustrumului
			determinePlanes(position);
			if (tps) {
				//bind obiect
				//elementele statice
				//deseneaza nava spatiala
				glActiveTexture(GL_TEXTURE0+5);
				glBindTexture(GL_TEXTURE_2D, texture_ship);
				glUniform1i(glGetUniformLocation(gl_program_shader,"flag"), 1);
				glUniform1i( glGetUniformLocation(gl_program_shader, "textura1"), 5);
				glUniform1i( glGetUniformLocation(gl_program_shader, "spotOn"), 0);
				glUniform1f(glGetUniformLocation(gl_program_shader,"h"), (float)SHIP_HEIGHT);
				glUniform1f(glGetUniformLocation(gl_program_shader,"w"), (float)SHIP_WIDTH);				
				glm::mat4 scale_matrix = glm::scale(glm::mat4(1.f), glm::vec3(0.8, 0.8, 0.8));
				glm::mat4 trans_matrix = glm::translate(glm::mat4(1.f), position + forward*(float)SHIP_DISTANCE + glm::vec3(0, -3, 0));
				int sgn = (forward.z > 0) - (forward.z < 0);
				glm::mat4 rot_matrix = glm::rotate(glm::mat4(1.f), 90 - sgn*180/(float)PI*acos(glm::dot(glm::normalize(forward), glm::vec3(1, 0, 0))/sqrt(forward.x*forward.x + forward.y*forward.y + forward.z*forward.z)), glm::vec3(0, 1, 0));
				
				model_matrix = trans_matrix * rot_matrix * scale_matrix;
				glUniformMatrix4fv(glGetUniformLocation(gl_program_shader,"model_matrix"),1,false,glm::value_ptr(model_matrix));
				glBindVertexArray(ship_vao);
				glDrawElements(GL_TRIANGLES, ship_num_indices, GL_UNSIGNED_INT, 0);

				glUniformMatrix4fv(glGetUniformLocation(gl_program_shader,"model_matrix"),1,false,glm::value_ptr(glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)));


				//deseneaza groundul
				glActiveTexture(GL_TEXTURE0+4);
				glBindTexture(GL_TEXTURE_2D, texture_ground);
				glUniform1i( glGetUniformLocation(gl_program_shader, "textura1"),4 );
				glUniform1i( glGetUniformLocation(gl_program_shader, "spotOn"),1 );
				glUniform1f( glGetUniformLocation(gl_program_shader, "angle"), SPOT_ANGLE );
				glUniform3fv( glGetUniformLocation(gl_program_shader, "position"), 1, glm::value_ptr(position + glm::vec3(0, -3, 0)));
				glUniform3fv( glGetUniformLocation(gl_program_shader, "forward"), 1, glm::value_ptr(forward));
				glBindVertexArray(ground_vao);
				glDrawElements(GL_TRIANGLES, ground_num_indices, GL_UNSIGNED_INT, 0);
				glUniform1i(glGetUniformLocation(gl_program_shader,"flag"), 1);

				//deseneaza cladirile
				for (int i = 0; i < scene_objects.size() - 1; i++) {
					if (isInFrustrum(scene_objects[i])) {
						glActiveTexture(GL_TEXTURE0+1);
						glBindTexture(GL_TEXTURE_2D, texture_building[tex_type[i]]);
						glUniform1i( glGetUniformLocation(gl_program_shader, "textura1"), 1);
						glUniform1i( glGetUniformLocation(gl_program_shader, "spotOn"), 1);
						glUniform1f( glGetUniformLocation(gl_program_shader, "angle"), SPOT_ANGLE );
						glUniform3fv( glGetUniformLocation(gl_program_shader, "position"), 1, glm::value_ptr(position + glm::vec3(0, -3, 0)));
						glUniform3fv( glGetUniformLocation(gl_program_shader, "forward"), 1, glm::value_ptr(forward));
						glUniform1f(glGetUniformLocation(gl_program_shader,"h"), scene_objects[i].height);
						glUniform1f(glGetUniformLocation(gl_program_shader,"w"), scene_objects[i].width);
						glBindVertexArray(object_vao[i]);
						glDrawElements(GL_TRIANGLES, object_num_indices[i], GL_UNSIGNED_INT, 0);
					}
				}
			}
			else {
				//deseneaza perspectiva generala
				glUniform1i(glGetUniformLocation(gl_program_shader,"flag"), 0);
				glUniform1i( glGetUniformLocation(gl_program_shader, "spotOn"),0 );
				for (int i = 0; i < scene_objects.size() - 1; i++) {
					if (isInFrustrum(scene_objects[i])) {
						glActiveTexture(GL_TEXTURE0+2);
						glBindTexture(GL_TEXTURE_2D, texture_green);
						glUniform1i( glGetUniformLocation(gl_program_shader, "textura1"),2 );
						glBindVertexArray(object_vao[i]);
						glDrawElements(GL_TRIANGLES, object_num_indices[i], GL_UNSIGNED_INT, 0);
					}
					else {
						glActiveTexture(GL_TEXTURE0+3);
						glBindTexture(GL_TEXTURE_2D, texture_red);
						glUniform1i( glGetUniformLocation(gl_program_shader, "textura1"),3 );
						glBindVertexArray(object_vao[i]);
						glDrawElements(GL_TRIANGLES, object_num_indices[i], GL_UNSIGNED_INT, 0);
					}
				}
			}
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}		
	}

	//functie ce afla ecuatiile planelor de frustrum
	void determinePlanes(glm::vec3 p) {
		glm::vec3 near_center, far_center, near_ul, near_ll, near_ur, near_lr, far_ul, far_ll, far_lr, far_ur;
		near_center = p + glm::normalize(forward)*(float)NEAR_DIST;
		far_center = p + glm::normalize(forward)*(float)FAR_DIST;
		glm::vec3 n, u, v;

		//afla punctele ce formeaza frustrumul
		glm::vec3 right = glm::normalize(glm::cross(up, forward));
		near_ul = near_center + (glm::normalize(up) * ((float)NEAR_HEIGHT/2)) - (right * ((float)NEAR_WIDTH/2));
		near_ur = near_center + (glm::normalize(up) * ((float)NEAR_HEIGHT/2)) + (right * ((float)NEAR_WIDTH/2));
		near_ll = near_center - (glm::normalize(up) * ((float)NEAR_HEIGHT/2)) - (right * ((float)NEAR_WIDTH/2));
		near_lr = near_center - (glm::normalize(up) * ((float)NEAR_HEIGHT/2)) + (right *((float)NEAR_WIDTH/2));
		far_ul = far_center + (glm::normalize(up) * ((float)FAR_HEIGHT/2)) - (right * ((float)FAR_WIDTH/2));
		far_ur = far_center + (glm::normalize(up) * ((float)FAR_HEIGHT/2)) + (right * ((float)FAR_WIDTH/2));
		far_ll = far_center - (glm::normalize(up) * ((float)FAR_HEIGHT/2)) - (right * ((float)FAR_WIDTH/2));
		far_lr = far_center - (glm::normalize(up) * ((float)FAR_HEIGHT/2)) + (right * ((float)FAR_WIDTH/2));

		//aflam coeficientii ecuatiei planului de aproape
		v = near_ul - near_ur;
		u = near_ul - near_ll;
		n = glm::normalize(glm::cross(v,u));
		A1 = n.x;
		B1 = n.y;
		C1 = n.z;
		D1 = glm::dot(-n, near_ul);

		//aflam coeficientii ecuatiei planului de departe
		v = far_ul - far_ur;
		u = far_ul - far_ll;
		n = glm::normalize(glm::cross(u,v));
		A2 = n.x;
		B2 = n.y;
		C2 = n.z;
		D2 = glm::dot(-n, far_ul);

		//aflam coeficientii ecuatiei planului drept
		v = far_ul - far_ll;
		u = far_ul - near_ul;
		n = glm::normalize(glm::cross(u,v));
		A3 = n.x;
		B3 = n.y;
		C3 = n.z;
		D3 = glm::dot(-n, far_ul);

		//aflam coeficientii ecuatiei planului stang
		v = far_ur - far_lr;
		u = far_ur - near_ur;
		n = glm::normalize(glm::cross(v,u));
		A4 = n.x;
		B4 = n.y;
		C4 = n.z;
		D4 = glm::dot(-n, far_ur);

		//aflam coeficientii ecuatiei planului superior
		v = far_ur - far_ul;
		u = far_ur - near_ur;
		n = glm::normalize(glm::cross(u,v));
		A5 = n.x;
		B5 = n.y;
		C5 = n.z;
		D5 = glm::dot(-n, far_ur);

		//aflam coeficientii ecuatiei planului inferior
		v = far_lr - far_ll;
		u = far_lr - near_lr;
		n = glm::normalize(glm::cross(v,u));
		A6 = n.x;
		B6 = n.y;
		C6 = n.z;
		D6 = glm::dot(-n, far_lr);

	}

	//functie ce determina daca un obiect se afla in frustrumul curent al camerei
	bool isInFrustrum(struct SceneObject obj) {
		bool is_valid = false;
		//verificam planul de aproape
		if (A1*obj.position.x + B1*obj.position.y + C1*obj.position.z + D1 < 0) is_valid = true;
		else if (A1*(obj.position.x + obj.length) + B1*obj.position.y + C1*obj.position.z + D1 < 0) is_valid = true;
		else if (A1*obj.position.x + B1*obj.position.y + C1*(obj.position.z + obj.width) + D1 < 0) is_valid = true;
		else if (A1*(obj.position.x +obj.length) + B1*obj.position.y + C1*(obj.position.z + obj.width) + D1 < 0) is_valid = true;
		else if (A1*obj.position.x + B1*(obj.position.y + obj.height) + C1*obj.position.z + D1 < 0) is_valid = true;
		else if (A1*(obj.position.x + obj.length) + B1*(obj.position.y + obj.height) + C1*obj.position.z + D1 < 0) is_valid = true;
		else if (A1*obj.position.x + B1*(obj.position.y + obj.height) + C1*(obj.position.z + obj.width) + D1 < 0) is_valid = true;
		else if (A1*(obj.position.x +obj.length) + B1*(obj.position.y + obj.height) + C1*(obj.position.z + obj.width) + D1 < 0) is_valid = true;

		if (is_valid == false) {
			return false;
		}
		is_valid = false;

		//verificam planul de departe
		if (A2*obj.position.x + B2*obj.position.y + C2*obj.position.z + D2 < 0) is_valid = true;
		else if (A2*(obj.position.x + obj.length) + B2*obj.position.y + C2*obj.position.z + D2 < 0) is_valid = true;
		else if (A2*obj.position.x + B2*obj.position.y + C2*(obj.position.z + obj.width) + D2 < 0) is_valid = true;
		else if (A2*(obj.position.x +obj.length) + B2*obj.position.y + C2*(obj.position.z + obj.width) + D2 < 0) is_valid = true;
		else if (A2*obj.position.x + B2*(obj.position.y + obj.height) + C2*obj.position.z + D2 < 0) is_valid = true;
		else if (A2*(obj.position.x + obj.length) + B2*(obj.position.y + obj.height) + C2*obj.position.z + D2 < 0) is_valid = true;
		else if (A2*obj.position.x + B2*(obj.position.y + obj.height) + C2*(obj.position.z + obj.width) + D2 < 0) is_valid = true;
		else if (A2*(obj.position.x +obj.length) + B2*(obj.position.y + obj.height) + C2*(obj.position.z + obj.width) + D2 < 0) is_valid = true;

		if (is_valid == false) {
			return false;
		}
		is_valid = false;

		//verificam planul drept
		if (A3*obj.position.x + B3*obj.position.y + C3*obj.position.z + D3 < 0) is_valid = true;
		else if (A3*(obj.position.x + obj.length) + B3*obj.position.y + C3*obj.position.z + D3 < 0) is_valid = true;
		else if (A3*obj.position.x + B3*obj.position.y + C3*(obj.position.z + obj.width) + D3 < 0) is_valid = true;
		else if (A3*(obj.position.x +obj.length) + B3*obj.position.y + C3*(obj.position.z + obj.width) + D3 < 0) is_valid = true;
		else if (A3*obj.position.x + B3*(obj.position.y + obj.height) + C3*obj.position.z + D3 < 0) is_valid = true;
		else if (A3*(obj.position.x + obj.length) + B3*(obj.position.y + obj.height) + C3*obj.position.z + D3 < 0) is_valid = true;
		else if (A3*obj.position.x + B3*(obj.position.y + obj.height) + C3*(obj.position.z + obj.width) + D3 < 0) is_valid = true;
		else if (A3*(obj.position.x +obj.length) + B3*(obj.position.y + obj.height) + C3*(obj.position.z + obj.width) + D3 < 0) is_valid = true;

		if (is_valid == false) {
			return false;
		}
		is_valid = false;

		//stang
		if (A4*obj.position.x + B4*obj.position.y + C4*obj.position.z + D4 < 0) is_valid = true;
		else if (A4*(obj.position.x + obj.length) + B4*obj.position.y + C4*obj.position.z + D4 < 0) is_valid = true;
		else if (A4*obj.position.x + B4*obj.position.y + C4*(obj.position.z + obj.width) + D4 < 0) is_valid = true;
		else if (A4*(obj.position.x +obj.length) + B4*obj.position.y + C4*(obj.position.z + obj.width) + D4 < 0) is_valid = true;
		else if (A4*obj.position.x + B4*(obj.position.y + obj.height) + C4*obj.position.z + D4 < 0) is_valid = true;
		else if (A4*(obj.position.x + obj.length) + B4*(obj.position.y + obj.height) + C4*obj.position.z + D4 < 0) is_valid = true;
		else if (A4*obj.position.x + B4*(obj.position.y + obj.height) + C4*(obj.position.z + obj.width) + D4 < 0) is_valid = true;
		else if (A4*(obj.position.x +obj.length) + B4*(obj.position.y + obj.height) + C4*(obj.position.z + obj.width) + D4 < 0) is_valid = true;

		if (is_valid == false) {
			return false;
		}
		is_valid = false;

		//planul superior
		if (A5*obj.position.x + B5*obj.position.y + C5*obj.position.z + D5 < 0) is_valid = true;
		else if (A5*(obj.position.x + obj.length) + B5*obj.position.y + C5*obj.position.z + D5 < 0) is_valid = true;
		else if (A5*obj.position.x + B5*obj.position.y + C5*(obj.position.z + obj.width) + D5 < 0) is_valid = true;
		else if (A5*(obj.position.x +obj.length) + B5*obj.position.y + C5*(obj.position.z + obj.width) + D5 < 0) is_valid = true;
		else if (A5*obj.position.x + B5*(obj.position.y + obj.height) + C5*obj.position.z + D5 < 0) is_valid = true;
		else if (A5*(obj.position.x + obj.length) + B5*(obj.position.y + obj.height) + C5*obj.position.z + D5 < 0) is_valid = true;
		else if (A5*obj.position.x + B5*(obj.position.y + obj.height) + C5*(obj.position.z + obj.width) + D5 < 0) is_valid = true;
		else if (A5*(obj.position.x +obj.length) + B5*(obj.position.y + obj.height) + C5*(obj.position.z + obj.width) + D5 < 0) is_valid = true;

		if (is_valid == false) {
			return false;
		}
		is_valid = false;

		//planul inferior
		if (A6*obj.position.x + B6*obj.position.y + C6*obj.position.z + D6 < 0) is_valid = true;
		else if (A6*(obj.position.x + obj.length) + B6*obj.position.y + C6*obj.position.z + D6 < 0) is_valid = true;
		else if (A6*obj.position.x + B6*obj.position.y + C6*(obj.position.z + obj.width) + D6 < 0) is_valid = true;
		else if (A6*(obj.position.x +obj.length) + B6*obj.position.y + C6*(obj.position.z + obj.width) + D6 < 0) is_valid = true;
		else if (A6*obj.position.x + B6*(obj.position.y + obj.height) + C6*obj.position.z + D6 < 0) is_valid = true;
		else if (A6*(obj.position.x + obj.length) + B6*(obj.position.y + obj.height) + C6*obj.position.z + D6 < 0) is_valid = true;
		else if (A6*obj.position.x + B6*(obj.position.y + obj.height) + C6*(obj.position.z + obj.width) + D6 < 0) is_valid = true;
		else if (A6*(obj.position.x +obj.length) + B6*(obj.position.y + obj.height) + C6*(obj.position.z + obj.width) + D6 < 0) is_valid = true;

		if (is_valid == false) {
			return false;
		}
		return true;
	}

	//functie chemata dupa ce am terminat cadrul de desenare (poate fi folosita pt modelare/simulare)
	void notifyEndFrame(){}
	//functei care e chemata cand se schimba dimensiunea ferestrei initiale
	void notifyReshape(int width, int height, int previos_width, int previous_height){
		//reshape
		if(height==0) height=1;
		screen_width = width;
		screen_height = height;
		float aspect = width/ height;
		projection_matrix = glm::perspective(75.0f, aspect,0.1f, 10000.0f);
	}


	//--------------------------------------------------------------------------------------------
	//functii de input output --------------------------------------------------------------------
	
	//tasta apasata
	void notifyKeyPressed(unsigned char key_pressed, int mouse_x, int mouse_y){
		if(key_pressed == 27) lab::glut::close();	//ESC inchide glut si 
		if(key_pressed == 32) {
			//SPACE reincarca shaderul si recalculeaza locatiile (offseti/pointeri)
			glDeleteProgram(gl_program_shader);
			gl_program_shader = lab::loadShader("shadere\\shader_vertex.glsl", "shadere\\shader_fragment.glsl");
		}
		if(key_pressed == 'i'){
			static bool wire =true;
			wire=!wire;
			glPolygonMode(GL_FRONT_AND_BACK, (wire?GL_LINE:GL_FILL));
		}
		// TODO: implementati functiile de mai jos in lab_camera.hpp
		if(key_pressed == 'w') { cam.translateForward(10.0f); }
		if(key_pressed == 'a') { cam.translateRight(-10.0f); }
		if(key_pressed == 's') { cam.translateForward(-10.0f); }
		if(key_pressed == 'd') { cam.translateRight(10.0f); }
		if(key_pressed == 'r') { cam.translateUpword(10.0f); }
		if(key_pressed == 'f') { cam.translateUpword(-10.0f); }
		if(key_pressed == 'q') { cam.rotateFPSoY(1.0f); }
		if(key_pressed == 'e') { cam.rotateFPSoY(-1.0f); }
		if(key_pressed == 'z') { cam.rotateFPSoZ(-1.0f); }
		if(key_pressed == 'c') { cam.rotateFPSoZ(1.0f); }
		if(key_pressed == 't') { cam.rotateFPSoX(1.0f); }
		if(key_pressed == 'g') { cam.rotateFPSoX(-1.0f); }
		
		if(key_pressed == '\t') {
			if (tps) {
				position = cam.getPosition();
				forward  = cam.getForward();
				up = cam.getUp();
				cam.set(overviewEye, glm::vec3(MAP_SIZE/2, 0, MAP_SIZE/2), overviewEye + glm::vec3(0,1,0));
				tps = false;
			}
			else {
				tps = true;
				cam.set2(position, forward, up);
			}			
		}
	}
	//tasta ridicata
	void notifyKeyReleased(unsigned char key_released, int mouse_x, int mouse_y){	}
	//tasta speciala (up/down/F1/F2..) apasata
	void notifySpecialKeyPressed(int key_pressed, int mouse_x, int mouse_y){
		if(key_pressed == GLUT_KEY_F1) lab::glut::enterFullscreen();
		if(key_pressed == GLUT_KEY_F2) lab::glut::exitFullscreen();
	}
	//tasta speciala ridicata
	void notifySpecialKeyReleased(int key_released, int mouse_x, int mouse_y){}
	//drag cu mouse-ul
	void notifyMouseDrag(int mouse_x, int mouse_y){ }
	//am miscat mouseul (fara sa apas vreun buton)
	void notifyMouseMove(int mouse_x, int mouse_y){ }
	//am apasat pe un boton
	void notifyMouseClick(int button, int state, int mouse_x, int mouse_y){ }
	//scroll cu mouse-ul
	void notifyMouseScroll(int wheel, int direction, int mouse_x, int mouse_y){ std::cout<<"Mouse scroll"<<std::endl;}
};

int main(){
	//initializeaza GLUT (fereastra + input + context OpenGL)
	lab::glut::WindowInfo window(std::string("lab shadere 3 - camera"),800,600,100,100,true);
	lab::glut::ContextInfo context(3,3,false);
	lab::glut::FramebufferInfo framebuffer(true,true,true,true);
	lab::glut::init(window,context, framebuffer);

	//initializeaza GLEW (ne incarca functiile openGL, altfel ar trebui sa facem asta manual!)
	glewExperimental = true;
	glewInit();
	std::cout<<"GLEW:initializare"<<std::endl;

	//creem clasa noastra si o punem sa asculte evenimentele de la GLUT
	//DUPA GLEW!!! ca sa avem functiile de OpenGL incarcate inainte sa ii fie apelat constructorul (care creeaza obiecte OpenGL)
	Laborator mylab;
	lab::glut::setListener(&mylab);

	//taste
	std::cout<<"Taste:"<<std::endl<<"\tESC ... iesire"<<std::endl<<"\tSPACE ... reincarca shadere"<<std::endl<<"\ti ... toggle wireframe"<<std::endl<<"\to ... reseteaza camera"<<std::endl;
	std::cout<<"\tw ... translatie camera in fata"<<std::endl<<"\ts ... translatie camera inapoi"<<std::endl;
	std::cout<<"\ta ... translatie camera in stanga"<<std::endl<<"\td ... translatie camera in dreapta"<<std::endl;
	std::cout<<"\tr ... translatie camera in sus"<<std::endl<<"\tf ... translatie camera jos"<<std::endl;
	std::cout<<"\tq ... rotatie camera FPS pe Oy, minus"<<std::endl<<"\te ... rotatie camera FPS pe Oy, plus"<<std::endl;
	std::cout<<"\tz ... rotatie camera FPS pe Oz, minus"<<std::endl<<"\tc ... rotatie camera FPS pe Oz, plus"<<std::endl;

	//run
	lab::glut::run();

	return 0;
}