#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define QUEU_SIZE 11
char queue [QUEU_SIZE] ; 
int front = 0 , rear = 0 ; // pointrs to front and rear of the queu
int items = 0 ; // number of items in queu
int done = 1 ; // to show that the input file is finished to read

pthread_mutex_t mutex ;
pthread_cond_t two_items_available ; // condition variable to show producer put a word in queue
pthread_cond_t three_items_available ; 
pthread_cond_t space_available ; // condition variable to show consumer delete a word from queue

void * p_thread() ;//the producer
void * c_thread_one() ;//the consumer
void * c_thread_two() ;

int main(int argc, char *argv[])
{

	pthread_t producer , consumer1,consumer2;
	//pthread_init();
	pthread_cond_init (&two_items_available, NULL) ;
	pthread_cond_init (&three_items_available, NULL) ;
	pthread_cond_init (&space_available, NULL) ;
	pthread_mutex_init (&mutex, NULL) ;
	
	if( pthread_create ( &producer , NULL, p_thread, NULL))  // create producer
	{
		fprintf(stderr, "Error creating producer thread\n");
		return 1;
	}

	if( pthread_create ( &consumer1 , NULL, c_thread_one, NULL))  // create the consumer
	{
		fprintf(stderr, "Error creating consumer thread\n");
		return 1;
	}

	if( pthread_create ( &consumer2 , NULL, c_thread_two, NULL))  // create the consumer
	{
		fprintf(stderr, "Error creating consumer thread\n");
		return 1;
	}
	
	if ( pthread_join ( producer , NULL ) ) // wait for all threads to finish
	{
		fprintf(stderr, "Error joining thread\n");
		return 2;
	}

	if ( pthread_join ( consumer1 , NULL ) ) 
	{
		fprintf(stderr, "Error joining consumer\n");
		return 2;
	}

	if ( pthread_join ( consumer2 , NULL ) ) 
	{
		fprintf(stderr, "Error joining consumer\n");
		return 2;
	}	
	
	return 0;
	
}

void * p_thread()
{	
	FILE *fp = fopen("strings.txt","r");
	int count= 0;
	int c ;
	//c = getc(fp);
	while ((c=fgetc(fp)) != EOF) 
	{
		//1
		sleep(1);
		pthread_mutex_lock (&mutex) ;
		//2
		printf ("	front<%d> rear<%d> items <%d>\n", front , rear , items ) ;	
		while ( items >= QUEU_SIZE ) // while there is no space in queue to write
		{
			pthread_cond_wait ( &space_available , &mutex );
		}
		//3	
		printf ("	front<%d> rear<%d> items <%d>\n", front , rear , items ) ;
		
		// now we cAN write in queue
		queue [front] = (char)c ;
		front ++ ;
		count++;
		if(front==QUEU_SIZE) front = 0 ;
		items ++ ;
		printf ("	character write to queue: <%c>\n" , (char)c ) ;
		
		
		if(count==2 )
		{
			pthread_cond_signal(&two_items_available); // wake up a consumer
			printf ("	wake up a consumer1 \n" ) ;	
		}
		
		if(count==3 )
		{
			pthread_cond_signal(&three_items_available); // wake up a consumer
			printf ("	wake up a consumer2 \n" ) ;
			count=0;	
		}
		pthread_mutex_unlock (&mutex) ;
		sleep (1);
		//c = getc(fp);		
	}
	pthread_mutex_lock (&mutex) ;
	done = 0 ; // we should tell the consumer that the file is finished
	pthread_cond_signal(&two_items_available);
	pthread_cond_signal(&three_items_available);
	pthread_mutex_unlock (&mutex) ;
	
	fclose (fp);

	
	//printf ("hello prof. Song Jiang\n") ;
	//return NULL;		
	 pthread_exit (0);
}

void * c_thread_one()
{	
	FILE* output1 = fopen ("result1.txt" , "w") ;
	//4
	while ( done != 0 ) { // while there is something to read
		pthread_mutex_lock (&mutex) ;
			//5
			printf ("front<%d> rear<%d> items <%d>\n", front , rear , items ) ;	
			while ( items <= 0 && done!= 0 ) // while there is no character in queu to read we should wait
				pthread_cond_wait ( &two_items_available , &mutex ) ;
			//6
			if(done==0)
			{
				pthread_mutex_unlock (&mutex) ;	
		 		break;
			}
			printf ("front<%d> rear<%d> items <%d>\n", front , rear , items ) ;	
				
			// read the character and write it:
			char c1 = queue [rear] ;
			rear ++ ;
			if (rear==QUEU_SIZE) rear = 0 ;
			char c2 =queue[rear];
			rear ++ ;
			if (rear==QUEU_SIZE) rear = 0 ;
			items = items-2 ;
			printf ("characters read from queue: <%c> <%c>\n" , c1,c2 ) ;
			printf ("wake up a producer \n" ) ;
			fprintf(output1,"%c%c",c1,c2) ;
			pthread_cond_signal(&space_available); // send signal for producer to show that thre is space
		
		pthread_mutex_unlock (&mutex) ;	
	}
	
	fclose (output1) ;
	//return NULL;
	pthread_exit (0);
}


void * c_thread_two()
{	
	FILE* output2 = fopen ("result2.txt" , "w") ;
	//4
	while ( done != 0 ) { // while there is something to read
		pthread_mutex_lock (&mutex) ;
			//5
			printf ("front<%d> rear<%d> items <%d>\n", front , rear , items ) ;	
			while ( items <= 0 && done!= 0 ) // while there is no character in queu to read we should wait
				pthread_cond_wait ( &three_items_available , &mutex ) ;
			//6
			if(done==0)
			{
				pthread_mutex_unlock (&mutex) ;	
		 		break;
			}
			printf ("front<%d> rear<%d> items <%d>\n", front , rear , items ) ;	
				
			// read the character and write it:
			char c1 = queue [rear] ;
			rear ++ ;
			if (rear==QUEU_SIZE) rear = 0 ;
			items = items-1 ;
			printf ("characters read from queue: <%c> \n" , c1 ) ;
			printf ("wake up a producer \n" ) ;
			fprintf(output2,"%c",c1) ;
			pthread_cond_signal(&space_available); // send signal for producer to show that thre is space
		
		pthread_mutex_unlock (&mutex) ;	
	}
	
	fclose (output2) ;
	//return NULL;
	pthread_exit (0);
}
