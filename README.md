
elchai bentov <br />
make <br />
exercise 3.<br />
--------------------------------------------------------------------------------------------------------------------------------
these program simulate a router, i.e., if there are n routers, you should run it n times in n different terminals.
this program  will implement the bellman ford algorithm for routing.

--------------------------------------------------------------------------------------------------------------------------------
this program contain 8 founction:

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
    
    --discribes of this functions apear in ex3.c--
    
-----------------------------------------------------------------------------------------------------------------------------
this program contain 3 structs:

    1.communication
    //==============================================================================
    2.router
    //==============================================================================
    3.all_routers
    //==============================================================================
    
    
        --discribes of this structs in ex3.c--

-----------------------------------------------------------------------------------------------------------------------------
files in the tar:
in the archive there are 2  files:
1.README
2.ex3.c- implement the functions and include main.
3.MAKEFILE - for compile the program.
--------------------------------------------------------------------------------------------------------------------------------

how to compile and run:
-first open the terminal in linux.
-now go to where the files are.
-enter make.
-run the program with: './ex3
