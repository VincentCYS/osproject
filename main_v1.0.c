#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INIT_CHILD 4
#define MAX_INPUT 1024
#define BUFFER_SIZE 256

enum EVENT_PRIORITY { // Priority
	PRIORITY_ADD_ASSIGNMENT,
	PRIORITY_ADD_PROJECT,
	PRIORITY_ADD_REVISION,
	PRIORITY_ADD_ACTIVITY,
	PRIORITY_UNDEFINED = -1,
};

int nRead; // For PIPE
int isAddedPeriod; // Is user input the period
int inputCount; // User input count
int receivedCount; // Parent received data count from Child
int iChild; // Tell you are "n"th child. (0 means the first one)
int childStatus[INIT_CHILD]; // Child busy? (0-Busy, 1-Free)
int fd[INIT_CHILD][2][2]; // For PIPE

char sDate[10]; // Period start date
char endDate[10]; // Period end date
char startTime[5]; // Period start time
char endTime[5]; // Period end time
char outputAlgorithm[10]; // Output algorithm
char outputFileName[64]; // Filename used to output
char input[2][BUFFER_SIZE]; // User input
char inputData[MAX_INPUT][6][BUFFER_SIZE]; // User input data including Name, Date(+Time), Duration, Priority, Accept(0 or 1, default 0), index
char inputLog[MAX_INPUT][2][BUFFER_SIZE]; // User input log

int evertPriority(char *str) {
	if (str != NULL) {
		if (!strcmp(str, "addAssignment")) {
			return PRIORITY_ADD_ASSIGNMENT;
		} else if (!strcmp(str, "addProject")) {
			return PRIORITY_ADD_PROJECT;
		} else if (!strcmp(str, "addRevision")) {
			return PRIORITY_ADD_REVISION;
		} else if (!strcmp(str, "addActivity")) {
			return PRIORITY_ADD_ACTIVITY;
		}
	}
	return PRIORITY_UNDEFINED;
}

int childProcessInput() {
	int i, priority, valid = 1;
	char *eventID, charInt[1], buf[BUFFER_SIZE], *value, msg[BUFFER_SIZE+2];
	read(fd[iChild][0][0], buf, BUFFER_SIZE);
	if (fcntl(fd[iChild][0][0], F_SETFL, O_NONBLOCK) < 0) {
		exit(2);
	}
	while (strcmp(buf, "getoff")) {
		value = strtok(buf, " ");
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
		}

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
}

int childInitialize() {
	nRead = -1;
	childProcessInput();
}

int parentReceiveData() {
	int i, j, valid, id;
	char buf[BUFFER_SIZE], *value;
	for (i = 0; i < INIT_CHILD; i++) {
		nRead = read(fd[i][1][0], buf, BUFFER_SIZE);
		switch (nRead) {
		case -1:
			if (errno == EAGAIN) {
				break;
			} else {
				perror("read");
				exit(4);
			}
		default:
			childStatus[i] = 1;
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
			strcpy(inputLog[id][1], (valid ? "1" : "0"));
			receivedCount++;
		}
	}
}

int parentPassCaseToChild(char *in) {
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

int parentProcessInput() {
	int i, c;
	char msg[BUFFER_SIZE+2];
	while (1) {
		if (nRead == 0) {
			exit(6);
		}
		printf("Please enter:\n");
		scanf("%s", &input[0]);
		if (!strcmp(input[0], "exitS3")) {
			for (i = 0; i < INIT_CHILD; i++) {
				strcpy(msg, "getoff");
				write(fd[i][0][1], msg, strlen(msg) + 1);
			}
			break;
		} else if (!isAddedPeriod) {
			if (!strcmp(input[0], "addPeriod")) {
				scanf("%s %s %s %s", &sDate, &endDate, &startTime, &endTime);
				isAddedPeriod = 1;
			} else {
				printf("Please add period FIRST! (Tips: addPeriod [start date] [end date] [start time] [end time])\n");
			}
		} else if (!strcmp(input[0], "addBatch")) {
			FILE *file = NULL;
			char *line = NULL;
			size_t len = 0;
			ssize_t lineLen;
			scanf("%s", input[1]);
			if (file = fopen(input[1], "r")) {
				while ((lineLen = getline(&line, &len, file)) != -1) {
					parentPassCaseToChild(line);
				}
				fclose(file);
			} else {
				printf("Batch file: '%s' not found!\n", input[1]);
			}
		} else if (!strcmp(input[0], "runS3")) {
			scanf("%s %s", &outputAlgorithm, &outputFileName);
			while (receivedCount < inputCount) {
				parentReceiveData();
			}
			// Pass to related algorithm function
		} else {
			if (inputCount < MAX_INPUT) {
				scanf("%[^\n]", input[1]);
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

int parentInitialize() {
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

int createPipe() {
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

int closePipe() {
	/* Close the pipe */
	int i, j, k;
	for (i = 0; i < INIT_CHILD; i++)
		for (j = 0; j < 2; j++)
			for (k = 0; k < 2; k++)
				close(fd[i][j][k]);
}

// Create Children Process
int createChildProcessor() {
	int i;
	for (iChild = 0; iChild < INIT_CHILD;) {
		int pid = fork();
		if (pid < 0) { /* error occurred */
			printf("Fork Failed\n");
			exit(3);
		} else if (pid == 0) { /* child process */
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
