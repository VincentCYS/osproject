#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INIT_CHILD 4
#define MAX_INPUT 1024
#define BUFFER_SIZE 2048

enum EVENT_PRIORITY { // Priority
	PRIORITY_ADD_PROJECT,
	PRIORITY_ADD_ASSIGNMENT,
	PRIORITY_ADD_REVISION,
	PRIORITY_ADD_ACTIVITY,
	PRIORITY_UNDEFINED = -1,
};

enum ALGORITHM { // Algorithm
	ALGORITHM_PRIORITY,
	ALGORITHM_SJF,
	ALGORITHM_UNDEFINED = -1,
};

int nRead; // For PIPE
int isAddedPeriod; // Is user input the period
int inputCount; // User input count
int receivedCount; // Parent received data count from Child
int iChild; // Tell you are "n"th child. (0 means the first one)
int childStatus[INIT_CHILD]; // Child busy? (0-Busy, 1-Free)
int fd[INIT_CHILD][2][2]; // For PIPE

char startDate[10+1]; // Period start date
char endDate[10+1]; // Period end date
char startTime[5+1]; // Period start time
char endTime[5+1]; // Period end time
char outputAlgorithm[10+1]; // Output algorithm
char outputFileName[64+1]; // Filename used to output
char input[2][BUFFER_SIZE]; // User input
char inputData[MAX_INPUT][6][BUFFER_SIZE]; // User input data including Name, Date(+Time), Duration, Priority, Accept(0 or 1, default 0), index
char inputLog[MAX_INPUT][2][BUFFER_SIZE]; // User input log
//char *algorithmResult;

int evertPriority(char *str) {
	if (str != NULL) {
		if (!strcmp(str, "addProject")) {
			return PRIORITY_ADD_PROJECT;
		} else if (!strcmp(str, "addAssignment")) {
			return PRIORITY_ADD_ASSIGNMENT;
		} else if (!strcmp(str, "addRevision")) {
			return PRIORITY_ADD_REVISION;
		} else if (!strcmp(str, "addActivity")) {
			return PRIORITY_ADD_ACTIVITY;
		}
	}
	return PRIORITY_UNDEFINED;
}

int algorithm(char *str) {
	if (str != NULL) {
		if (!strcmp(str, "PR")) {
			return ALGORITHM_PRIORITY;
		} else if (!strcmp(str, "SJF")) {
			return ALGORITHM_SJF;
		}
	}
	return ALGORITHM_UNDEFINED;
}

int calTimeSlot(){
	return ((endTime[0]+0) *10 +  (endTime[1]+0)) - ((startTime[0]+0) *10 +  (startTime[1]+0)) -1;
}

void switchArray(int i, int j){
	char temp[256];
	int k;
	for(k=0; k<6; k++) {
		strcpy(temp, inputData[i][k]);
		strcpy(inputData[i][k], inputData[j][k] );
		strcpy(inputData[j][k], temp);
	}
}

//calculate workload (hour)
int totalWork(){
	int total = 0;
	int i;
	for(i=0; i < inputCount; i++)
		total = total + atoi(inputData[i][2]);
	return total;
}

//compare date, today = today+i
//return 1 to smaller of equal to target, 0 to large
int cmpdate(char today[], char target[], int i){
	char *delim = "-";
	char *pch;
	int s[3],e[3];
	char temp[10];
	char temp2[10];
	stpcpy(temp,today);
	stpcpy(temp2,target);
	int j;
	pch = strtok(temp,delim);
	for(j=0; pch!=NULL; j++) {
		s[j]=atoi(pch);
		if(j==2)
			s[j]=s[j]+i;
		pch = strtok (NULL, delim);
	}
	pch = strtok(temp2,delim);
	for(j=0; pch!=NULL; j++) {
		e[j]=atoi(pch);
		pch = strtok (NULL, delim);
	}

	if(s[0]<e[0])
		return 1;
	else if(s[1]<e[1] && s[0]==e[0])
		return 1;
	else if(s[2]<=e[2] && s[1]==e[1] && s[0]==e[0])
		return 1;
	return 0;
}

int priotity(int timeSlotPerDay, int workload, char result[][64]){

	int i,j,k,count=0;
	int day=0;
	char temp[10];
	char temp2[10];
	stpcpy(temp,startDate);
	for(i=0; i< inputCount-1; i++)
		for(j=i+1; j< inputCount; j++)
			if( inputData[i][3] > inputData[j][3])
				switchArray(i, j);


	for(i=0; i<inputCount; i++)
		if(  (strcmp (inputData[i][3],"1") ==0) || (strcmp (inputData[i][3],"2") ==0) )
			for(j=0; j< atoi(inputData[i][2]); j++) {
				stpcpy(temp2,inputData[i][1]);
				if( cmpdate(temp, temp2,day) == 1 ) {
					stpcpy(result[count], inputData[i][0]);
					count++;
					if( count%timeSlotPerDay ==0)
						day++;
				}
			}


	for(i=0; i<inputCount; i++) {
		if( (strcmp (inputData[i][3],"3") ==0) || (strcmp (inputData[i][3],"4") ==0) ) {
			stpcpy(temp2,inputData[i][1]);
			if( cmpdate(temp, temp2,day) == 1 && (timeSlotPerDay-(count%timeSlotPerDay)) >= atoi(inputData[i][2]) ) {
				for( k=0; k< atoi(inputData[i][2]); k++) {
					stpcpy(result[count], inputData[i][0]);
					count++;
					if( count%timeSlotPerDay ==0)
						day++;
				}
			}
			else if(cmpdate(temp, temp2,day+1) == 1 && atoi(inputData[i][2]) <= timeSlotPerDay) {
				while(count%timeSlotPerDay !=0) {
					stpcpy(result[count],"NA");
					count++;
				} day++;

				for( k=0; k< atoi(inputData[i][2]); k++) {
					stpcpy(result[count], inputData[i][0]);
					count++;
					if( count%timeSlotPerDay ==0)
						day++;

				}
			}
		}
	}



	return count;
}

void priorityAlgorithm() {
	int i;
	int timeSlotPerDay = calTimeSlot();
	int workload = totalWork();
	char result[workload+20][64];
	int count = priotity(timeSlotPerDay, workload, &result);
	for(i = 0; i < count; i++)
		printf("%s \n", result[i]);
}

void childProcessInput() {
	int i, j, priority, algorithmUsed, valid = 1;
	char *eventID, charInt[1], buf[BUFFER_SIZE], *value, msg[BUFFER_SIZE+2];
	read(fd[iChild][0][0], buf, BUFFER_SIZE);
	if (fcntl(fd[iChild][0][0], F_SETFL, O_NONBLOCK) < 0) {
		exit(2);
	}
	while (strcmp(buf, "getoff")) {
		printf("------------------------------[DEBUG] Child %d: I received '%s'!\n", iChild+1, buf);
		value = strtok(buf, " ");
		if (atoi(value) >= 0) {
			eventID = strdup(value);
			strcpy(msg, eventID); // Event ID
			strcat(msg, "-");

			value = strtok(NULL, " ");
			priority = evertPriority(value);
			if (priority == PRIORITY_ADD_ASSIGNMENT || priority == PRIORITY_ADD_PROJECT) {
				for (i = 0; i < 3; i++) { // Name, Date & Duration
					value = strtok(NULL, " ");
					if (!value) {
						valid = 0;
						break;
					}
					strcat(msg, value);
					strcat(msg, " ");
				}
			} else if (priority == PRIORITY_ADD_REVISION || priority == PRIORITY_ADD_ACTIVITY) {
				for (i = 0; i < 4; i++) { // Name, Date & Duration
					value = strtok(NULL, " ");
					if (!value) {
						valid = 0;
						break;
					}
					strcat(msg, value);
					if (i == 1) {
						strcat(msg, "-");
					} else {
						strcat(msg, " ");
					}
				}
			} else valid = 0;
			if (valid) {
				sprintf(charInt, "%d", priority); // Priority
				strcat(msg, charInt);
				strcat(msg, " ");
				sprintf(charInt, "%d", 0); // Accept (0 or 1, default 0)
				strcat(msg, charInt);
			} else {
				strcpy(msg, eventID); // Event ID
				strcat(msg, "-");
				strcat(msg, "INVALID");
				printf("------------------------------[DEBUG] Child %d: I received an invalid input!\n", iChild+1);
			}
		} else {
			value = strtok(NULL, " ");
			inputCount = atoi(value);
			value = strtok(NULL, " ");
			strcpy(startDate, value);
			value = strtok(NULL, " ");
			strcpy(endDate, value);
			value = strtok(NULL, " ");
			strcpy(startTime, value);
			value = strtok(NULL, " ");
			strcpy(endTime, value);
			value = strtok(NULL, " ");
			algorithmUsed = algorithm(value);
			value = strtok(NULL, " ");
			strcpy(outputFileName, value);

			for (i = 0; i < inputCount; i++) {
				for (j = 0; j < 6; j++) {
					value = strtok(NULL, " ");
					if (!value) {
						valid = 0;
						break;
					}
					strcpy(inputData[i][j], value);
				}
				if (!value) {
					valid = 0;
					break;
				}
			}

			if (algorithmUsed == ALGORITHM_PRIORITY) {
				priorityAlgorithm();
			} else if (algorithmUsed == ALGORITHM_SJF) {

			} else valid = 0;

			if (valid) {
				// Output file function
				strcpy(msg, "FINISH");
			} else {
				strcpy(msg, "OUTPUT-INVALID");
				printf("------------------------------[DEBUG] Child %d: I received an invalid output!\n", iChild+1);
			}
		}

		printf("------------------------------[DEBUG] Child %d: I send '%s'!\n", iChild+1, msg);
		write(fd[iChild][1][1], msg, strlen(msg) + 1);
		while ((nRead = read(fd[iChild][0][0], buf, BUFFER_SIZE)) == -1) {
			if (errno == EAGAIN) {
				sleep(1);
			} else {
				perror("read");
				exit(5);
			}
		}
		if (nRead == 0) {
			printf("Hey! What happened? My parent close the PIPE!! [Received by Child %d]\n", iChild+1);
		}
	}
	printf("------------------------------[DEBUG] Child %d: I exit!\n", iChild+1);
}

void childInitialize() {
	nRead = -1;
	childProcessInput();
}

void parentReceiveData() {
	int i, j, valid, id;
	char buf[BUFFER_SIZE], *value;
	for (i = 0; i < INIT_CHILD; i++) {
		nRead = read(fd[i][1][0], buf, BUFFER_SIZE);
		printf("------------------------------[DEBUG] Parent: Start to receive Child %d.\n", i+1);
		switch (nRead) {
		case -1:
			if (errno == EAGAIN) {
				printf("------------------------------[DEBUG] Parent: Child %d PIPE Empty.\n", i+1);
				break;
			} else {
				perror("read");
				exit(4);
			}
		default:
			printf("------------------------------[DEBUG] Parent: Received Child %d: '%s'\n", i+1, buf);
			childStatus[i] = 1;
			if (strcmp(buf, "FINISH") && strcmp(buf, "OUTPUT-INVALID")) {
				valid = (strstr(buf, " ") ? 1 : 0);
				value = strtok(buf, "-");
				id = atoi(value);
				if (valid) { // Valid input
					strcpy(inputData[id][5], value);
					for (j = 0; j < 5; j++) {
						value = strtok(NULL, " "); // Split data by space
						strcpy(inputData[id][j], value); // Put data into array
					}
				}
				printf("------------------------------[DEBUG] Parent: Added data '%s %s %s %s %s %s'\n", inputData[id][0], inputData[id][1], inputData[id][2], inputData[id][3], inputData[id][4], inputData[id][5]);
				printf("------------------------------[DEBUG] Parent: Data log '%s'\n", inputLog[id][0]);
				strcpy(inputLog[id][1], (valid ? "1" : "0"));
				receivedCount++;
			}
		}
	}
}

void parentPassOutputToChild(char *in) {
	int i, j, k, allChildBusy;
	char strInputCount[12], msg[BUFFER_SIZE+2];
	do {
		parentReceiveData();
		allChildBusy = 1;
		for (i = 0; i < INIT_CHILD; i++) {
			if (childStatus[i]) {
				sprintf(strInputCount, "%d", receivedCount);
				strcpy(msg, "-1 ");
				strcat(msg, strInputCount);
				strcat(msg, " ");
				strcat(msg, startDate);
				strcat(msg, " ");
				strcat(msg, endDate);
				strcat(msg, " ");
				strcat(msg, startTime);
				strcat(msg, " ");
				strcat(msg, endTime);
				strcat(msg, in);
				printf("%s\n",msg);
				for (j = 0; j < receivedCount; j++) {
					for (k = 0; k < 6; k++) {
						strcat(msg, " ");
						strcat(msg, inputData[j][k]);
						printf("%s\n",msg);
					}
				}
				printf("------------------------------[DEBUG] Parent: I send to child %d 'RunS3 %s'!\n", i+1, in);
				write(fd[i][0][1], msg, strlen(msg) + 1);
				childStatus[i] = 0;
				allChildBusy = 0;
				break;
			}
		}
		if (allChildBusy) {
			printf("Children are busy, please hang on a moment...\n");
			sleep(1);
		}
	} while(allChildBusy);
}

void parentPassCaseToChild(char *in) {
	int i, allChildBusy;
	char eventID[12], msg[BUFFER_SIZE+2];
	do {
		parentReceiveData();
		allChildBusy = 1;
		for (i = 0; i < INIT_CHILD; i++) {
			if (childStatus[i]) {
				sprintf(eventID, "%d", inputCount);
				strcpy(msg, eventID);
				strcat(msg, " ");
				strcat(msg, in);
				printf("------------------------------[DEBUG] Parent: I send to child %d '%s'!\n", i+1, msg);
				strcpy(inputLog[inputCount++][0], msg);
				write(fd[i][0][1], msg, strlen(msg) + 1);
				childStatus[i] = 0;
				allChildBusy = 0;
				break;
			}
		}
		if (allChildBusy) {
			printf("Children are busy, please hang on a moment...\n");
			sleep(1);
		}
	} while(allChildBusy);
}

void parentProcessInput() {
	int i, c;
	char *pos;
	char msg[BUFFER_SIZE+2];
	while (1) {
		if (nRead == 0) {
			printf("------------------------------[ERROR] Parent: Child close the conversation.\n");
			exit(6);
		}
		printf("Please enter:\n");
		scanf("%s", &input[0]);
		if (!strcmp(input[0], "exitS3")) {
			for (i = 0; i < INIT_CHILD; i++) {
				strcpy(msg, "getoff");
				write(fd[i][0][1], msg, strlen(msg) + 1);
			}
			printf("------------------------------[DEBUG] Parent: I exit!\n");
			break;
		} else if (!isAddedPeriod) {
			if (!strcmp(input[0], "addPeriod")) {
				scanf("%s %s %s %s", &startDate, &endDate, &startTime, &endTime);

				if ((pos = strchr(startDate, '\n')) != NULL)
					*pos = '\0';
				if ((pos = strchr(endDate, '\n')) != NULL)
					*pos = '\0';
				if ((pos = strchr(startTime, '\n')) != NULL)
					*pos = '\0';
				if ((pos = strchr(endTime, '\n')) != NULL)
					*pos = '\0';
				//algorithmResult = malloc(sizeof(char)*20+1);
				isAddedPeriod = 1;
				printf("------------------------------[DEBUG] Parent: Period created! [%s %s %s %s]\n", startDate, endDate, startTime, endTime);
			} else {
				printf("Please add period FIRST! (Tips: addPeriod [start date] [end date] [start time] [end time])\n");
			}
		} else if (!strcmp(input[0], "addBatch")) {
			FILE *file = NULL;
			char *line = NULL;
			size_t len = 0;
			ssize_t lineLen;
			scanf("%s", input[1]);
			if ((pos = strchr(input[1], '\n')) != NULL)
				*pos = '\0';
			if (file = fopen(input[1], "r")) {
				while ((lineLen = getline(&line, &len, file)) != -1) {
					parentPassCaseToChild(line);
				}
				fclose(file);
			} else {
				printf("Batch file: '%s' not found!\n", input[1]);
			}
		} else if (!strcmp(input[0], "runS3")) {
			scanf("%[^\n]", input[1]);
			//input[1][0] = '\0';
			//scanf("%s %s", &outputAlgorithm, &outputFileName);
			if ((pos = strchr(input[1], '\n')) != NULL)
				*pos = '\0';
			while (receivedCount < inputCount) {
				parentReceiveData();
			}
			printf("------------------------------[DEBUG] Parent: RunS3 [%s]\n", input[1]);
			strcpy(msg, input[1]);
			parentPassOutputToChild(msg);
			// Pass to related algorithm function
		} else {
			if (inputCount < MAX_INPUT) {
				scanf("%[^\n]", input[1]);
				if ((pos = strchr(input[1], '\n')) != NULL)
					*pos = '\0';
				strcpy(msg, input[0]);
				strcat(msg, input[1]);
				parentPassCaseToChild(msg);
			} else {
				printf("Input size fulled\n");
			}
		}
		while ((c = getchar()) != '\n' && c != EOF) { }
	}
}

void parentInitialize() {
	int i;
	nRead = -1;
	isAddedPeriod = 0;
	inputCount = 0;
	receivedCount = 0;
	for (i = 0; i < INIT_CHILD; i++) {
		childStatus[i] = 1;
	}
	parentProcessInput();
}

void createPipe() {
	/* Create the named pipe */
	int i, j;
	char childID[12];
	for (i = 0; i < INIT_CHILD; i++) {
		for (j = 0; j < 2; j++) {
			if (pipe(fd[i][j]) == -1) {
				fprintf(stderr, "Pipe Failed" );
				exit(1);
			}
			if (j == 1 && fcntl(fd[i][j][0], F_SETFL, O_NONBLOCK) < 0) {
				exit(2);
			}
		}
	}
}

void closePipe() {
	/* Close the pipe */
	int i, j, k;
	for (i = 0; i < INIT_CHILD; i++)
		for (j = 0; j < 2; j++)
			for (k = 0; k < 2; k++)
				close(fd[i][j][k]);
}

// Create Children Process
void createChildProcessor() {
	int i;
	for (iChild = 0; iChild < INIT_CHILD;) {
		int pid = fork();
		if (pid < 0) { /* error occurred */
			printf("Fork Failed\n");
			exit(3);
		} else if (pid == 0) { /* child process */
			printf("------------------------------[DEBUG] Child %d: I was born!\n", iChild+1);
			childInitialize();
			closePipe();
			exit(0);
		} else { /* parent process */
			iChild++;
		}
	}

	printf("~~WELCOME TO S3~~\n");
	parentInitialize();
	for (i = 0; i < INIT_CHILD; i++) {
		int cid = wait(NULL);
		//printf("Parent: Child %d collected\n", cid);
	}
	printf("Bye-bye!\n");
	closePipe();
	exit(0);
}

// Main
int main(int argc, char *argv[]) {
	createPipe();
	createChildProcessor();
	return 0;
}
