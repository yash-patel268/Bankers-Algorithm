#include <stdio.h>
#include<stdlib.h>
#include <pthread.h>

#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_RESOURCES 3

int total[NUMBER_OF_RESOURCES];
int available[NUMBER_OF_RESOURCES];
int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

int Finish[NUMBER_OF_CUSTOMERS];

int release_resources(int customer_num);
int request_resources(int customer_num, int request[]);
void *threadFunction(void* customer_numt);
int bankersAlgorithm(int customer_num,int request[]);
void printAll();

pthread_mutex_t mutex;
char output[NUMBER_OF_RESOURCES*2];
int safeSequence[NUMBER_OF_CUSTOMERS];

int main(int argc, const char * argv[]) {
    for(int x=0 ; x<argc-1; x++){
        available[x] = atoi(argv[x+1]);
        total[x] = available[x];      
    }

    for(int x=0; x<NUMBER_OF_RESOURCES; x++){
        for(int y=0; y<NUMBER_OF_CUSTOMERS; y++){
            maximum[y][x]=rand() % (total[x]+1);    
            need[y][x]=maximum[y][x];    
        }
    }
    
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

    printAll();
        
    pthread_mutex_init(&mutex, NULL);   
    pthread_t attr[NUMBER_OF_CUSTOMERS];
    int nums[NUMBER_OF_CUSTOMERS]={0,1,2,3,4};

    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        pthread_create(&attr[i],NULL,threadFunction, &nums[i]);
    }

    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        pthread_join(attr[i], NULL);     
    }
    return 0;
}


void *threadFunction(void* threadCustomerNum){
    int customer_num = *(int*) threadCustomerNum;
    int requestSum = 0;
    
    while(!Finish[customer_num]){  
        requestSum = 0;
        int request[NUMBER_OF_RESOURCES] = {0};
        
        for(int x=0; x<NUMBER_OF_RESOURCES; x++){
            request[x] = rand() % (need[customer_num][x]+1); 
            requestSum = requestSum + request[x];
        }
        
        if(requestSum != 0){
            while(request_resources(customer_num,request) == -1); 
        }   
    }
    return 0;
}



int request_resources(int customer_num, int request[]){
    int returnValue = 0;
    pthread_mutex_lock(&mutex);
    printf("\nP%d requests for ", customer_num+1);

    for(int x=0; x<NUMBER_OF_RESOURCES; x++){
        printf("%d ",request[x]);
    }
    printf("\n");

    for(int x=0; x<NUMBER_OF_RESOURCES; x++){   
        if(request[x] > available[x]){
            printf("P%d is waiting for the reaources...\n",customer_num+1);
            pthread_mutex_unlock(&mutex);   
            return -1;
        }
    }

    returnValue = bankersAlgorithm(customer_num, request);  
    
    if(returnValue == 0){ 
        int needIsZero = 1;
        printf("a safe sequence is found: ");

        for(int x=0; x<NUMBER_OF_CUSTOMERS; x++){
            printf("P%d ", safeSequence[x]+1);
        }

        printf("\nP%d's request has been granted\n", customer_num+1);
       
        for(int x=0; x <NUMBER_OF_RESOURCES; x++){ 
            allocation[customer_num][x] = allocation[customer_num][x] + request[x];
            available[x] = available[x] - request[x];
            need[customer_num][x] = need[customer_num][x] - request[x];

            if(need[customer_num][x]!=0){
                needIsZero=0;
            }
        }
        
        if(needIsZero){
            Finish[customer_num]=1; 
            release_resources(customer_num); 
        }
        
        printAll();
        
    } else {
        printf("cannot find a safe sequence\n");
    }
   
    pthread_mutex_unlock(&mutex); 
    return returnValue;
}


int release_resources(int customer_num){
    printf("P%d releases all the resources\n", customer_num+1);

    for(int x=0; x<NUMBER_OF_RESOURCES; x++){
        available[x] = available[x] + allocation[customer_num][x]; 
        allocation[customer_num][x] = 0;
    }

    return 0;
}

int bankersAlgorithm(int customer_num,int request[]){
    int bankAvailable[NUMBER_OF_RESOURCES];
    int bankAllocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
    int bankNeed[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
    int finish[NUMBER_OF_CUSTOMERS] = {0};
    int count = 0;
    
    for(int x=0; x<NUMBER_OF_RESOURCES; x++){
        bankAvailable[x] = available[x];

        for(int y=0; y<NUMBER_OF_CUSTOMERS; y++){
            bankAllocation[y][x]=allocation[y][x];
            bankNeed[y][x]=need[y][x];
        }
    }
    
    for(int x=0; x<NUMBER_OF_RESOURCES; x++){
        bankAvailable[x] = bankAvailable[x] - request[x];
        bankAllocation[customer_num][x] = bankAllocation[customer_num][x] + request[x];
        bankNeed[customer_num][x] = bankNeed[customer_num][x] - request[x];
    }

    while(1){
        int check = -1;
        
        for(int x=0; x<NUMBER_OF_CUSTOMERS; x++){ 
            int needLTA=1;

            for(int y=0; y<NUMBER_OF_RESOURCES; y++){

                if(bankNeed[x][y] > bankAvailable[y] || finish[x] == 1){
                    needLTA=0;
                    break;
                }
            }

            if(needLTA){ 
                check = x;
                break;
            }
        }
        
        if(check != -1){
            safeSequence[count] = check; 
            count++;
            finish[check] = 1; 

            for(int k=0;k<NUMBER_OF_RESOURCES;k++){  
                bankAvailable[k]=bankAvailable[k]+bankAllocation[check][k];
            }
        } else { 
            for(int i=0;i<NUMBER_OF_CUSTOMERS;i++){
                if(finish[i]==0){
                    return -1;
                }
            }
            return 0; 
        }
    }
}


void printAll(){
    printf("Processes (currently allocated resources):\n");
    printf("   %s\n",output);
    
    for(int x=0; x<NUMBER_OF_CUSTOMERS; x++){
        printf("P%d ", x+1);

        for(int y=0; y<NUMBER_OF_RESOURCES; y++){
            printf("%d ",allocation[x][y]);
        }

        printf("\n");
    }

    printf("Processes (possibly needed resources):\n");
    printf("   %s\n", output);

    for(int x=0; x<NUMBER_OF_CUSTOMERS; x++){
        printf("P%d ", x+1);

        for(int y=0; y<NUMBER_OF_RESOURCES; y++){
            printf("%d ",need[x][y]);
        }
        
        printf("\n");
    }

    printf("Available system resources are:\n");
    printf("%s\n",output);
    
    for (int x=0; x<NUMBER_OF_RESOURCES; x++) {
        printf("%d ", available[x]);
    }
    
    printf("\n");
}

