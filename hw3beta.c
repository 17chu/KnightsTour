#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

//**************GLOBAL VARIABLES******************
long max_squares = 0;
int arg3 = -1;
char*** dead_end_boards;
int dead_size;
int dead_i;
pthread_mutex_t lock;

void* threadSolve(void* arg);

struct solverArgs {
	char** _board;
	int _x;
	int _y;
	int _m;
	int _n;
	
};

char ** copy(char** _board, int _m, int _n){
	char ** copyboard = calloc(_m , sizeof(char*));
		for (int i  = 0; i< _m; i++){
			copyboard[i] = calloc(_n,sizeof(char));
		}
		for (int i = 0; i<_m; i++){
			for (int j = 0; j<_n; j++){
				copyboard[i][j] = _board[i][j];
			}
		}
		return copyboard;
}

int tourCompleted(char** _board,int _m, int _n){//scans through board to find '.'
	for ( int i = 0; i<_m; i++ ){
		for ( int j = 0; j<_n; j++){
			if (_board[i][j] == '.'){
				return 0;
			}
		}
	}
	return 1;
	//if you find no periods board is completed
}	

int numMovesUsed(char** _board,int _m, int _n){
	int moves = 0;
	for ( int i = 0; i<_m; i++ ){
		for ( int j = 0; j<_n; j++){
			if (_board[i][j] == 'S'){
				moves+=1;
			}
		}
	}
	return moves;
}



void freeBoard(char** _cache, int _m){
	for ( int i = 0; i<_m; i++ ){
		free(_cache[i]);
	}
		free(_cache);
}
void printBoard(char** _cache, int _m, int _n){
	for ( int i = 0; i<_m; i++ ){
		for ( int j = 0; j<_n; j++){
				printf("(%d,%d): %c ",i,j,_cache[i][j]);
		}
		printf("\n");
	}
}

int numLegalMoves(char**_board,int _x, int _y, int _m, int _n){
	int counter = 0;
	if (_x+2 < _m && _y-1 >= 0){
		if (_board[_x+2][_y-1] == '.'){
			counter++;
		}
	}
	if (_x+2< _m && _y+1 < _n){
		if (_board[_x+2][_y+1] == '.'){
			counter++;
		}
	}
	if (_x-2 >= 0 && _y-1 >= 0){
		if (_board[_x-2][_y-1] == '.'){
			counter++;
		}
	}
	if (_x-2 >= 0 && _y+1 < _n){
		if (_board[_x-2][_y+1] == '.'){
			counter++;
		}
	}
	if (_x+1 < _m && _y-2 >= 0){
		if (_board[_x+1][_y-2] == '.'){
			counter++;
		}
	}
	if (_x+1<_m && _y+2 < _n){
		if (_board[_x+1][_y+2] == '.'){
			counter++;
		}
	}
	if (_x-1>=0 && _y-2 >= 0){
		if (_board[_x-1][_y-2] == '.'){
			counter++;
		}
	}
	if (_x-1>= 0 && _y+2 < _n){
		if (_board[_x-1][_y+2] == '.'){
			counter++;
		}
	}
	return counter;
}


char** legalMoves(char**_board,int _x, int _y, int _m, int _n){ //return parallel arrays for x and y coordinates
/*
	Need to know how many places can go, and where to go
	Take current location and board size

*/
	int counter = numLegalMoves(_board,_x,_y,_m,_n);

	//contains array for x and y
	char** xys = calloc(2,sizeof(char*));
	char* xs = calloc(counter,sizeof(int));
	char* ys = calloc(counter,sizeof(int));
	xys[0] = xs;
	xys[1] = ys;
	counter = 0;

	//8) -1 -2
	if (_x-1>=0 && _y-2 >= 0){
		if (_board[_x-1][_y-2] == '.'){
			xs[counter] = _x-1;
			ys[counter] = _y-2;
			counter++;
		}
	}
	//1) -2 -1
	if (_x-2 >= 0 && _y-1 >= 0){
		if (_board[_x-2][_y-1] == '.'){
			xs[counter] = _x-2;
			ys[counter] = _y-1;
			counter++;
		}
	}
	//2) -2 +1
	if (_x-2 >= 0 && _y+1 < _n){
		if (_board[_x-2][_y+1] == '.'){
			xs[counter] = _x-2;
			ys[counter] = _y+1;
			counter++;
		}
	}
	//3) -1 +2
	if (_x-1>= 0 && _y+2 < _n){
		if (_board[_x-1][_y+2] == '.'){
			xs[counter] = _x-1;
			ys[counter] = _y+2;
			counter++;
		}
	}
	//4) +1 +2
	if (_x+1<_m && _y+2 < _n){
		if (_board[_x+1][_y+2] == '.'){
			xs[counter] = _x+1;
			ys[counter] = _y+2;
			counter++;
		}
	}
	//5)  +2 +1
	if (_x+2< _m && _y+1 < _n){
		if (_board[_x+2][_y+1] == '.'){
			xs[counter] = _x+2;
			ys[counter] = _y+1;
			counter++;
		}
	}
	//6) +2 -1
	if (_x+2 < _m && _y-1 >= 0){
		if (_board[_x+2][_y-1] == '.'){
			xs[counter] = _x+2;
			ys[counter] = _y-1;
			counter++;
		}
	}
	//7) +1 -2
	if (_x+1 < _m && _y-2 >= 0){
		if (_board[_x+1][_y-2] == '.'){
			xs[counter] = _x+1;
			ys[counter] = _y-2;
			counter++;
		}
	}



	return xys;

}




int solver(char** _board,int _x,int _y,int _m, int _n){
	/*
	recursive function for solving
	*/

	#ifdef DEBUG_MODE
	printf("Child details %d %d %d %d\n",_x,_y,_m,_n);
	printBoard(_board,_m,_n);
	#endif

	int a = numLegalMoves(_board,_x,_y,_m,_n); //the number of legal moves
	#ifdef DEBUG_MODE
	printf("Child thread: %ld has %d legal moves continuing solve\n",pthread_self(),a);
	#endif
	

	//base cases
	if (tourCompleted(_board,_m,_n)){//finish knights tour
		printf("THREAD %ld: Sonny found a full knight's tour!\n",pthread_self());
		//free completed board here
		int x = (_m * _n);
		//compare and update global max
		//==========critical section ====================
		pthread_mutex_lock(&lock);
		if (x > max_squares){
			max_squares = x;
		}
		pthread_mutex_unlock(&lock);
		//==========end critical ========================
		return x;
	}else if(a == 0){//reach dead end
		printf("THREAD %ld: Dead end after move #%d\n",pthread_self(),numMovesUsed(_board,_m,_n));
		//save board to dead ends if needed, if not free and exit
		int x = numMovesUsed(_board,_m,_n);
		//==========critical section ====================
		pthread_mutex_lock(&lock);
		if (x > max_squares){
			max_squares = x;
		}
		//storing dead end boards
		if(arg3 == -1){ //if no argument 3 given store all dead ends
			if (dead_i == dead_size){
				char*** resized = realloc(dead_end_boards,dead_size +5);
				dead_end_boards = resized;
				dead_size = dead_size+5;
			}
			dead_end_boards[dead_i] = _board;
			dead_i++; //move index
		} else if (x >= arg3 ){//if arg3 given and board has at least arg3 squares covered
			if (dead_i == dead_size){
				char*** resized = realloc(dead_end_boards,dead_size +5);
				dead_end_boards = resized;
				dead_size = dead_size+5;
			}
			dead_end_boards[dead_i] = _board; //move ptr to global scope
			dead_i++;
		} else { //otherwise board must be freed
			freeBoard(_board,_m);
		}
		pthread_mutex_unlock(&lock);
		//==========end critical ========================		
		return x;
	}else if (a==1){ // otherwise continue recursively solving
		//one possibility, do not make new threads
		//printBoard(_board,_m,_n);
		char** leg = legalMoves(_board,_x,_y,_m,_n); //create array of possible x (leg[0]) and y (leg[0])
		//next arguments for recursion
		//printf("Debugging:currently at %d %d moving to %d %d\n",_x,_y,leg[0][0],leg[0][1]);
		int xcoord = leg[0][0];
		int ycoord = leg[1][0];
		_board[xcoord][ycoord] = 'S';
		//reused the edited board from function arguments
		int x = solver(_board,xcoord,ycoord,_m,_n);

		free(leg[0]);
		free(leg[1]);
		free(leg);

		return x;

		
	}else{//2+ threads needed, work in progress
		printf("THREAD %ld: %d moves possible after move #%d; creating threads...\n",pthread_self(),a,numMovesUsed(_board,_m,_n));
		int rc;
		pthread_t tid[a];

		char** leg = legalMoves(_board,_x,_y,_m,_n);
		void* threadMax = 0;

		for (int i= 0;i<a;i++){
			struct solverArgs* nextArgs = malloc(sizeof(*nextArgs));
			nextArgs->_m = _m;
			nextArgs->_n = _n;
			//move to that spot
			nextArgs->_x = leg[0][i]; //position in loop, depends on leg arrays
			nextArgs->_y = leg[1][i];
			//have to copy board if new thread
			char** myCopy = copy(_board,_m,_n);
			myCopy[nextArgs->_x][nextArgs->_y] = 'S'; //edit the copy
			nextArgs->_board = myCopy;

			rc = pthread_create(&tid[i],NULL,threadSolve, nextArgs);
			if (rc != 0){
				fprintf(stderr,"THREAD %ld: could not create thread (%d)",pthread_self(),i);
				return EXIT_FAILURE;
			}
			//if no parallel flag threads must be joined after creation
			#ifdef NO_PARALLEL
				
				void* threadRet;
				rc = pthread_join(tid[i],&threadRet);
				if (rc != 0){
					fprintf(stderr,"THREAD %ld: could not join thread %ld (rc:%d)\n",pthread_self(),tid[i],rc);
					return EXIT_FAILURE;
				}else{
					printf("THREAD %ld: thread [%ld] joined (returned %ld)\n",pthread_self(),tid[i],(long)threadRet); //should return a value
					if ((long)threadRet>(long)threadMax){//if child of this thread has max larger than this one overwrite
						threadMax = threadRet;
					}
				}
			#endif

		}
		//must rejoin the threads if no parallel is not active
		#ifndef NO_PARALLEL
			for (int i=0; i<a; i++){
				void* threadRet;
				rc = pthread_join(tid[i],&threadRet);
				if (rc != 0){
					fprintf(stderr,"THREAD %ld: could not join thread %ld (rc:%d)\n",pthread_self(),tid[i],rc);
					return EXIT_FAILURE;
				}else{
					printf("THREAD %ld: thread [%ld] joined (returned %ld)\n",pthread_self(),tid[i],(long)threadRet); //should return a value
					if ((long)threadRet>(long)threadMax){//if child of this thread has max larger than this one overwrite
						threadMax = threadRet;
					}
				}
			}
		#endif
		//printf("THREAD %ld: %d moves possible after move #%d; creating threads...\n",pthread_self(),a,numMovesUsed(_board,_m,_n));
		freeBoard(_board,_m);
		//free legs
		free(leg[0]);
		free(leg[1]);
		free(leg);
		return (long)threadMax; 
	}	
}
void* threadSolve(void* arg){
	struct solverArgs* _args = arg; //pull arguments from pthread_create, convert
	//create local versions of arguments
	char** myboard = _args->_board;
	int myx = _args->_x;
	int myy = _args->_y;
	int mym = _args->_m;
	int myn = _args->_n;
	long max = (long)solver(myboard,myx,myy,mym,myn);
	void* vmax = (long*)max;
	free(arg);

	pthread_exit(vmax);

}

int main(int argc, char** argv){
	/*
	Three arguments:
	0: dimension m
	1: dimension n
	2: x optional argument
	*/
	if (argc != 3 && argc != 4){
		fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: a.out <m> <n> [<x>]\n");
		return EXIT_FAILURE;
	}
	int m = atoi(argv[1]);
	int n = atoi(argv[2]);

	if (m<=2 || n<=2){//if either m or n under 2 error
		fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: a.out <m> <n> [<x>]\n");
		return EXIT_FAILURE;
	}

	//int x = -1;
	if (argc == 4){
		arg3 = atoi(argv[3]);
	}

	if (argc == 4 && arg3 > (m*n) ){ //if optional argument is present x < m * n
		fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: a.out <m> <n> [<x>]\n");
		return EXIT_FAILURE;
	}

	if (pthread_mutex_init(&lock,NULL) != 0){ //check the mutex initialization
		fprintf(stderr,"Mutex lock failed\n");
		return EXIT_FAILURE;
	}

	dead_end_boards = calloc(64,sizeof(char**)); //array for storing, must be freed later
	dead_size = 64;
	dead_i = 0;
	/*
	Create the m by n board
	*/

	char ** board = calloc(m , sizeof(char*));
	for (int i  = 0; i< m; i++){
		board[i] = calloc(n,sizeof(char));
	}
	for (int i = 0; i<m; i++){
		for (int j = 0; j<n; j++){
			board[i][j] = '.';
		}
	}
	board[0][0] = 'S'; //initialize corner


	#ifdef DEBUG_MODE
	printBoard(board,m,n);
	#endif

	char** leg = legalMoves(board,0,0,m,n); //arrays for x and y 's of legal coordinates
	int a = numLegalMoves(board,0,0,m,n); //number of legal moves also number of children
	// printf("legal moves: %d\n",a);
	// for (int i=0;i<a;i++){
	// 	printf("%d , %d\n",leg[0][i],leg[1][i]);
	// }

	printf("THREAD %ld: Solving Sonny's knight's tour problem for a %dx%d board\n",pthread_self(),m,n);
	printf("THREAD %ld: %d moves possible after move #1; creating threads...\n",pthread_self(),a);

	/*
		pthread multithreading
	*/
	pthread_t tid[a];
	int rc;

	for (int i = 0; i<a; i++){// for each possible move create a new thread.
		//make local copy of board
		char ** myCopy = copy(board,m,n);
		struct solverArgs* InitArg = malloc(sizeof(*InitArg));
		InitArg->_m = m;
		InitArg->_n = n;
		InitArg->_board = myCopy;
		InitArg->_x = leg[0][i]; //x coordinate of destination
		InitArg->_y = leg[1][i]; //y coordinate of destination
		//move current location to destination
		myCopy[InitArg->_x][InitArg->_y] = 'S';
		// printBoard(board,m,n);

		//printf("LOOP ITERATION %d out of %d\n",i,a);
		rc = pthread_create(&tid[i],NULL,threadSolve, InitArg);
		if (rc != 0){
			fprintf(stderr,"THREAD %ld: could not create thread (%d)",pthread_self(),i);
			return EXIT_FAILURE;
		}
		//argument InitArg will get freed in child thread, freeing here will free to early 

		//optional agument for no parallel
		#ifdef NO_PARALLEL
			void* threadMax;
			rc = pthread_join(tid[i],&threadMax);
			if (rc != 0){
				fprintf(stderr,"THREAD %ld: could not join thread %ld (rc:%d)\n",pthread_self(),tid[i],rc);
				return EXIT_FAILURE;
			}else{
				printf("THREAD %ld: thread [%ld] joined (returned %ld)\n",pthread_self(),tid[i],(long)threadMax); //should return a value
			}
		#endif
	}

	#ifndef NO_PARALLEL
		//must rejoin the threads
		for (int i=0; i<a; i++){
			void* threadMax;
			rc = pthread_join(tid[i],&threadMax);
			if (rc != 0){
				fprintf(stderr,"THREAD %ld: could not join thread %ld (rc:%d)\n",pthread_self(),tid[i],rc);
				return EXIT_FAILURE;
			}else{
				printf("THREAD %ld: thread [%ld] joined (returned %ld)\n",pthread_self(),tid[i],(long)threadMax); //should return a value
			}
		}
	#endif

	printf("THREAD %ld: Best solution(s) found visit %ld squares (out of %d)\n",pthread_self(),max_squares,m*n);
	//if needed print dead end boards and then free
	printf("THREAD %ld: Dead end boards:\n",pthread_self());
	for (int k =0;k<dead_i;k++){
		for (int i= 0;i<m;i++){
			if (i==0){
				printf("THREAD %ld: > ",pthread_self());
			}else{
					printf("THREAD %ld:   ",pthread_self());
			}
			for (int j=0;j<n;j++){
				
				printf("%c",dead_end_boards[k][i][j]);
			}
			printf("\n");
		}

	}



	freeBoard(board,m);
	free(leg[0]);
	free(leg[1]);
	free(leg);
	for (int i = 0;i<dead_i;i++){
		freeBoard(dead_end_boards[i],m);
	}
	free(dead_end_boards);
	pthread_mutex_destroy(&lock);


}