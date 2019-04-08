
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>



#define STARTDATE "2019-03-20"
#define ENDDATE "2019-03-26"
#define STARTTIME "19:00"
#define ENDTIME "23:00"
#define DAYSINWEEK 7


//SJF 




struct InputItem{
    int year,month,day,hour,minute,duration,priority,accept,id,remainDuration,done;
    char* name;
};

struct InputItem* insert(int year,int month,int day,int hour,int minute,int duration,int priority,int accept,int id,char* name){
    struct InputItem *item = (struct InputItem*) malloc(sizeof(struct InputItem));
    item->year=year;
    item->month=month;
    item->day=day;
    item->hour=hour;
    item->minute=minute;
    item->duration=duration;
    item->priority=priority;
    item->accept=accept;
    item->id=id;
    item->name=name;
    item->remainDuration=duration;
    item->done=0;
    return item;
};

struct DayItem{
    int year,month,day,startTime,endTime,remainSlot,done;
    char** timeSlot;
};

struct DayItem* addDay(int year,int month,int day,int startTime,int endTime){
    struct DayItem *item = (struct DayItem*) malloc(sizeof(struct DayItem));
    item->year=year;
    item->day=day;
    item->month=month;
    item->startTime=startTime;
    item->endTime=endTime;
    item->done=0;
    
    item->remainSlot=endTime-startTime;
    item->timeSlot=malloc( (item->endTime-item->startTime) * sizeof(char*));
    for (int i=0; i<(item->endTime-item->startTime); i++) {
        item->timeSlot[i] = malloc(1000 * sizeof(char));
        strcpy(item->timeSlot[i],"NA");
        if(strcmp(item->timeSlot[i],"NA")!=0){
            item->remainSlot--;
        }
    }
    
    
    return item;
};




char **SJF(char* input[][6],int M,char* startDate,char* endDate, char* startTime, char* endTime) {
    char str[40];
    char delim[] = "-", delim2[]=":";
    char *ptr, *ptr2;
    struct InputItem* arr[M];
    struct DayItem* days[DAYSINWEEK];
    int year,month,day,hour,minute,duration,priority,accept,id;
    int start,end;
    char* name;
    int minDuration=-1;
    int loopFlag=0;
    
     /*-----------------------------initialize DayItem object-----------------------*/
    strcpy(str,startDate);
    ptr=strtok(str, delim);
    for (int x=0; ptr!=NULL; x++) {
//                    printf("'%d:%s'\n", x,ptr);
        switch (x) {
            case 0:
                year=atoi(ptr);break;
            case 1:
                month=atoi(ptr);break;
            case 2:
                day=atoi(ptr);break;
            default:
                break;
        }
        ptr = strtok(NULL, delim);
    }
    
    strcpy(str,startTime);
    ptr2=strtok(str, delim2);
    for (int x=0; ptr2!=NULL; x++) {
       if(x==0)start=atoi(ptr2);
        ptr2 = strtok(NULL, delim2);
    }
    
    strcpy(str,endTime);
    ptr2=strtok(str, delim2);
    for (int x=0; ptr2!=NULL; x++) {
        if(x==0)end=atoi(ptr2);
        ptr2 = strtok(NULL, delim2);
    }
    
    for (int i=0; i<DAYSINWEEK; i++) {
        days[i]=addDay(year,month,day+i,start,end);
////        printf("%d-%d-%d start:%d end:%d \n",days[i]->year,days[i]->month,days[i]->day,days[i]->startTime,days[i]->endTime);
//        for (int x=0; x<(days[i]->endTime-days[i]->startTime); x++) {
//            printf("%s\n",days[i]->timeSlot[x]);
//        }
//        printf("remainSlot:%d \n",days[i]->remainSlot);
        
    }
    
    
    
    /*-----------------------------initialize inputItem Object-----------------------*/
    for (int i=0; i<M; i++) {
        name=input[i][0];
        strcpy(str,input[i][1]);
        ptr=strtok(str, delim);
        minute=-1;
        hour=-1;
        for (int x=0; ptr!=NULL; x++) {
//            printf("'%d:%s'\n", x,ptr);
            switch (x) {
                case 0:
                    year=atoi(ptr);break;
                case 1:
                    month=atoi(ptr);break;
                case 2:
                    day=atoi(ptr);break;
                case 3:
                    ptr2=strtok(ptr, delim2);
                    for (int t=0; ptr2!=NULL; t++) {
//                        printf("'%OMG:%s'\n", t,ptr2);
                        if (t==0) {
                            hour=atoi(ptr2);
                        }else{
                            minute=atoi(ptr2);
                        }
                        ptr2 = strtok(NULL, delim2);
                    }
                    break;
                default:
                    break;
            }
            ptr = strtok(NULL, delim);
        }
        duration=atoi(input[i][2]);
        priority=atoi(input[i][3]);
        accept=atoi(input[i][4]);
        id=atoi(input[i][5]);
        
        arr[i]=insert(year,month,day,hour,minute,duration,priority,accept,id,name);     //initialize
        //        printf("%s id:%d prio:%d accept:%d \n",arr[i]->name,arr[i]->id,arr[i]->priority,arr[i]->accept);
        //        printf("%d-%d-%d, %d:%d \n",arr[i]->year,arr[i]->month,arr[i]->day,arr[i]->hour,arr[i]->minute);
    }
    
    printf("~~~~~~~~Loading plase wait~~~~~~~~~\n");

    /*-----------------------------algorithmn start-----------------------*/
   while (loopFlag!=M) {

        for (int i=0; i<M; i++) {
            
            
            //check input done?
            if (arr[i]->done==1){
                continue;
            }
            
            int index=arr[i]->day-days[0]->day;
            int time=arr[i]->hour-days[0]->startTime;
            int maxSlot=days[0]->endTime-days[0]->startTime-1;
            int freePos=-1;
            
            //check min duration and allocate sequence
            if(minDuration==-1){
                minDuration=arr[i]->duration;                 //inital first process minDuration              
            }

            if(arr[i]->duration < minDuration){              //smaller duration
                minDuration=arr[i]->duration;                 
            }

            if(arr[i]->duration > minDuration){              //larger duration countinue
                // printf("skip!\n");
                continue;               
            }


            //check priority
            switch (arr[i]->priority) {
                case 1:
                case 2:{
                    //project & assignment
                    
                   for (int x=0; x<index; x++) {
                       if (days[x]->remainSlot!=0) {
                           freePos=x;
                           arr[i]->accept=1;
                           //accept

                            int counter=0;
                            for (int p=0; p<(days[x]->endTime-days[x]->startTime); p++) {


                                if(strcmp(days[x]->timeSlot[p],"NA")==0 && arr[i]->remainDuration!=0) {
                                    strcpy(days[x]->timeSlot[p],arr[i]->name);        //write into days array
                                    arr[i]->remainDuration--;                         //update remainDuration
                                    days[x]->remainSlot--;                            //update remainSlot
                                }
                            }
                            // printf("id:%d, remainDur:%d\n",arr[i]->id,arr[i]->remainDuration);
                               
                       }
                   }
                   if(freePos==-1) arr[i]->done=1;     //reject


                    break;
                }

                case 3:
                case 4:{
                    //revision & activity
                    

                    //check accept
                    if(days[index]->remainSlot >= arr[i]->duration ){
                        int tempFlag=0;                                             //starting time pos
                        for (int d=0; d<arr[i]->duration; d++) {
                            int temp=time+d;
                            if(temp<maxSlot){
                                if(strcmp(days[index]->timeSlot[time+d],"NA")==0){
                                    tempFlag++;
                                }
                            }
                            
                        }
                        
                        if (tempFlag==arr[i]->duration) {
                            arr[i]->accept=1;           //accept
                        }else{
                            arr[i]->done=1;             //reject
                        }
                      
                        //accept
                    }else{
                        arr[i]->done=1; //reject
                    }


                    //if accept then write into days
                    if (arr[i]->accept==1){
                        for (int p=0; p<arr[i]->duration; p++) {
                            strcpy(days[index]->timeSlot[time+p],arr[i]->name);         //update slot array
                            arr[i]->remainDuration--;                                   //update remainduration
                               
                        }
                        
                        days[index]->remainSlot=days[index]->endTime-days[index]->startTime;
                        for (int q=0; q<(days[index]->endTime-days[index]->startTime); q++) {
                            if(strcmp(days[index]->timeSlot[q],"NA")!=0){
                                days[index]->remainSlot--;                              //update remainSlot
                                
                            }
                        }
                    }


                    break;

                }
            }

            // printf("%d ",arr[i]->id);
            // printf("minDur:%d duration:%d remainDur:%d accept:%d ",minDuration,arr[i]->duration,arr[i]->remainDuration,arr[i]->accept);

            //update process
            if(arr[i]->remainDuration==0){
                arr[i]->done=1;
            }

            
            // printf("done:%d \n",arr[i]->done);
           

        }

        

        minDuration++;
       //count loop flag to stop loop
       for (int i=0; i<M; i++) {
           if (arr[i]->done==1) {
               loopFlag++;
           }
       }

    //    printf("loopFlag:%d\n",loopFlag);
   }
   


   /*-------------------------------------------DEBUG STRING OUTPUT -------------------------------------------------*/
    printf("\n[DAY DEBUG:]\n");
    for (int i=0; i<DAYSINWEEK; i++) {

        printf("Day:%d remainSlot:%d\n",days[i]->day,days[i]->remainSlot);
        printf("----------------------\n");
        for (int x=0; x<(days[i]->endTime-days[i]->startTime); x++) {
            printf("%s\n",days[i]->timeSlot[x]);
        }
        printf("----------------------\n");
    }


    printf("\n[STATUS DEBUG:]\n");
    for (int i=0; i<M; i++) {
        printf("id:%d duration:%d remainDur:%d accept:%d done:%d \n",arr[i]->id,arr[i]->duration,arr[i]->remainDuration,arr[i]->accept,arr[i]->done);
    }
    
    
    
    /*------------------------------OUTPUT---------------------------*/

    int different=days[0]->endTime-days[0]->startTime;
    char ** output = malloc((DAYSINWEEK*different) * sizeof(char*));
    for(int h=0,i=0,x=0;h<DAYSINWEEK*different ;h++,x++){
             output[h] = malloc(100 * sizeof(char));
             
             if(h%different==0 && h!=0){
                x=0;
                i++;
            } 
            // printf("h:%d, i:%d, x:%d \n",h,i,x);
            strcpy(output[h],days[i]->timeSlot[x] );
            
    }

    return output;
}


int  main() {
    
    char * input[][6] = {
        //name + date (time) + duration + priority + accept + item id
        {"COMP1111Revision1","2019-03-23-19:00","2","3","0","0"},
        {"Sport1","2019-03-24-20:00","2","4","0","1"},
       {"COMP2222Assignment1","2019-03-24","4","2","0","2"},
        {"COMP3333Revision2","2019-03-22-19:00","3","3","0","3"},
         {"COMP3333Revision3","2019-03-22-19:00","1","3","0","4"},
        {"Sport2","2019-03-21-22:00","3","4","0","5"},
       {"COMP1234Project","2019-03-26","10","1","0","6"},
        {"COMP3333Revision4","2019-03-24-19:00","2","3","0","7"},
        {"COMP3333Revision5","2019-03-25-21:00","3","3","0","8"},
       {"COMP2222Assignment2","2019-03-24","4","2","0","9"}
    };
    
    
    char * input2[][6] = {
        //name + date (time) + duration + priority + accept + item id
                {"COMP2222Assignment1","2019-03-24","2","2","0","0"},
                {"COMP1234Project1","2019-03-26","6","1","0","1"},
        {"COMP3333Revision4","2019-03-24-19:00","1","3","0","2"},
                {"COMP2222Assignment2","2019-03-21","4","2","0","3"},
        {"COMP1234Project2","2019-03-24","8","1","0","4"}
    };
    
   int rows=sizeof(input) / sizeof(input[0]);
    //printf("row:%d\n",rows);
    char** str = SJF(input,rows,STARTDATE,ENDDATE,STARTTIME,ENDTIME);   //see MARCO   <-----USE this!!!!!!!
    
    //output
    for (int i=0; i<28; i++) {
         printf("%s\n", str[i]); 
    }
   
}


