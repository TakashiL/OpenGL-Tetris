// Tetris - CMPT 361 Assignment 1
// Student Name: Ziyue Lu
// Student ID: 301295780

#include "Angel.h"

static vec3 base_colors[] = {
	// All colors for tiles
	vec3( 1.0, 0.7, 0.0 ),
	vec3( 0.4, 0.7, 1.0 ),
	vec3( 1.0, 0.6, 0.6 ),
	vec3( 0.6, 0.2, 1.0 ),
	vec3( 1.0, 0.4, 0.7 )
};

static vec3 white  = vec3(1.0, 1.0, 1.0);
static vec3 black  = vec3(0.0, 0.0, 0.0);

const int NumGridPoints = 64; // totally 32 lines to draw, 4 points are duplicated
const int NumBoardPoints = 1200; // totally 400 triangles to draw

float gameSpeed = 600.0; // speed of tile failing

static vec2 allTilesShape[7][4][4] = {
	// All types and orientations of tile
	// O
  {{vec2(0,-1), vec2(0,0), vec2(-1,0), vec2(-1,-1)},
	{vec2(0,-1), vec2(0,0), vec2(-1,0), vec2(-1,-1)},
	{vec2(0,-1), vec2(0,0), vec2(-1,0), vec2(-1,-1)},
	{vec2(0,-1), vec2(0,0), vec2(-1,0), vec2(-1,-1)}},

	// I
	{{vec2(0,-2), vec2(0,-1), vec2(0,0), vec2(0,1)},
	{vec2(-2,0), vec2(-1,0), vec2(0,0), vec2(1,0)},
	{vec2(0,-1), vec2(0,0), vec2(0,1), vec2(0,2)},
	{vec2(-1,0), vec2(0,0), vec2(1,0), vec2(2,0)}},

	// S
	{{vec2(-1,-1), vec2(-1,0), vec2(0,0), vec2(0,1)},
	{vec2(-1,1), vec2(0,1), vec2(0,0), vec2(1,0)},
	{vec2(0,-1), vec2(0,0), vec2(1,0), vec2(1,1)},
	{vec2(1,-1), vec2(0,-1), vec2(0,0), vec2(-1,0)}},

	// Z
	{{vec2(0,-1), vec2(0,0), vec2(-1,0), vec2(-1,1)},
	{vec2(-1,0), vec2(0,0), vec2(0,1), vec2(1,1)},
	{vec2(1,-1), vec2(1,0), vec2(0,0), vec2(0,1)},
	{vec2(-1,-1), vec2(0,-1), vec2(0,0), vec2(1,0)}},

	// L
	{{vec2(-1,-1), vec2(0,-1), vec2(0,0), vec2(0,1)},
	{vec2(-1,1), vec2(-1,0), vec2(0,0), vec2(1,0)},
	{vec2(1,1), vec2(0,1), vec2(0,0), vec2(0,-1)},
	{vec2(1,-1), vec2(1,0), vec2(0,0), vec2(-1,0)}},

	// J
	{{vec2(0,-1), vec2(0,0), vec2(0,1), vec2(-1,1)},
	{vec2(-1,0), vec2(0,0), vec2(1,0), vec2(1,1)},
	{vec2(0,1), vec2(0,0), vec2(0,-1), vec2(1,-1)},
	{vec2(1,0), vec2(0,0), vec2(-1,0), vec2(-1,-1)}},

	// T
	{{vec2(0,-1), vec2(0,0), vec2(0,1), vec2(-1,0)},
	{vec2(1,0), vec2(0,0), vec2(-1,0), vec2(0,1)},
	{vec2(0,-1), vec2(0,0), vec2(0,1), vec2(1,0)},
	{vec2(0,-1), vec2(1,0), vec2(0,0), vec2(-1,0)}}
};

// global variables for current tile
int currentTileGrid[5][5]; 	// Store current tile in a 5*5 grid
int currentTileGridOffsetRow;
int currentTileGridOffsetCol;
int currentTileType;
int currentTileOrientation;
int currentTileColor;

// the game board
int board[20][10];

vec2 gridPoints[NumGridPoints];
vec3 gridColors[NumGridPoints];

vec2 boardPoints[NumBoardPoints];
vec3 boardColors[NumBoardPoints];

// location of vertex attributes in the shader program
GLuint vPosition;
GLuint vColor;

// VectorArrayObject and VectorBufferObject
GLuint vaos[2]; // For grid and board
GLuint vbos[2]; // Position and color for each vao

//----------------------------------------------------------------------------

void init();
void initGrid();
void initBoard();
void initCurrentTile();

void currentTileFailling(int);
void updateBoardColor();

void moveDown();
void rotate();
void moveCol(int offsetCol);
void checkRowCompletion();
void removeRowFromBoard(int row);

vec2 currentTileCellRealPosition(int currentTileCellRow, int currentTileCellCol);
void pasteCurrentTileToBoard();
void removeCurrentTileFromBoard();
bool has_collision(int girdOffsetRow, int gridOffsetCol, int testGrid[5][5]);
bool tile_showup();
bool firstRowDie();

void display();
void keyboard(unsigned char key, int x, int y);
void keyboardSpecial(int key, int x, int y);

//----------------------------------------------------------------------------

void init() {
  srand(time(NULL)); //set rand seed

  // Load shaders and use the resulting shader program
  GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );
  glUseProgram(program);

  // Initialize the vertex position attribute from the vertex shader
  vPosition = glGetAttribLocation( program, "vPosition" );
  vColor = glGetAttribLocation( program, "vColor" );

	initGrid();
	initBoard();
	initCurrentTile();

  glClearColor( black[0], black[1], black[2], 1.0 ); // black background
}

//----------------------------------------------------------------------------

void initGrid() {
  // Horizontal lines
	for(int i = 0; i < 21; i++){
    float offset = 1.9 / 20 * i;
		gridPoints[2*i] = vec2(-0.95, 0.95 - offset);
		gridPoints[2*i + 1] = vec2(0.95, 0.95 - offset);
	}

	// Vertical lines
	for(int i = 0; i < 11; i++){
    float offset = 1.9 / 10 * i;
		gridPoints[42 + 2*i] = vec2(-0.95 + offset, 0.95);
		gridPoints[42 + 2*i + 1] = vec2(-0.95 + offset, -0.95);
	}

	// Make all grid lines white
	for(int i = 0; i < 64; i++)
		gridColors[i] = white;

  // Create and bind a vertex array object
  glGenVertexArrays( 1, &vaos[0] );
  glBindVertexArray( vaos[0] );

  // Create and initialize a buffer object
  glGenBuffers( 1, &vbos[0] );
  glBindBuffer( GL_ARRAY_BUFFER, vbos[0] );

  glBufferData( GL_ARRAY_BUFFER, sizeof(gridPoints) + sizeof(gridColors), gridPoints, GL_STATIC_DRAW );

  glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(gridPoints), gridPoints );
  glBufferSubData( GL_ARRAY_BUFFER, sizeof(gridPoints), sizeof(gridColors), gridColors );

  glEnableVertexAttribArray( vPosition );
  glVertexAttribPointer( vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

  glEnableVertexAttribArray( vColor );
  glVertexAttribPointer( vColor, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(gridPoints)) );
}

//----------------------------------------------------------------------------

void initBoard() {
	for(int i=0; i<20; i++){
		for(int j=0; j<10; j++){
			board[i][j] = -1; // initially all points are black
		}
	}
	for(int i=0; i<1200; i++){
		boardColors[i] = black; // initially all points are black
	}
	for(int i=0; i<20; i++){
		for(int j=0; j<10; j++){
			vec2 leftUp = vec2(-0.95 + j * 1.9 / 10, 0.95 - i * 1.9 / 20);
			vec2 leftbottom = vec2(-0.95 + j * 1.9 / 10, 0.95 - (i+1) * 1.9 / 20);
			vec2 rightUp = vec2(-0.95 + (j+1) * 1.9 / 10, 0.95 - i * 1.9 / 20);
			vec2 rightbottom = vec2(-0.95 + (j+1) * 1.9 / 10, 0.95 - (i+1) * 1.9 / 20);

			boardPoints[6 * (10*i + j)    ] = leftUp;
			boardPoints[6 * (10*i + j) + 1] = leftbottom;
			boardPoints[6 * (10*i + j) + 2] = rightbottom;
			boardPoints[6 * (10*i + j) + 3] = leftUp;
			boardPoints[6 * (10*i + j) + 4] = rightUp;
			boardPoints[6 * (10*i + j) + 5] = rightbottom;
		}
	}
}

//----------------------------------------------------------------------------

void initCurrentTile(){
	for(int i=0; i<5; i++){
		for(int j=0; j<5; j++){
			currentTileGrid[i][j] = -1; // initially all cells are black
		}
	}
	// offset for left-bottom cell of the 5*5 grid
	currentTileGridOffsetRow = 0;
	currentTileGridOffsetCol = rand() % 6; // random starting position
	currentTileType = rand() % 7; // random type
	currentTileOrientation = rand() % 4; // random orientation
	currentTileColor = rand() % 5; // random color

	// paste random tile to currentTileGrid[5][5]
	vec2 newTile[4] = allTilesShape[currentTileType][currentTileOrientation];
	for(int i=0; i<4; i++){
		vec2 cell = newTile[i];
		currentTileGrid[2-int(cell.x)][int(cell.y)+2] = currentTileColor;
	}
}

//----------------------------------------------------------------------------

void currentTileFailling(int){
	// this function will be recalled every gameSpeed time
	moveDown();
	glutTimerFunc(gameSpeed, currentTileFailling, 0);
}

//----------------------------------------------------------------------------

void updateBoardColor() {
	// update boardColors[] according to board[][]
	for(int i=0; i<20; i++){
		for(int j=0; j<10; j++){
			int color = board[i][j];
			if(color == -1){
				//black
				boardColors[6 * (10*i + j)    ] = black;
				boardColors[6 * (10*i + j) + 1] = black;
				boardColors[6 * (10*i + j) + 2] = black;
				boardColors[6 * (10*i + j) + 3] = black;
				boardColors[6 * (10*i + j) + 4] = black;
				boardColors[6 * (10*i + j) + 5] = black;
			} else{
				boardColors[6 * (10*i + j)    ] = base_colors[color];
				boardColors[6 * (10*i + j) + 1] = base_colors[color];
				boardColors[6 * (10*i + j) + 2] = base_colors[color];
				boardColors[6 * (10*i + j) + 3] = base_colors[color];
				boardColors[6 * (10*i + j) + 4] = base_colors[color];
				boardColors[6 * (10*i + j) + 5] = base_colors[color];
			}
		}
	}

	// only update canvas here
	glutPostRedisplay();
}

//----------------------------------------------------------------------------

void moveDown(){
	removeCurrentTileFromBoard();
	int nextTileGridOffsetRow = currentTileGridOffsetRow + 1;

	// check validation
	if(has_collision(nextTileGridOffsetRow, currentTileGridOffsetCol, currentTileGrid)){
		// current tile needs to be stacked, check if game is over first
		if(!tile_showup()){
			// game over
			printf("Game Over!\n");
			exit( EXIT_SUCCESS );
		}
		// game is not over, stack current tile
		pasteCurrentTileToBoard();
		updateBoardColor();
		// check if any rows need to be removed
		checkRowCompletion();
		if(firstRowDie()){
			// game over since the first row has tile
			printf("Game Over!\n");
			exit( EXIT_SUCCESS );
		}
		// game is not over, init a new tile
		initCurrentTile();
	} else{
		// allow moving down
		currentTileGridOffsetRow = nextTileGridOffsetRow;
		pasteCurrentTileToBoard();
		updateBoardColor();
	}
}

void rotate(){
	// will reject rotating the tile if the rotation will cause collision
	removeCurrentTileFromBoard();

	// find tile after rotation
	int rotateTileGrid[5][5];
	for(int i=0; i<5; i++){
		for(int j=0; j<5; j++){
			rotateTileGrid[i][j] = -1;
		}
	}

	int rotateTileOrientation = (currentTileOrientation + 1) % 4;
	vec2 rotateTile[4] = allTilesShape[currentTileType][rotateTileOrientation];
	for(int i=0; i<4; i++){
		vec2 cell = rotateTile[i];
		rotateTileGrid[2-int(cell.x)][int(cell.y)+2] = currentTileColor;
	}

	// check validation
	if(!has_collision(currentTileGridOffsetRow, currentTileGridOffsetCol, rotateTileGrid)){
		// allow rotating
		currentTileOrientation = rotateTileOrientation;
		for(int i=0; i<5; i++){
			for(int j=0; j<5; j++){
				currentTileGrid[i][j] = rotateTileGrid[i][j];
			}
		}
		pasteCurrentTileToBoard();
		updateBoardColor();
	} else {
		// no need for updating canvas here because we reject the rotating
		pasteCurrentTileToBoard();
	}
}

void moveCol(int offsetCol){
	// offsetCol: -1 means move left, 1 means move right
	removeCurrentTileFromBoard();

	// check validation
	int movedTileGridOffsetCol = currentTileGridOffsetCol + offsetCol;
	if(!has_collision(currentTileGridOffsetRow, movedTileGridOffsetCol, currentTileGrid)){
		// allow rotating
		currentTileGridOffsetCol = movedTileGridOffsetCol;
		pasteCurrentTileToBoard();
		updateBoardColor();
	} else {
		pasteCurrentTileToBoard();
	}
}

//----------------------------------------------------------------------------

void checkRowCompletion(){
	for(int i=0; i<20; i++){
		bool rowFinish = true;
		for(int j=0; j<10; j++){
			if(board[i][j] == -1){
				rowFinish = false;
			}
		}
		if(rowFinish == true){
			removeRowFromBoard(i);
		}
	}
}

void removeRowFromBoard(int row){
	for(int i=row; i>0; i--){
		for(int j=0; j<10; j++){
			board[i][j] = board[i-1][j];
		}
	}
	for(int j=0; j<10; j++){
		board[0][j] = -1;
	}
}

//----------------------------------------------------------------------------

vec2 currentTileCellRealPosition(int currentTileCellRow, int currentTileCellCol){
	int realRow = currentTileGridOffsetRow + currentTileCellRow -4;
	int realCol = currentTileGridOffsetCol + currentTileCellCol;
	return vec2(realRow, realCol);
}

void pasteCurrentTileToBoard(){
	for(int i=0; i<5; i++){
		for(int j=0; j<5; j++){
			if(currentTileGrid[i][j] != -1){
				vec2 realPosition = currentTileCellRealPosition(i, j);
				int realRow = int(realPosition.x);
				int realCol = int(realPosition.y);
				if(realRow>=0 && realRow<=19 && realCol>=0 && realCol<=9){
					board[realRow][realCol] = currentTileGrid[i][j];
				}
			}
		}
	}
}

void removeCurrentTileFromBoard(){
	for(int i=0; i<5; i++){
		for(int j=0; j<5; j++){
			if(currentTileGrid[i][j] != -1){
				vec2 realPosition = currentTileCellRealPosition(i, j);
				int realRow = int(realPosition.x);
				int realCol = int(realPosition.y);
				if(realRow>=0 && realRow<=19 && realCol>=0 && realCol<=9){
					board[realRow][realCol] = -1;
				}
			}
		}
	}
}

bool has_collision(int girdOffsetRow, int gridOffsetCol, int testGrid[5][5]){
	// return true if the tested tile has collision with board
	for(int i=0; i<5; i++){
		for(int j=0; j<5; j++){
			if(testGrid[i][j] != -1){
				int realRow = girdOffsetRow + i - 4;
				int realCol = gridOffsetCol + j;
				if(realRow >= 0){
					if(realRow > 19 || realCol < 0 || realCol > 9){
						// has collision with game window
						return true;
					} else if(board[realRow][realCol] != -1){
						// has collision with current board
						return true;
					}
				} else{
					// for cell that haven't shown
					if( realCol < 0 || realCol > 9){
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool tile_showup(){
	// return ture if the tile has totally showed up
	for(int i=0; i<5; i++){
		for(int j=0; j<5; j++){
			if(currentTileGrid[i][j] != -1){
				vec2 realPosition = currentTileCellRealPosition(i, j);
				int realRow = int(realPosition.x);
				int realCol = int(realPosition.y);
				if(realRow < 0){
					return false;
				}
			}
		}
	}
	return true;
}

bool firstRowDie(){
	// return true if the first row of board has tile
	for(int j=0; j<10; j++){
		if(board[0][j] != -1){
			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------------

void display() {
	// rebind the board data here so it can be updated every time
	glGenVertexArrays( 1, &vaos[1] );
	glBindVertexArray( vaos[1] );

	glGenBuffers( 1, &vbos[1] );
	glBindBuffer( GL_ARRAY_BUFFER, vbos[1] );

	glBufferData( GL_ARRAY_BUFFER, sizeof(boardPoints) + sizeof(boardColors), boardPoints, GL_STATIC_DRAW );

	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(boardPoints), boardPoints );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(boardPoints), sizeof(boardColors), boardColors );

	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

	glEnableVertexAttribArray( vColor );
	glVertexAttribPointer( vColor, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(boardPoints)) );

  // clear the window
  glClear( GL_COLOR_BUFFER_BIT );

	// draw the board
	glBindVertexArray( vaos[1] );
  glDrawArrays( GL_TRIANGLES, 0, NumBoardPoints );

  // draw the grids after the board so it looks better
	glBindVertexArray( vaos[0] );
	glDrawArrays( GL_LINES, 0, NumGridPoints );

  glFlush();
}

//----------------------------------------------------------------------------

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
  case 033:
		printf("Esc pressed!\n");
    exit( EXIT_SUCCESS );
    break;
	case 113:
		// 'q' for exit
		printf("Q pressed! Quit game!\n");
		exit( EXIT_SUCCESS );
		break;
	case 114:
		// 'r' for restart
		printf("R pressed! Restart!\n");
		init();
		break;
  }
}

void keyboardSpecial(int key, int x, int y) {
	switch (key) {
  case GLUT_KEY_DOWN:
  	printf("down arrow pressed!\n");
    moveDown();
    break;
	case GLUT_KEY_UP:
	  printf("up arrow pressed!\n");
	 	rotate();
	  break;
	case GLUT_KEY_LEFT:
		printf("left arrow pressed!\n");
		moveCol(-1);
		break;
	case GLUT_KEY_RIGHT:
		printf("right arrow pressed!\n");
		moveCol(1);
		break;
	}
}

//----------------------------------------------------------------------------

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA);
    glutInitWindowSize(300, 600);

    // If you are using freeglut, the next two lines will check if the code is truly 3.2. Otherwise, comment them out
    glutInitContextVersion(3, 2);
    glutInitContextProfile(GLUT_CORE_PROFILE);

    glutCreateWindow("Tetris!");

    // If you get a segmentation error at line 34, please uncomment the line below
    glewExperimental = GL_TRUE;
    glewInit();

    init();

    glutDisplayFunc(display);
		glutTimerFunc(gameSpeed, currentTileFailling, 0);
    glutKeyboardFunc(keyboard);
		glutSpecialFunc(keyboardSpecial);

    glutMainLoop();
    return 0;
}
