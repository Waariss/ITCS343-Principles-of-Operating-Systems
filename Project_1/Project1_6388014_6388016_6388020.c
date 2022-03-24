// Project 1 of ITCS 343 
// 6388014 Waris Damkham
// 6388016 Thanawath Huayhongthong
// 6388020 Chalumphu Atjarit
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "common.h"
#include "common_threads.h"
#ifdef linux
#include <semaphore.h>
#elif __APPLE__
#include "zemaphore.h"
#endif
#define BufferSize 5 // Size of the buffer
int pn = 0; //Process number 
int cn = 0; //Devices number
int in = 0; //Producer Buffer runner
int out = 0;//Consumer Buffer runner
int totreq = 0;//Total requests
int buffer[BufferSize];// array size of buffer
int turn = 0;// Turnaroundtime
int waiting = 0;// Waiting time
int j = 1; //Running process number
int mn = 0;
int maxt = 0;// Maximum time
int mint = 0;//Minimum time
double sum = 0;// Count total print
int action = 0;//Action
double statt =0 ;// Count Drop Request
int length=0;//For Counting the Buffer Status
sem_t empty;
sem_t full;
pthread_mutex_t mutex;
void *producer(void *pno){
	int item;
	for(int i = 0;i <totreq; i++){ //run loops by total requests
		item = j;
   		j++; //Adding process number to next number
   		if(action == 1){// Wait action
   			length ++;
   			int ran = (rand() % (500 - 100 + 1)) + 100;//Randoming Wait time
			waiting += ran; //Added time in to Waittime
			turn += ran; //Added time in to Turnaroundtime
			usleep(ran*1000);// wait
			sem_wait(&empty);
			pthread_mutex_lock(&mutex);
			buffer[in] = item;
			printf("Bartender %d: Starting brew menu number: %d at bar counter number: %d Waittime %d ms\n", *((int *)pno),buffer[in],in,ran);
			in++; // Count menu 
			if(in == BufferSize){// if menu is full reset
				in = 0;
			}
			pthread_mutex_unlock(&mutex);
			sem_post(&full);
   		}
   		else{
   			if(action == 2){ // drop action
	   			if(buffer[in]==BufferSize){ // if queue is full
	   				printf("Drop mocktail at bar counter number: %d\n",in);
	    				statt++;// Counting  Drop Request
	   			}
	   			else{
	   				length ++;
	   				int ran = (rand() % (500 - 100 + 1)) + 100;//Randoming Wait time
					waiting += ran;//Added time in to Waittime
					turn += ran;//Added time in to Turnaroundtime
					usleep(ran*1000);// wait
					sem_wait(&empty);
					pthread_mutex_lock(&mutex);
					buffer[in] = item;
					printf("Bartender %d: Starting brew menu number: %d at bar counter number: %d Waittime %d ms\n", *((int *)pno),buffer[in],in,ran);
					in++;// Count menu 
					if(in == BufferSize){// if menu is full reset
						in = 0;
					}
					pthread_mutex_unlock(&mutex);
					sem_post(&full);
	   			}
   			}
	   		else if(action == 3){// replace action
			    	if(buffer[in]==BufferSize){ // if queue is full
			    		statt++; // Counting  Drop Request
			    		// find min in buffer
			    		int min = 1e9;
			    		int index = 0;
			    		for(int k = 0; k < BufferSize; k++){
			    			if(buffer[k] < min){
			    				min = buffer[k];
			    				index = k;
			    			}
			    		}
			    		for(int h = 0; h < BufferSize;h++){
			    			printf(" %d ",buffer[h]);
			    		} 
			    		printf("\n");
			    		buffer[index] = item;
			    		printf("Bartender %d: Replace mocktail number: %d with mocktail number: %d at bar counter number: %d \n",*((int *)pno),min,item,in);
			    		for(int h = 0; h < BufferSize;h++){
			    			printf("%d ",buffer[h]);
			    		}
			    		printf("\n");
		    		}
		    		else{
		    			length ++;
		    			int ran = (rand() % (500 - 100 + 1)) + 100;//Randoming Wait time
					waiting += ran;//Added time in to Waittime
					turn += ran;//Added time in to Turnaroundtime
					usleep(ran*1000);// wait
			    		sem_wait(&empty);
					pthread_mutex_lock(&mutex);
					buffer[in] = item;
					printf("Bartender %d: Starting brew menu number: %d at bar counter number: %d Waittime %d ms\n", *((int *)pno),buffer[in],in,ran);
					in++;// Count menu 
					if(in == BufferSize){// if menu is full reset
						in = 0;
					}
					pthread_mutex_unlock(&mutex);
					sem_post(&full); 
		    		}
	   		} 
   		}
	}
	return NULL;
}
void *consumer(void *cno)
{   
    while (1){
    	length -- ;
    	int ran2 = (rand() % (maxt - mint + 1)) + mint; //Randoming Burst time
	turn += ran2; //Added time in to Turnaroundtime
	sem_wait(&full); 
	pthread_mutex_lock(&mutex);
	usleep(ran2*1000); //wait
	int item = buffer[out];
	printf("Customers %d: Pick up brew menu number: %d from bar counter number: %d Bursttime %d ms\n",*((int *)cno),item, out,ran2);
	out = (out+1)%BufferSize;
	pthread_mutex_unlock(&mutex);
	sem_post(&empty);
	sum++;//count print 
	if(length <= 0) break;
    }
    return NULL;
}
int main(int argc, char *argv[])
{  
    if(argc !=7)
    {
    	fprintf(stderr, "usage: %s <Processes number> <Devices number> <Total requests> <Minimum time> <Maximum time> <Action>\n", argv[0]);
        exit(1);
    }
    pn = atoi(argv[1]);
    cn = atoi(argv[2]);
    totreq = atoi(argv[3]);
    mint = atoi(argv[4]);
    maxt = atoi(argv[5]);
    action = atoi(argv[6]);
    pthread_t pro[pn],con[cn];
    pthread_mutex_init(&mutex, NULL);
    sem_init(&empty,0,BufferSize);
    sem_init(&full,0,0); 
    mn = cn;
    if(pn>cn) {
    	mn = pn;
    }
    int a[mn] ; //Just used for numbering the producer
    for (int i=0 ; i<mn ; i++){
    	a[i] = i;
    }
    printf("**********Bar is Open***********\n");
    for(int i = 0; i < pn; i++){
    	pthread_create(&pro[i], NULL, (void *)producer, (void *)&a[i]);
    	//a[i] = i + 1;
    }
    for(int i = 0; i < cn; i++) {
        pthread_create(&con[i], NULL, (void *)consumer, (void *)&a[i]);
        //b[i] = i + 1;
    }
    for(int i = 0; i < pn; i++) {
        pthread_join(pro[i], NULL);
    }
    for(int i = 0; i < cn; i++) {
        pthread_join(con[i], NULL);
    }
    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);
    printf("*******Sorry We're Closed********\n");
    //printf("Total Print = %d\n",(int)sum);
    //printf("Drop Request: %d \n",(int)statt);
    printf("Average Waiting time: %d ms\n",waiting/(pn*totreq));
    printf("Percentage of dropped requests: %.2lf %%\n",(double)(statt/(sum+statt))*100.00);
    printf("Total time of the simulation time: %d second \n",turn/1000);
    return 0;
}
