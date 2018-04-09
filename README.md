
elchai bentov <br />
make <br />
exercise 3.<br />
--------------------------------------------------------------------------------------------------------------------------------
these program simulate a router, i.e., if there are n routers, you should run it n times in n different terminals.<br />
this program  will implement the bellman ford algorithm for routing.<br />

--------------------------------------------------------------------------------------------------------------------------------<br />
this program contain 8 founction:<br />

    //==============================================================================<br />
    void * calculate(void * myne);<br />
    //==============================================================================<br />
    int sum_asci(char * str);<br />
    //==============================================================================<br />
    int hash_func(router * my_router,char * name);<br />
    //==============================================================================<br />
    all_routers * create_router(char * line, router * my_router,int i);<br />
    //==============================================================================<br />
    void put_parameters(char *argv[], router * my_router);<br />
    //==============================================================================<br />
    void * send_data(void * args);<br />
    //==============================================================================<br />
    void * receive_data(void * args);<br />
    //==============================================================================<br />
    void free_my_router(router * my_router);<br />
    //==============================================================================<br />
    
    --discribes of this functions apear in ex3.c--<br />
    
-----------------------------------------------------------------------------------------------------------------------------<br />
this program contain 3 structs:<br />

    1.communication<br />
    //==============================================================================<br />
    2.router<br />
    //==============================================================================<br />
    3.all_routers<br />
    //==============================================================================<br />
    
    
        --discribes of this structs in ex3.c--<br />

-----------------------------------------------------------------------------------------------------------------------------<br />
files in the tar:<br />
in the archive there are 2  files:<br />
1.README<br />
2.ex3.c- implement the functions and include main.<br />
3.MAKEFILE - for compile the program.<br />
--------------------------------------------------------------------------------------------------------------------------------<br />

how to compile and run:<br />
-first open the terminal in linux.<br />
-now go to where the files are.<br />
-enter make.<br />
-run the program with: './ex3<br />
