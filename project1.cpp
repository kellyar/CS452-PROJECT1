#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <vector>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "shader.h"
#include <stdlib.h>
#include <iostream> 


#define NUM_VERTICES num_vertices
#define NUM_INDICES num_indices	

#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#define ARRAY_COUNT( array ) (sizeof( array ) / (sizeof( array[0] ) * (sizeof( array ) != sizeof(void*) || sizeof( array[0] ) <= sizeof(void*))))

//MACRO For Collision detection fix
#define epsilon 20
//#define epsilon 0.0001

void keyboard( unsigned char key, int x, int y );

GLuint shaderProgramID;
GLuint squareVao;
GLuint platformVao;
GLuint vbo;
GLuint positionID;
GLuint colorID;
GLuint indexBufferID; 		

float horizontal = 0.0f;
float vertical = -0.0f;
float thetaY = 0.00f;
float thetaX = 0.00f;
float scaleAmount = 1.0f;
const int numLevels = 50;
const int num_vertices = 4 + 8 * numLevels; //number of lines  each bar adds 8
const int num_indices = 6 + 12 * numLevels; //num elements   each bar adds 12
float barVertical = 0.0f;
float barDelt=0.002;
int score=0;

GLfloat vertices[num_vertices * 3]; //num_verticies * 3
GLfloat color[num_vertices * 4]; //num_verticies * 3
GLuint indices[num_indices];


bool collision(float horizontal, float vertical, char key);

void genVertextData(){

	vertices[0] = -0.035f; vertices[1] = -0.035f; vertices[2] = 0.00f;
	
	vertices[3] = 0.035f; vertices[4] = -0.035f;	vertices[5] = 0.00f;
	
	vertices[6] = 0.035f; vertices[7] = 0.035f; vertices[8] = 0.00f;
	
	vertices[9] = -0.035f; vertices[10] = 0.035f; vertices[11] = 0.00f;

	

	float barhlx;
	float bartop = -0.15f;
	float barhrx;
	float barbottom;
	float border = 1.0f;

	int n = 1;

	for(int i=0; i<ARRAY_COUNT(color); i++){
		if(i < 15){
			color[i] = 1.0f;
		}else{
			if(n==1)
				color[i] = 1.0f;
			if(n==2)
				color[i] = 0.0f;
			if(n==3)
				color[i] = 0.0f;
			if(n==4){
				color[i] = 0.0f;
				n=0;
			}
			n++;
		}
		
	}

	for(int i=12; i<ARRAY_COUNT(vertices); i+=24){


		float barhlx = -1.0 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(2.0)));
		float barbottom = bartop - 0.1f;
		float barhrx = barhlx + 0.2f;
		

		vertices[i]= -border; vertices[i+1] = barbottom; vertices[i+2] = 0.0f;
		
		vertices[i+3] = barhlx; vertices[i+4] = barbottom; vertices[i+5] = 0.0f;
		
		vertices[i+6] = barhlx; vertices[i+7] = bartop; vertices[i+8] = 0.0f;
		
		vertices[i+9]= -border; vertices[i+10] = bartop; vertices[i+11] = 0.0f;
		
		vertices[i+12] = barhrx; vertices[i+13] = barbottom; vertices[i+14] = 0.0f;

		vertices[i+15]= border; vertices[i+16] = barbottom; vertices[i+17] = 0.0f;

		vertices[i+18]= border; vertices[i+19] = bartop; vertices[i+20] = 0.0f;

		vertices[i+21] = barhrx; vertices[i+22] = bartop; vertices[i+23] = 0.0f;
		
		bartop -= 0.3f;

	}

	
	n=0;
	for(int i=0; i<ARRAY_COUNT(indices); i+=6){
		
		indices[i] = n;
		indices[i+1] = n+1;
		indices[i+2] = n+2;
		indices[i+3] = n;
		indices[i+4] = n+2;
		indices[i+5] = n+3;
		n+=4;
	}
}



void AdjustVertexData(float fXOffset, float fYOffset, float barV)
{
    std::vector<float> fNewData(ARRAY_COUNT(vertices));
    memcpy(&fNewData[0], vertices, sizeof(vertices));
   
    for(int i = 0; i < ARRAY_COUNT(vertices); i += 3)
    {
    	if(i<12){
    		fNewData[i] += fXOffset;
        	fNewData[i + 1] += fYOffset;
    	}else{
    		fNewData[i + 1] += barV;
    	}
        
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &fNewData[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void changeViewport(int w, int h){
	glViewport(0, 0, w, h);
}



// Here is the function that gets called each time the window needs to be redrawn.
// It is the "paint" method for our program, and is set up from the glutDisplayFunc in main
void render() {

	score += 1;

	if(score % 1000 == 0){
		barDelt += 0.0007f;
		fprintf(stderr, "Think you're doing good?.... I'll speed up!!\n");
	}

	barVertical += barDelt;
	if(collision(horizontal, vertical, 's')){
		vertical += barDelt;
		if(vertical >= 1.0){
			fprintf(stderr, "The block hit the top, you lost!!\n");
			fprintf(stderr, "Your score was: %d\n", score);
			exit( EXIT_SUCCESS );
		}
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glUseProgram(shaderProgramID);

	AdjustVertexData(horizontal, vertical, barVertical);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_QUADS);
	glDrawElements (GL_TRIANGLES, NUM_INDICES, GL_UNSIGNED_INT, NULL);
	glutSwapBuffers();
	glutPostRedisplay();
}

int main (int argc, char** argv) {
	
	fprintf(stderr, "Navigate to the bottom of the screen to win.\nIf you hit the top you lose!\n");
	fprintf(stderr, "Starting in 3.... 2.... 1....\n");
	sleep(1);

	genVertextData();


	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Index Buffers");
	glutReshapeFunc(changeViewport);
	glutDisplayFunc(render);
	glewInit();

	// Make a shader
	shaderProgramID = createShaders();

	glGenVertexArrays(1, &squareVao);
	glBindVertexArray(squareVao);

	glGenVertexArrays(1, &platformVao);
	glBindVertexArray(platformVao);

/*
---------------------------------------------------------------------------
verticies	
---------------------------------------------------------------------------
*/
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// Create the buffer, but don't load anything yet
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(color), NULL, GL_STATIC_DRAW);
	// Load the vertex points
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
	// Load the colors right after that
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices),sizeof(color), color);
/*
---------------------------------------------------------------------------
indices
---------------------------------------------------------------------------
*/	
	glGenBuffers(1, &indexBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	positionID = glGetAttribLocation(shaderProgramID, "s_vPosition");
	colorID = glGetAttribLocation(shaderProgramID, "s_vColor");


	glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(colorID, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vertices)));
	
	
	glutKeyboardFunc(keyboard);

	glUseProgram(shaderProgramID);
	glEnableVertexAttribArray(positionID);
	glEnableVertexAttribArray(colorID);
	glutMainLoop();
	
	return 0;
}

bool collision(float horizontal, float vertical, char key){

	int plat_1x, plat_1y, plat_2x, plat_2y, plat_4x;


	/*update the variables based on key press*/
	int box_0x=0;//sw corner
	int box_0y=1;
	int box_1x=3;//se corner
	int box_2y=7;
	
	float box_top=vertices[box_2y]+vertical;//top of box
	float box_bot=vertices[box_0y]+vertical;//bottom of box
	float box_left=vertices[box_0x]+horizontal;//left side coord of box
	float box_right=vertices[box_1x]+horizontal;//right side coord of box
	float box_x=vertices[box_0x]+horizontal;//lower left coord
	float box_y=vertices[box_0y]+vertical;//lower left coord
	float box_x_1=vertices[box_1x]+horizontal;//lower right coord
	float box_x_0=vertices[box_0x]+horizontal;//lower left coord (model coord+ofset)

	plat_1x=15;//left se
	plat_1y=16;
	plat_2x=18;//left ne
	plat_2y=19;
	plat_4x=24;//right sw

	float plat_top;
	float plat_bot;
	float plat_left_hole;
	float plat_right_hole;


	for(int n = 0; n<numLevels; n++){

		plat_top=vertices[plat_2y] + barVertical;//top of platform
		plat_bot=vertices[plat_1y] + barVertical;//bottom of platform
		plat_left_hole=vertices[plat_1x];//left side of hole
		plat_right_hole=vertices[plat_4x];//right side of hole

		if( key == 'a' &&
			( ( -epsilon <= (box_x_0 - vertices[plat_1x])) && 
		( (box_x_0 - vertices[plat_1x]) <= epsilon ) ) && 
		( box_y >= vertices[plat_1y] && box_y < vertices[plat_2y] ) ){//be able to still move if above platform
			
			return false;
		}




		if(key == 'd' && ( ( -epsilon <= (box_x_1 - vertices[plat_4x])) && ( (box_x_1 - vertices[plat_4x]) <= epsilon ) ) && ( box_y >= vertices[plat_1y] && box_y < vertices[plat_2y] ) ){//IT NOW WORKS!!
			return false;
		}
	
		if(key == 's'){
			if ((box_left >= plat_left_hole) and (box_right <= plat_right_hole)){//ARE WE IN A HOLE?
				
			}//If I'm in the hole I should always be able to move down so CHECK ME FIRST!
			
			else if (box_top - 0.1 > (plat_top)){//Are we above the platform? //This used to say //if (box_bot >= (plat_top+0.1))//half the fall dist?//Take where I am, if I move down, do I go under the platform?
				
			}
			else{//we must be below the top of the platform --either in a hole or beneath the platform itself
				//printf("\nI'm not above a platform!\nBOX_BOT=%f != PLAT TOP=%f\n",box_bot , plat_top);
				if (box_bot <= plat_top){//Are we beneath the top of a platform? --true if above is false
				//ARE WE IN A HOLE?
					if ((box_left >= plat_left_hole) and (box_right <= plat_right_hole)){//ARE WE IN A HOLE?
						//printf("\nI'm in a hole!\nSo I moved Down!\n");
						
					}
					else{//so I'm beneath or on the surface but not in a hole? (Check if beneath and allow)
						//printf("\n...I'm not in a hole!\n");
						if (box_bot <= plat_bot){//Am I under a platform?
							
						}
						else{
							return false;
						}
					}
				}
			}
		}

		plat_1x += 24;
		plat_1y += 24;
		plat_2x += 24;
		plat_2y += 24;
		plat_4x += 24;
	}

	return true;
}


void keyboard( unsigned char key, int x, int y ){
    
    switch( key ) {//ERROR : HOLDING A KEY BREAKS COLLIOSION DETECTION
    	case 033:
    	fprintf(stderr, "Gave up already?\n");
   		fprintf(stderr, "Your score was: %d\n", score);
	    exit( EXIT_SUCCESS );
	    break;
	case 'a'://left -- WORKS!

		if(collision(horizontal - 0.05, vertical, key)){
			horizontal -= 0.05;
		}

		break;
	case 'd'://right box1, plat4
		if(collision(horizontal + 0.05, vertical, key)){
			horizontal += 0.05;
		}	
		break;

	case 'w'://up...//JUST DISABLE UP!
	
		
		vertical += 0.05;	
		break;

	case 's'://down
	
		if(collision(horizontal, vertical - 0.05, key)){
			vertical -= 0.05;
		}	

		if(vertical < -1){
			fprintf(stderr, "You made it to the bottom.  Congrats you win!\n");
			fprintf(stderr, "Your score was: %d\n", score);
			exit( EXIT_SUCCESS );
		}

		break;

    }

    

}
