#include <stdio.h>
#include<stdlib.h>
#include <pthread.h>

//These define statement can be any number >=0
//We will be choosing 5 for customers and 3 for resources
#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_RESOURCES 3

//The total amount of resources
int total[NUMBER_OF_RESOURCES];
//The available amount of each resource
int available[NUMBER_OF_RESOURCES];
//The max demand of each customer
int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
//The amount currently allocated to each customer
int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
//The remaining need of each customer
int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
//Hold the finished resources
int Finish[NUMBER_OF_CUSTOMERS];

//Initialize a function called release_resources
int release_resources(int customer_num);
//Initialize a function called request_resources
int request_resources(int customer_num, int request[]);
//Initialize function called threadFunction
void *threadFunction(void* customer_numt);
//Initialize function called bankersAlgorithm
int bankersAlgorithm(int customer_num,int request[]);
//Initialize function called printAll
void printAll();

//Initialize a mutex
pthread_mutex_t mutex;
//Initialize global variables called output to hold what will be displayed to the console
char output[NUMBER_OF_RESOURCES*2];
//Initialize global variable called safeSequence that holds the safe sequence that the algorithm can request in
int safeSequence[NUMBER_OF_CUSTOMERS];


int main(int argc, const char * argv[]) {
    //Initialize all matrices
    for(int x=0 ; x<argc-1; x++){
        available[x] = atoi(argv[x+1]);
        //In the beginning of the program the total resources is equal to the available
        total[x] = available[x];      
    }

    for(int x=0; x<NUMBER_OF_RESOURCES; x++){
        for(int y=0; y<NUMBER_OF_CUSTOMERS; y++){
            maximum[y][x]=rand() % (total[x]+1);    
            need[y][x]=maximum[y][x];    
        }
    }
    //Created the columns
    for (int x=0; x<NUMBER_OF_RESOURCES; x++) {
        output[x*2] = x +'A';
        output[x*2+1] = ' ';
    }

    printf("Total system resources are:\n");
    printf("%s\n", output);

    for (int x=0; x<NUMBER_OF_RESOURCES; x++) {
        printf("%d ", total[x]);
    }

    printf("\n\nProcesses (maximum resources):\n");
    printf("   %s\n",output);

    for(int i=0; i<NUMBER_OF_CUSTOMERS;i++){
        printf("P%d ",i+1);

        for(int j=0;j<NUMBER_OF_RESOURCES;j++){
            printf("%d ",maximum[i][j]);
        }
        printf("\n");
    }
    //Call printAll function
    printAll();
    //Initialize mutex
    pthread_mutex_init(&mutex, NULL);
    //Initialixe thread array   
    pthread_t attr[NUMBER_OF_CUSTOMERS];
    //Initialize array with customers
    int nums[NUMBER_OF_CUSTOMERS]={0,1,2,3,4};
    //Create all threads
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        pthread_create(&attr[i],NULL,threadFunction, &nums[i]);
    }
    //Join all threads
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        pthread_join(attr[i], NULL);     
    }
    return 0;
}

//Defining threadFunction 
void *threadFunction(void* threadCustomerNum){
    //Initializing variable which is pointer to a parse int pointer paramaeter
    int customer_num = *(int*) threadCustomerNum;
    //Initialize variable hold request_resource return value
    int requestSum = 0;
    
    //Loop till finish is not full
    while(!Finish[customer_num]){  
        //Set var back to zero
        requestSum = 0;
        //Initialize matrix to hold hold values
        int request[NUMBER_OF_RESOURCES] = {0};
        //For loop that will loop till >3 
        for(int x=0; x<NUMBER_OF_RESOURCES; x++){
            //Randomly request a need 
            request[x] = rand() % (need[customer_num][x]+1); 
            requestSum = requestSum + request[x];
        }
        //If statement to make sure zero resources are request
        if(requestSum != 0){
            //While loop till request has been granted succesfully
            while(request_resources(customer_num,request) == -1); 
        }   
    }
    return 0;
}

//Defining request_resources function
int request_resources(int customer_num, int request[]){
    //Initialize variable that will hold a return value
    int returnValue = 0;
    //Lock mutexx
    pthread_mutex_lock(&mutex);
    printf("\nP%d requests for ", customer_num+1);
    //Print all requests
    for(int x=0; x<NUMBER_OF_RESOURCES; x++){
        printf("%d ",request[x]);
    }
    printf("\n");
    //Loop through the request and see if the request is greater than avaiable resources
    for(int x=0; x<NUMBER_OF_RESOURCES; x++){   
        if(request[x] > available[x]){
            printf("P%d is waiting for the reaources...\n",customer_num+1);
            //Unlock untex
            pthread_mutex_unlock(&mutex);   
            return -1;
        }
    }
    //Set the return value to the value return by the bankers algorithm
    returnValue = bankersAlgorithm(customer_num, request);  
    
    //If the algorithm returns a zero
    if(returnValue == 0){ 
        //Initialize a variable that hold that says if need is zero
        int needIsZero = 1;
        printf("a safe sequence is found: ");
        //Print the safe sequence found in the banker algorithm
        for(int x=0; x<NUMBER_OF_CUSTOMERS; x++){
            printf("P%d ", safeSequence[x]+1);
        }

        printf("\nP%d's request has been granted\n", customer_num+1);
        //Give resources to the thread
        for(int x=0; x <NUMBER_OF_RESOURCES; x++){ 
            allocation[customer_num][x] = allocation[customer_num][x] + request[x];
            available[x] = available[x] - request[x];
            need[customer_num][x] = need[customer_num][x] - request[x];
            //Check if the need isnt zero
            if(need[customer_num][x]!=0){
                //Change value to false
                needIsZero=0;
            }
        }
        //If need is zero
        if(needIsZero){
            Finish[customer_num]=1; 
            release_resources(customer_num); 
        }
        //Print all information
        printAll();
    //If a safe sequence was found
    } else {
        printf("cannot find a safe sequence\n");
    }
    
    //Unlock mutex
    pthread_mutex_unlock(&mutex); 
    return returnValue;
}

//Defining release_resources function
int release_resources(int customer_num){
    printf("P%d releases all the resources\n", customer_num+1);
    //Release the resources
    for(int x=0; x<NUMBER_OF_RESOURCES; x++){
        available[x] = available[x] + allocation[customer_num][x]; 
        allocation[customer_num][x] = 0;
    }
    return 0;
}

//Defining the bankersAlgorithm function
int bankersAlgorithm(int customer_num,int request[]){
    //Initialize variable that banker will use to hold available resources
    int bankAvailable[NUMBER_OF_RESOURCES];
    //Initialize variable that banker will use to hold allocated resources
    int bankAllocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
    //Initialize variable that banker will use to hold needed resources
    int bankNeed[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
    //Initialize variable that banker will use to hold resources after algoritm
    int bankFinish[NUMBER_OF_CUSTOMERS] = {0};
    //Initialize variable that will be used to cycle through safe sequence
    int count = 0;
    //Copy all matrices into banker versions
    for(int x=0; x<NUMBER_OF_RESOURCES; x++){
        bankAvailable[x] = available[x];

        for(int y=0; y<NUMBER_OF_CUSTOMERS; y++){
            bankAllocation[y][x]=allocation[y][x];
            bankNeed[y][x]=need[y][x];
        }
    }
    //Pretend to give resources to the thread
    for(int x=0; x<NUMBER_OF_RESOURCES; x++){
        bankAvailable[x] = bankAvailable[x] - request[x];
        bankAllocation[customer_num][x] = bankAllocation[customer_num][x] + request[x];
        bankNeed[customer_num][x] = bankNeed[customer_num][x] - request[x];
    }

    while(1){
        //Initialize check variable in false state
        int check = -1;
        
        for(int x=0; x<NUMBER_OF_CUSTOMERS; x++){ 
            int needLTA=1;

            for(int y=0; y<NUMBER_OF_RESOURCES; y++){
                //If thread is found that has need that is less than or equal to available.
                if(bankNeed[x][y] > bankAvailable[y] || bankFinish[x] == 1){
                    needLTA=0;
                    break;
                }
            }
            //If found record its thread number
            if(needLTA){ 
                check = x;
                break;
            }
        }
        //If check didnt change
        if(check != -1){
            //Record the safe sequence
            safeSequence[count] = check; 
            count++;
            bankFinish[check] = 1; 
            //Pretend to give resources to thread
            for(int k=0;k<NUMBER_OF_RESOURCES;k++){  
                bankAvailable[k]=bankAvailable[k]+bankAllocation[check][k];
            }
        //Can not find any thread that its need is less than or equal to available.
        } else { 
            for(int i=0;i<NUMBER_OF_CUSTOMERS;i++){
                if(bankFinish[i]==0){
                    return -1;
                }
            }
            return 0; 
        }
    }
}

//Defining printAll function
void printAll(){
    printf("Processes (currently allocated resources):\n");
    printf("   %s\n",output);
    //Print all allocated resources
    for(int x=0; x<NUMBER_OF_CUSTOMERS; x++){
        printf("P%d ", x+1);

        for(int y=0; y<NUMBER_OF_RESOURCES; y++){
            printf("%d ",allocation[x][y]);
        }

        printf("\n");
    }

    printf("Processes (possibly needed resources):\n");
    printf("   %s\n", output);
    //Print all needed resources
    for(int x=0; x<NUMBER_OF_CUSTOMERS; x++){
        printf("P%d ", x+1);

        for(int y=0; y<NUMBER_OF_RESOURCES; y++){
            printf("%d ",need[x][y]);
        }
        
        printf("\n");
    }

    printf("Available system resources are:\n");
    printf("%s\n",output);
    //Print all available resources
    for (int x=0; x<NUMBER_OF_RESOURCES; x++) {
        printf("%d ", available[x]);
    }
    
    printf("\n");
}

