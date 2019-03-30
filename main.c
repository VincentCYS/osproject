#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define INIT_CHILD 4

enum EVENT_PRIORITY {
  ADD_ASSIGNMENT,
  ADD_PROJECT,
  ADD_REVISION,
  ADD_ACTIVITY,
  UNDEFINED = -1,
};

const char *PIPE_NAME[] = { "/tmp/pipeCOMP2432ProjectGroup23PtC",
							"/tmp/pipeCOMP2432ProjectGroup23CtP" }; /* pathname of the named pipe */

int isAddedPeriod;
int eventCount;
int iChild; // Tell you are "n"th child. (0 means the first one)
int **fd; // Used for save PIPE
int childStatus[INIT_CHILD]; // Child Busy? (0-Busy, 1-Free)

char startDate[10];
char endDate[10];
char startTime[5];
char endTime[5];

int evertPriority(char *str) {
  if (!strcmp(str, "addAssignment")) {
    return ADD_ASSIGNMENT;
  } else if (!strcmp(str, "addProject")) {
    return ADD_PROJECT;
  } else if (!strcmp(str, "addRevision")) {
    return ADD_REVISION;
  } else if (!strcmp(str, "addActivity")) {
    return ADD_ACTIVITY;
  }
  return UNDEFINED;
}

int childProcessInput() {
  int i, priority;
  char charInt[1], buf[256], *value, *msg = malloc(sizeof(char)*256+1);
  read(fd[iChild][0], buf, 256);
  while (strcmp(buf, "getoff")) {
    printf("[DEBUG] Child %d received '%s'!\n", iChild+1, buf);
    value = strtok(buf, " ");
    priority = evertPriority(value);
    if (priority == ADD_ASSIGNMENT || priority == ADD_PROJECT) {
      for (i = 0; i < 3; i++) { // Name, Date & Duration
        value = strtok(NULL, " ");
        (i==0)? strcpy(msg, value):strcat(msg, value);
        strcat(msg, " ");
      }
    } else if (priority == ADD_REVISION || priority == ADD_ACTIVITY) {
      for (i = 0; i < 4; i++) { // Name, Date & Duration
        value = strtok(NULL, " ");
        (i==0)? strcpy(msg, value):strcat(msg, value);
        if (i == 1) {
          strcat(msg, "-");
        } else {
          strcat(msg, " ");
        }
      }
    } else {
      printf("Invalid! (What you give me to do arrrr?)\n");
    }
    sprintf(charInt, "%d", priority); // Priority
    strcat(msg, charInt);
    strcat(msg, " ");
    sprintf(charInt, "%d", 0); // Accept (0 or 1, default 0)
    strcat(msg, charInt);
    strcat(msg, " ");
    strcat(msg, strtok(NULL, " ")); // Event ID

    printf("[DEBUG] Child %d send '%s'!\n", iChild+1, msg);
    write(fd[i][1], msg, strlen(msg) + 1);
    read(fd[iChild][0], buf, 256);
  }
  printf("[DEBUG] Child %d exit!\n", iChild+1);
}

int childInitialize() {
  childProcessInput();
}

int parentProcessInput() {
  int i, allChildBusy;
  char eventID[12], input[2][256], *msg = malloc(sizeof(char)*256+1);
  while (1) {
    printf("Please enter:\n");
    scanf("%s", &input[0]);
    if (!strcmp(input[0], "exitS3")) {
      for (i = 0; i < INIT_CHILD; i++) {
        msg = "getoff";
        write(fd[i][0], msg, strlen(msg) + 1);
      }
      printf("[DEBUG] Parent exit!\n");
      break;
    }

    if (!isAddedPeriod) {
      if (!strcmp(input[0], "addPeriod")) {
        scanf("%s %s %s %s", &startDate, &endDate, &startTime, &endTime);
        isAddedPeriod = 1;
        printf("[DEBUG] Period created! [%s %s %s %s]\n", startDate, endDate, startTime, endTime);
      } else {
        printf("Please add period FIRST! (Tips: addPeriod [start date] [end date] [start time] [end time])\n");
      }
    } else {
      scanf("%[^\n]", input[1]);
      do {
        allChildBusy = 1;
        for (i = 0; i < INIT_CHILD; i++) {
          if (childStatus[i]) {
            printf("[DEBUG] Parent give task to child %d!\n", i+1);
            sprintf(eventID, "%d", ++eventCount);
            strcpy(msg, input[0]);
            strcat(msg, input[1]);
            strcat(msg, " ");
            strcat(msg, eventID);
            printf("[DEBUG] Parent send to child %d '%s'!\n", i+1, msg);
            write(fd[i][0], msg, strlen(msg) + 1);
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
  }
}

int parentInitialize() {
  int i;
  isAddedPeriod = 0;
  eventCount = 0;
  for (i = 0; i < INIT_CHILD; i++) {
    childStatus[i] = 1;
  }
  parentProcessInput();
}

int createPipe() {
	/* Create the named pipe */
	int i, j;
	char *name, childID[12];
	for (i = 0; i < INIT_CHILD; i++) {
		for (j = 0; j < 2; j++) {
			sprintf(childID, "%d", i);
			name = malloc((sizeof(char) + 1));
			strcpy(name, PIPE_NAME[j]);
			strcat(name, childID);
			unlink(name); // Normally, this line no need.
			              // I tested successfully without this line.
			              // But if you close this program NOT NORMAL,
						  // then this line can prevent error happened.
			if (mkfifo(name, 0600) < 0) {
				printf("Pipe creation error\n");
				exit(1);
			}
			if ((fd[i][j] = open(name, O_RDWR)) < 0) {
				printf("Pipe open error\n");
				exit(1);
			}
		}
	}
}

int closePipe() {
	/* Close the pipe */
	int i, j;
	for (i = 0; i < INIT_CHILD; i++) {
		for (j = 0; j < 2; j++) {
			close(fd[i][j]);
		}
	}
}

int removePipe() {
	/* Remove the pipe */
	int i, j;
	char *name, childID[12];
	for (i = 0; i < INIT_CHILD; i++) {
		for (j = 0; j < 2; j++) {
			sprintf(childID, "%d", i);
			name = malloc((sizeof(char) + 1));
			strcpy(name, PIPE_NAME[j]);
			strcat(name, childID);
			unlink(name);
		}
	}
}

// Create Children Process
int createChildProcessor() {
	int i;
	for (iChild = 0; iChild < INIT_CHILD;) {
		int pid = fork();
		if (pid < 0) { /* error occurred */
			printf("Fork Failed\n");
			exit(1);
		} else if (pid == 0) { /* child process */
      printf("[DEBUG] Child %d created!\n", iChild+1);
			childInitialize();
      closePipe();
			exit(0);
		} else { /* parent process */
			iChild++;
		}
	}

  parentInitialize();
	for (i = 0; i < INIT_CHILD; i++) {
		int cid = wait(NULL);
		//printf("Parent: Child %d collected\n", cid);
	}
  printf("Bye-bye!\n");
	closePipe();
	removePipe();
	exit(0);
}

// Main
int main(int argc, char *argv[]) {
  int i;
  fd = malloc(((sizeof(int) + 1)*2)*INIT_CHILD + 1);
  for (i = 0; i < INIT_CHILD; i++)
    fd[i] = malloc((sizeof(int) + 1)*2);

	createPipe();
	createChildProcessor();
	return 0;
}
