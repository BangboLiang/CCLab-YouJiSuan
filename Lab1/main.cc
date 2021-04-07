#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>

#include "sudoku.h"
#define TASKNUM 5000000
#define threadnum 12



pthread_t pid[threadnum];
pthread_mutex_t mutex;

char question[TASKNUM][128];
char answer[TASKNUM][128];

int total_ques = 0;
int curr_ques = 0;
int total_solved = 0;
bool all_done = false;
bool (*solve)(int,bool[][NUM+1],int[],int[],int[],int) = solve_sudoku_min_arity_cache;

int getajob(){
	int result;
	pthread_mutex_lock(&mutex);
	if (curr_ques >= total_ques){
		all_done = true;
		//printf("all done is true\n");
		pthread_mutex_unlock(&mutex);
		return -1; //all done
		
	}
	result = curr_ques;
	curr_ques++;
	pthread_mutex_unlock(&mutex);
	return result;
}

void* myfunc (void*){
	int board[N];
	int spaces[N];
	bool occupied[N][NUM+1];
	int arity[N];

	while(!all_done){
		//printf("try to get a job\n");
		int id = getajob();
		if (id == -1){
		//printf("curr_ques:%d\n",curr_ques);
		//printf("id is :%d\n",id);
		//printf("didn't get job!\n");
			//all_done = true;
			break;
		}
		//printf("try to input the puzzle\n");
		int t = input(question[id],board,spaces);
		init_cache(occupied,arity,board);
		//printf("inited the cache!\n");
		if (solve (0,occupied,arity,board,spaces,t)){
			//printf("try to print answer\n");
			for (int i = 0; i < 81; ++i){
				answer[id][i] = board[i] + '0';
				//printf("%d",board[i]);
			}
			//printf("\n");
			pthread_mutex_lock(&mutex);
			total_solved ++;
			pthread_mutex_unlock(&mutex);
			if (!solved(board))
          			assert(0);
		}
		else{
			printf("No: %s", question[id]);		
		}
	}
	//printf("exiting myfunc\n");
	return NULL;
}

void Create_pthreads(){
	for (int i = 0;i < threadnum ; ++i){
		pthread_create(&pid[i],NULL,myfunc,NULL);
	}
	//printf("exiting Create_pthreads\n");
}

int64_t now()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000000 + tv.tv_usec;
}

int main(int argc, char* argv[])
{
  init_neighbors();
  char filename[30]/*="./test1000"*/;
  scanf("%s",filename);
  FILE* fp = fopen(filename, "r");
  int64_t start = now();
  while (fgets(question[total_ques], sizeof question[total_ques], fp) != NULL) {
    if (strlen(question[total_ques]) >= N) {
      ++total_ques;
	}
      else {
        printf("No: %s", question[total_ques]);
      }
    }
	//printf("puzzle num :%d\n",total_ques);
  Create_pthreads();
//printf("try to wait all the pthreads\n");
  for (int i = 0;i < threadnum ; ++i){
   pthread_join(pid[i],NULL);
  }
//printf("Waited the pthreads\n");
//   for (int i = 0; i < total_ques; ++i)
//   {
//   	 printf("%s\n",answer[i]);
//   }
  int64_t end = now();
  for (int i = 0;i < total_ques; ++i){
        printf("%s\n",answer[i]);
  }
  double sec = (end-start)/1000000.0;
  //printf("%f sec %f ms each %d\n", sec, 1000*sec/total_ques, total_solved);

  return 0;
}

