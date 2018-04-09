//elchai bentov 305500704
//ex3 routers
#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include  <fcntl.h>
#include <stdint.h>
#include <signal.h>
//==============================================================================
typedef struct all_routers {
    char * ip;
    int port;
    char * name;
    int index;
} all_routers;
//==============================================================================

typedef struct router {
    //for synchronization
    pthread_cond_t * wake_calculator; 
    pthread_mutex_t * lock;
    pthread_cond_t * wake_sender;
    //============================
    char ** via_arr; // array represent via to the destination.
    int calcurator_finished; // boolean
    int dv_modified; //boolean
    int index_col;
    int index_row;
    char * name;
    char * ip;
    int port;
    int num_of_routers;
    int num_of_neighbors;
    int * my_dv; 
    int ** expanded_dv;
    all_routers ** all_routers;
    struct router ** my_neighbors;
    int * neighbors_sent;
    int * neighbors_finished;
} router;
//==============================================================================
typedef struct communication {
    pthread_cond_t * wake_reciver;
    int trial;
    char * ip;
    int index;
    int n_port; //neighbor port
    int my_port;
    int * dv;
    int ** f_dv;
    char * my_name; //my name
    char * neighbor_name;
    router * router;
} communication;

//==============================================================================
void * calculate(void * myne);
//==============================================================================
int sum_asci(char * str);
//==============================================================================
int hash_func(router * my_router,char * name);
//==============================================================================
all_routers * create_router(char * line, router * my_router,int i);
//==============================================================================
void put_parameters(char *argv[], router * my_router);
//==============================================================================
void * send_data(void * args);
//==============================================================================
void * receive_data(void * args);
//==============================================================================
void free_my_router(router * my_router);
//==============================================================================
int main(int argc, char *argv[]) { 
    router * my_router= (router *) malloc(sizeof(router));
    my_router->name=malloc(sizeof(char)*strlen(argv[2])+1);
    strcpy(my_router->name,argv[2]);
    put_parameters(argv,my_router);
    my_router->neighbors_sent=calloc(sizeof(int),(my_router->num_of_neighbors-1));
    my_router->neighbors_finished=calloc(sizeof(int),(my_router->num_of_neighbors-1));
    pthread_t * thread_receiver = malloc (sizeof(pthread_t)*(my_router->num_of_neighbors-1)); 
    pthread_t  * thread_sender=malloc(sizeof(pthread_t)*(my_router->num_of_neighbors-1)); 
    pthread_t  * calculator=malloc(sizeof(pthread_t)); 
    communication   ** args = malloc(sizeof(communication*)*(my_router->num_of_neighbors-1));
    if(my_router==NULL|| !my_router->neighbors_sent||!my_router->neighbors_finished||thread_sender==NULL||thread_receiver==NULL||calculator==NULL||args==NULL)    
    {
        perror("Can't alloceate memory");
        exit(1);
    }
    my_router->calcurator_finished=0;
   	int rc, t,g,d;
    d=pthread_create(& calculator[0], NULL,calculate, my_router);
    if (d<0)
    { 
	    perror("create pthread failed"); 
	    exit(-1); 
	} 
	pthread_mutex_t lock; 
	pthread_cond_t cv; 
	pthread_cond_t cv1;
	pthread_mutex_init(&lock, NULL); 
	pthread_cond_init(&cv, NULL); 
	pthread_cond_init(&cv1, NULL);
	my_router->wake_sender=&cv1;
	my_router->dv_modified=1;
	pthread_mutex_init(&lock, NULL); 
    my_router->lock=&lock;
	my_router->wake_calculator=&cv;
	for(t=1;t<my_router->num_of_neighbors;t++)
	{ 
	    args[t-1] = malloc(sizeof(communication));
	    if(args[t-1]==NULL)
	    {
	        perror("Can't alloceate memory");
            exit(1);
	    }
		args[t-1]->router=my_router;
		args[t-1]->trial=atoi(argv[3]);
		args[t-1]->n_port=my_router->my_neighbors[t]->port;
		args[t-1]->ip=(my_router->ip);
		args[t-1]->dv = my_router->my_dv;
		args[t-1]->f_dv = my_router->expanded_dv;
		args[t-1]->index=my_router->my_neighbors[t]->index_row;
		args[t-1]->my_name=my_router->name;
		args[t-1]->my_port=my_router->port;
		args[t-1]->neighbor_name=my_router->my_neighbors[t]->name;
		rc = pthread_create(&thread_sender[t-1], NULL,send_data, args[t-1]); 
		g = pthread_create(&thread_receiver[t-1], NULL,receive_data, args[t-1]);
		if (rc<0||g<0)
		{ 
		    perror("create pthread failed"); 
	        exit(-1); 
		} 

	}
	for(t=0;t<my_router->num_of_neighbors-1;t++)
	{
		pthread_join(thread_receiver[t], NULL);
		pthread_join(thread_sender[t], NULL);
	}
	if(my_router->num_of_neighbors==1) //i have no neighbors
    {
        sleep(3);//for the calculte reach to wait
        pthread_cond_signal(my_router->wake_calculator);
    }
	pthread_join(calculator[0], NULL);
	free(thread_receiver);
	free(thread_sender);
	free(calculator);
	int i;
	for(i=0;i<my_router->num_of_neighbors-1;i++)
	{
	    free(args[i]);
	}
	free(args);
    free_my_router(my_router);
	return 0;
}




//==============================================================================
void put_parameters(char *argv[],router *my_router )
{ 
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
            printf("\nhello\n");

        perror("can't open file");
        exit(-1);
    }
        //printf("\nhello\n");

    char * token;
    getline(&line, &len, fp);
    token = strtok(line, " ");
    my_router->num_of_routers = atoi(token);
    int i=0;
    int count=my_router->num_of_routers;
    char * my_name=my_router->name;
    my_router ->all_routers= (all_routers **) malloc(sizeof(all_routers*)*my_router->num_of_routers);
    my_router->via_arr=calloc(sizeof(char*),my_router->num_of_routers);
    my_router->my_neighbors=malloc(sizeof(router*));
    my_router->my_dv=calloc(sizeof(int),my_router->num_of_routers);
    if(!my_router ->all_routers||!my_router->via_arr||!my_router->my_neighbors||! my_router->my_dv)
    { 
        perror("Can't alloceate memory");
        exit(1);
    }
    while ((read = getline(&line, &len, fp)) != -1&&count!=0)  //create all router
    {
        count--;
        my_router->all_routers[i]=create_router(line,my_router,i);
        i++;
    }
    int num_of_neighbors=0;
    i=0;
    for(;i<my_router->num_of_routers;i++)
    {
        my_router->my_dv[i]=INT_MAX;
    }
    while (read !=-1) //create neighbor routers
    {  
        token = strtok(line, " ");
        char * first=token;
        char * second="";
        int third=0;
        token = strtok(NULL, " ");
        second=token;
        token = strtok(NULL, " ");
        third=atoi(token);
        if(strcmp(my_name,first)==0||strcmp(my_name,second)==0)
        {
            num_of_neighbors++;
            //aclocation for the neighbors
            my_router->my_neighbors=realloc(my_router->my_neighbors,((num_of_neighbors+1)*sizeof(router*)));//maybe without +1
            my_router->my_neighbors[num_of_neighbors]=  malloc(sizeof(router));
            my_router->my_neighbors[num_of_neighbors]->my_dv=calloc(sizeof(int),my_router->num_of_routers);
            my_router->my_neighbors[num_of_neighbors]->name=calloc(sizeof(char),strlen(my_name)+1);
            //==================================================================================
            if(!my_router->my_neighbors || ! my_router->my_neighbors[num_of_neighbors] || !my_router->my_neighbors[num_of_neighbors]->my_dv || !my_router->my_neighbors[num_of_neighbors]->name )
            { 
                perror("Can't alloceate memory");
                exit(1);
            }
            for(i=0;i<my_router->num_of_routers;i++)
            {
                my_router->my_neighbors[num_of_neighbors]->my_dv[i]=INT_MAX;
            }
            if(strcmp(my_name,first)==0)
            {
                
                my_router->my_dv[hash_func(my_router,second)]=third;
                strcpy(my_router->my_neighbors[num_of_neighbors]->name,second);
            }
            else
            {
                my_router->my_dv[hash_func(my_router,first)]=third;
                strcpy(my_router->my_neighbors[num_of_neighbors]->name,first);
            }
            my_router->my_neighbors[num_of_neighbors]->index_row=num_of_neighbors;
            my_router->my_neighbors[num_of_neighbors]->index_col=hash_func(my_router,my_router->my_neighbors[num_of_neighbors]->name);
            my_router->my_neighbors[num_of_neighbors]->my_dv[hash_func(my_router,my_router->my_neighbors[num_of_neighbors]->name)]=0;
            my_router->via_arr[hash_func(my_router,my_router->my_neighbors[num_of_neighbors]->name)]=malloc(sizeof(char)*strlen(my_router->my_neighbors[num_of_neighbors]->name)+1);
            strcpy( my_router->via_arr[hash_func(my_router,my_router->my_neighbors[num_of_neighbors]->name)],my_router->my_neighbors[num_of_neighbors]->name);
            int i=0;
            int port=0;
            for(;i<my_router->num_of_routers;i++)
            {
                if(strcmp(my_router->all_routers[i]->name,my_router->my_neighbors[num_of_neighbors]->name)==0)
                {
                    port=my_router->all_routers[i]->port;
                }
            }
            my_router->my_neighbors[num_of_neighbors]->port=port;
        }
        read = getline(&line, &len, fp);
    }
    my_router->my_dv[hash_func(my_router,my_name)]=0;
    my_router->num_of_neighbors=num_of_neighbors+1;
    my_router->ip=malloc(sizeof(char)*strlen(my_router->all_routers[0]->ip)+1);
     if(! my_router->ip )
    {
        perror("Can't alloceate memory");
        exit(1);
    }
    strcpy(my_router->ip,my_router->all_routers[0]->ip);
    my_router->port=my_router->all_routers[my_router->index_col]->port;
    my_router->expanded_dv = malloc (sizeof (int*) * (my_router->num_of_neighbors));
    if(!my_router->expanded_dv )
    {
        perror("Can't alloceate memory");
        exit(1);
    }
    my_router->my_neighbors[0]=my_router; //alliasin!
    for(i=0;i<my_router->num_of_neighbors;i++)
    {
         my_router->expanded_dv[i]=my_router->my_neighbors[i]->my_dv;
    }

    fclose(fp);
    if (line)
        free(line);
        
}


//==============================================================================


all_routers * create_router(char *line, router * my_router, int i)
{
    char * token = strtok(line, " ");
     if(strcmp(token,my_router->name)==0)
     {
        my_router->index_col=i;
     }
    all_routers  * temp = malloc(sizeof(all_routers));
    temp->name=malloc(sizeof(char)*strlen(token)+1);
    if(!temp->name )
    {
        perror("Can't alloceate memory");
        exit(1);
    }
    strcpy(temp->name,token);
    token = strtok(NULL, " ");
    temp->ip = malloc (sizeof(char)*strlen(token)+1);
    if(!temp->ip )
    {
        perror("Can't alloceate memory");
        exit(1);
    }
    strcpy(temp->ip,token);
    token = strtok(NULL, " ");
    temp->port=atoi(token);
    temp->index=i;
    return temp;
}
//==============================================================================
void * send_data(void * args)
{
    signal(SIGPIPE, SIG_IGN);
    communication *actual_args = args;
    struct sockaddr_in srv;
    srv.sin_family = AF_INET;
    int sock;
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
	    actual_args->router->neighbors_finished[actual_args->index-1]=1000;
        actual_args->router->neighbors_sent[actual_args->index-1]=1000;
        perror("Can't create TCP socket");
	    return NULL;
	}
	srv.sin_port=htons(actual_args->n_port+sum_asci(actual_args->my_name));
	srv.sin_addr.s_addr=inet_addr(actual_args->ip);
	int i=0;
	for(;i<actual_args->trial;i++)
	{
	    if((connect(sock, (struct sockaddr *)&srv, sizeof(srv))) < 0 )
	    {
	        if( i>=actual_args->trial-1)
	        {
	            actual_args->router->neighbors_finished[actual_args->index-1]=1000; //for the reciever dont wait to my messeage.
                actual_args->router->neighbors_sent[actual_args->index-1]=1000;
	            perror("Could not connect");
            	return NULL;
	        }
	        else
	        {
	            sleep(1);
	        }
	    }
	    else
	        break;
	}
	int *  arrayToSend = malloc(sizeof(int32_t)*(actual_args->router->num_of_routers+1));
	if(! arrayToSend )
    {
        perror("Can't alloceate memory");
        return NULL;
    }
	pthread_mutex_lock(actual_args->router->lock); 
    while(1)
	{         
	   
  	    int nbytes=1;
        for (i=1 ; i < actual_args->router->num_of_routers+1 ; ++i)
        {
            arrayToSend[i] =htonl(actual_args->dv[i-1]) ;
        }
        if(actual_args->router->dv_modified)
        { 
            arrayToSend[0]=htonl(1);
        }
        else
            arrayToSend[0]=htonl(0);
  	    if((nbytes =write(sock,arrayToSend,sizeof(int32_t)*(actual_args->router->num_of_routers+1)) < 0))
        {
            perror("write");
            pthread_mutex_unlock(actual_args->router->lock);
            close(sock);
            free(arrayToSend);
    	   return NULL;
        }
        else
        {
                    if(actual_args->router->calcurator_finished)
	                {
	                     pthread_mutex_unlock(actual_args->router->lock);
	                    close(sock);
	                    free(arrayToSend);
	                    return NULL;
	                }

        }
          pthread_cond_wait(actual_args->router->wake_sender, actual_args->router->lock);
	}
  	close(sock);
	pthread_exit(NULL);
}
//==============================================================================
void * receive_data(void * args)
{  

    signal(SIGPIPE, SIG_IGN);
    struct sockaddr_in cli;	      /* used by accept() */
    int newfd;			      /* returned by accept() */
    socklen_t cli_len = sizeof(cli);	/* used by accept() */
    communication *actual_args = args;
    int fd;		/* socket descriptor */
    if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        actual_args->router->neighbors_finished[actual_args->index-1]=1000;
        actual_args->router->neighbors_sent[actual_args->index-1]=1000;
	    perror("Can't create TCP socket");
	    return NULL;
    }
	struct sockaddr_in srv;	/* used by bind() */

    /* create the socket */

    srv.sin_family = AF_INET; /* use the Internet addr family */
    srv.sin_port = htons(actual_args->my_port+sum_asci(actual_args->neighbor_name)); /* bind socket ‘fd’ to port 80*/
    /* bind: a client may connect to any of my addresses */
    srv.sin_addr.s_addr = INADDR_ANY;
    if(bind(fd, (struct sockaddr*) &srv, sizeof(srv)) < 0)
    {
    	perror("bind"); 
    	return NULL;
    }
    if(listen(fd, 1) < 0) 
    {
	    perror("listen");
    	return NULL;
    }
    newfd = accept(fd, (struct sockaddr*) &cli, &cli_len);
    if(newfd < 0)
    {
	    perror("accept");
	    return NULL;
    }
    int nbytes=1;			      /* used by read() */
    int i = 0;
    int * arrayReceived = malloc(sizeof(int32_t)*(actual_args->router->num_of_routers+1)) ;
    if(! arrayReceived )
    {
        perror("Can't alloceate memory");
        return NULL;
    }
    while(1)
    {
        if((nbytes =recv(newfd, arrayReceived, sizeof(int32_t)*(actual_args->router->num_of_routers+1), MSG_WAITALL))<=0)
        {
            actual_args->router->neighbors_finished[actual_args->index-1]=1000;
            actual_args->router->neighbors_sent[actual_args->index-1]=1000;
            free(arrayReceived);
            close(fd);
            if(nbytes<0)
	            perror("read");
	        return NULL;
        }
        actual_args->router->neighbors_sent[actual_args->index-1]=1;
        if(ntohl(arrayReceived[0])==0)
        {
            actual_args->router->neighbors_finished[actual_args->index-1]=1;
        }
        else
            for(i=1;i<actual_args->router->num_of_routers+1;i++)
            {
                actual_args->f_dv[actual_args->index][i-1]=ntohl(arrayReceived[i]);
            }
        int counter=0;
        for(i=0;i<actual_args->router->num_of_neighbors-1;i++)
        {
            if( actual_args->router->neighbors_sent[i]>0)
                counter++;
        }
        if(counter==actual_args->router->num_of_neighbors-1)
        {
            sleep(1);
            pthread_cond_signal(actual_args->router->wake_calculator);
        }
    }
    
    close(fd);
    pthread_exit(NULL);
}
//==============================================================================
int hash_func(router * my_router,char * name)
{
    int i=0;
    for(;i<my_router->num_of_routers;i++)
    {
        if(strcmp(name,my_router->all_routers[i]->name)==0)
        {
            return my_router->all_routers[i]->index;
        }
           
    }
    return -1;
}
//==============================================================================
int sum_asci(char * str)
{
    int sum=0;
    int len = strlen(str);
    int i;
    for ( i = 0; i < len; i++)
    {
        sum = sum + str[i];
    }
    return sum;
}
//==============================================================================
void * calculate(void * myne)
{
    sleep(1);
    while(1)
    {
        router * a_myne = myne;
        int i;
        int j=0;
        i=1;
        pthread_cond_wait(a_myne->wake_calculator, a_myne->lock); 
        int counter=0;
        for(i=0;i<a_myne->num_of_neighbors-1;i++)
        {
            if( a_myne->neighbors_finished[i]>0)
                counter++;
        }        a_myne->dv_modified=0;
        for(i=1;i<a_myne->num_of_neighbors;i++)
        {
            j=0;
            for(;j<a_myne->num_of_routers;j++)
            {
                if(a_myne->my_dv[j]>a_myne->expanded_dv[i][j]+a_myne->my_dv[hash_func(a_myne,a_myne->my_neighbors[i]->name)]&&a_myne->expanded_dv[i][j]+a_myne->my_dv[hash_func(a_myne,a_myne->my_neighbors[i]->name)]>-1)
                {
                    a_myne->dv_modified=1;
                    a_myne->my_dv[j]=a_myne->expanded_dv[i][j]+a_myne->my_dv[hash_func(a_myne,a_myne->my_neighbors[i]->name)];
                    a_myne->expanded_dv[0]= a_myne->my_dv;
                     free(a_myne->via_arr[j]);
                    a_myne->via_arr[j]=malloc(sizeof(char)*strlen(a_myne->my_neighbors[i]->name)+1);
                    //this is for check if the via to this nrighbor is throw another neighbor
                    if(strcmp(a_myne->via_arr[hash_func(a_myne,a_myne->my_neighbors[i]->name)],a_myne->my_neighbors[i]->name)==0)
                         strcpy(a_myne->via_arr[j],a_myne->my_neighbors[i]->name);
                    else 
                    {
                        strcpy(a_myne->via_arr[j],a_myne->via_arr[hash_func(a_myne,a_myne->my_neighbors[i]->name)]);
                    }
              
                }
            }
        }
          if(counter==a_myne->num_of_neighbors-1||a_myne->num_of_neighbors==1)
                {
                    
                    a_myne->dv_modified=0;
                    a_myne->calcurator_finished=1;
                    pthread_cond_broadcast(a_myne->wake_sender);
                    for(i=0;i<a_myne->num_of_routers;i++)
                    {
                        if(i!=a_myne->index_col)
                        {
                            int x=a_myne->my_dv[i];
                            if(x!=INT_MAX)
                                printf("\n %s %s %d \n",a_myne->all_routers[i]->name,a_myne->via_arr[i],a_myne->my_dv[i]);
                            else
                            printf("\n %s %s %s \n",a_myne->all_routers[i]->name,a_myne->via_arr[i],"infinity");
                        }

                        
                    }
                    pthread_mutex_unlock(a_myne->lock);
                    return NULL;
                }
       // a_myne->expanded_dv[0]=a_myne->my_dv;
        for(i=0;i<a_myne->num_of_neighbors-1;i++)
        {
            a_myne->neighbors_sent[i]--;

        }
         pthread_cond_broadcast(a_myne->wake_sender);
    }
    pthread_exit(NULL);
}
//====================================================================================================================
void free_my_router(router * my_router)
{
    int i;
    for(i=0;i<my_router->num_of_routers;i++)
    {
        free(my_router->via_arr[i]);
    }
    free(my_router->via_arr);
    free(my_router->name);
        my_router->name=NULL;
    free(my_router->ip);
    free(my_router->my_dv);
    for(i=1;i<my_router->num_of_neighbors;i++)//mabe problem
    {
        free(my_router->expanded_dv[i]);
    }
    free(my_router->expanded_dv);
    for(i=0;i<my_router->num_of_routers;i++)//mabe problem
    {
        free(my_router->all_routers[i]->name);
        free(my_router->all_routers[i]->ip);
        free(my_router->all_routers[i]);
    }
     free(my_router->all_routers);
      for(i=1;i<my_router->num_of_neighbors;i++)//mabe problem
    {
        free(my_router->my_neighbors[i]->name);
        my_router->my_neighbors[i]->ip=NULL;
        if(i!=0)
        free(my_router->my_neighbors[i]);
    }
     free(my_router->my_neighbors);
     free(my_router->neighbors_sent);
     free(my_router->neighbors_finished);
     free(my_router);
}
