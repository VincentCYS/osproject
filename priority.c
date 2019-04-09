#include <stdio.h>
#include <string.h>

int calTimeSlot(char startTime[5],char endTime[5]){
	return ((endTime[0]+0) *10 +  (endTime[1]+0)) - ((startTime[0]+0) *10 +  (startTime[1]+0)) -1;
}

void switchArray(char *plan[][6], int i, int j){
	char *temp;
	int k;
	for(k=0; k<6; k++) {
		strcpy(&temp, &plan[i][k]);
		strcpy(&plan[i][k],&plan[j][k] );
		strcpy(&plan[j][k], &temp);
	}
}

//calculate workload (hour)
int totalWork(char *plan[][6], int row){
	int total = 0;
	int i;
	for(i=0; i< row; i++)
		total = total + atoi(plan[i][2]);
	return total;
}

//compare date, today = today+i
//return 1 to smaller of equal to target, 0 to large
int cmpdate(char today[],char target[],int i){
	char *delim = "-";
	char * pch;
	char *s[3],*e[3];
	char temp[10];
	char temp2[10];
	stpcpy(temp,today);
	stpcpy(temp2,target);
	int j;
	pch = strtok(temp,delim);
	for(j=0;pch!=NULL;j++){
		s[j]=atoi(pch);
		if(j==2)
			s[j]=s[j]+i;
		pch = strtok (NULL, delim);
	}
	pch = strtok(temp2,delim);
	for(j=0;pch!=NULL;j++){
		e[j]=atoi(pch);
		pch = strtok (NULL, delim);
	}
	
	// printf("to:%s ta:%s day:%d\n",today,target,i);
	if(s[0]<e[0])
		return 1;
	else if(s[1]<e[1] && s[0]==e[0])
		return 1;
	else if(s[2]<=e[2] && s[1]==e[1] && s[0]==e[0]){
		// printf("month[1]=%d, [2]=%d \n",s[1],e[1]);
		return 1;
	}
	return 0;
}

int priotity( char *plan[][6],char periodStart[], int row, int timeSlotPerDay,int workload, char result[][64]){

	int i,j,k,count=0;
	int day=0;
	char temp[10];
	char temp2[10];
	stpcpy(temp,periodStart);
	for(i=0; i< row-1; i++)
		for(j=i+1; j< row; j++)
			if( plan[i][3] > plan[j][3])
				switchArray(plan,i,j);

	for(i=0; i<row; i++)
		if(  (strcmp (plan[i][3],"1") ==0) || (strcmp (plan[i][3],"2") ==0) )
			for(j=0; j< atoi(plan[i][2]); j++) {
				stpcpy(temp2,plan[i][1]);
				if( cmpdate(temp, temp2,day) == 1 ){
					stpcpy(result[count], plan[i][0]);
					count++;
					if( count%timeSlotPerDay ==0)
						day++;
				}
			}

	for(i=0; i<row; i++){
		if( (strcmp (plan[i][3],"3") ==0) || (strcmp (plan[i][3],"4") ==0) ){
			stpcpy(temp2,plan[i][1]);
			if( cmpdate(temp, temp2,day) == 1 && (timeSlotPerDay-(count%timeSlotPerDay)) > atoi(plan[i][2]) ){
				for( k=0;k< atoi(plan[i][2]);k++){
					stpcpy(result[count], plan[i][0]);
					count++;
				}
				if( count%timeSlotPerDay ==0)
					day++;
			}
			else if(cmpdate(temp, temp2,day+1) == 1 && atoi(plan[i][2]) <= timeSlotPerDay){
				// printf("temp %s, temp2 %s,day+1 %d\n",temp, temp2,day+1);
				while(count%timeSlotPerDay !=0){
					stpcpy(result[count],"NA");
					count++;
				}day++;
				
				for( k=0;k< atoi(plan[i][2]);k++){
					stpcpy(result[count], plan[i][0]);
					// printf("plan:%s  day:%d\n", plan[i][0],day);
					count++;
				
				}
				if( count%timeSlotPerDay ==0)
					day++;
			}
		}
	}



	return count;
}

main(){
	//[event name, due date, duration, priority, accept(0 or 1, default 0), index ]
	//11223344
	//assume data
	char *plan[][6] = {{"ethicP1","2019-04-11","4","1","0","0" },
			   {"OSP12222","2019-03-21","4","1","0","1" },
			   {"Chinese","2019-04-20","4","2","0","2" },
			   {"EngA2","2019-04-20","4","2","0","3" },
			   {"ELC1568R","2019-04-22","1","3","0","4" },
			   {"COMP2432R","2019-04-21","3","3","0","5" },
			   {"playing","2018-04-13","4","4","0","6" },
			   {"running","2020-04-11","3","4","0","7" },};
			  
	// char *plan[][6] = {{"ELC1568R","2019-04-22","1","3","0","4" },
			   // {"COMP2432R","2019-04-21","2","3","0","5" },
			   // {"playing","2019-04-21","5","4","0","6" },
			   // {"running","2019-04-21","2","4","0","7" }};
			   
	char periodStart = "";
	char periodEnd = "";
	char sDate[11] = "2019-04-08";
	// char endDate[10] = "2019-04-21";
	char startTime[5] = "19:00";
	char endTime[5] = "24:00";
	

	//real program
	// plan pass by reference? will change the data original
	int i;
	int row = sizeof(plan)/sizeof(plan[0]);
	
	int timeSlotPerDay = calTimeSlot(startTime,endTime);
	int workload = totalWork(plan,row);
	char result[workload+20][64];
	int count = priotity( plan, sDate, row, timeSlotPerDay, workload, &result);
	for(i=0; i<count; i++)
		printf("%s \n",result[i]);
}
