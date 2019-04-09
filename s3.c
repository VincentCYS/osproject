#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	// 8 to 21
	// 19:00 to 22:00
	int NUM = 4;
	char *data[14][6] = { 
		// event name	datetime			duration	priority	A/R		index
		{ "COMP2432A1", "2019-04-18", 		"12", 		"2", 		"1", 	"1"}, 
		{ "COMP2422P1", "2019-04-20", 		"26", 		"1", 		"1", 	"2"}, 
		{ "COMP2000", 	"2019-04-14-19:00", "2", 		"3", 		"1", 	"3" },
		{ "Meeting", 	"2019-04-18-20:00", "2", 		"4", 		"0", 	"4" }
	};

	createLogFile(NUM, "S3_fcfs_01.log", "FCFS", data);
	createReportFile("lol.dat", "fcfs", data, 4, 14, 14);
}

createLogFile(int NUM_INPUT, char * fileName, char *algorithm, char *data[14][6]) {
	FILE *outputFile;
	char *eventType[30];// event type: assignment...
	char *status[15];	// store accetped or rejected
	char *strbuf[30];
	int i, j, len;

	//printf("%s\n", fileName);
	outputFile = fopen(fileName, "w");

	if (outputFile == NULL) 
	{ 
		// Error
		printf("Failed to open the file");
	} else
	{
		fprintf(outputFile, "\n\n*** Log File %s *** \n\n", algorithm);
		fprintf(outputFile, "%-5s	%-40s	%-20s\n", "ID", "Event", "Accepted/Rejected");
		fprintf(outputFile, "=================================================================\n");

		for(i = 0; i < NUM_INPUT; i++) 
		{
				// print uid
				fprintf(outputFile, "%04d	", i + 1);
				
				// priority exists?
				if (data[i][3] != NULL) 
				{
					// addProject
					if (strcmp(data[i][3], "1") == 0) 
					{
						strcpy(eventType, "addProject");
					// addAssignment
					} else if (strcmp(data[i][3], "2") == 0) 
					{
						strcpy(eventType, "addAssignment");	
					// addRevision
					} else if (strcmp(data[i][3], "3") == 0) 
					{
						strcpy(eventType, "addRevision");		
					// addActivity
					} else if (strcmp(data[i][3], "4") == 0) 
					{
						strcpy(eventType, "addActivity");
					}
				}

				// concat event data
				for (j = 0; j < 3; j++) 
				{
					// field exists?
					if (data[i][j] != NULL) 
					{
						// add space 
						sprintf(strbuf, " %s", data[i][j]);
						strcat(eventType, strbuf);
					}
				}

				// status exists?
				if (data[i][4] != NULL) 
				{
					// check accepted/rejected status
					if (strcmp(data[i][4], "1") == 0) 
					{
						strcpy(status, "Accepted");
					} else 
					{
						strcpy(status, "Rejected");
					}
				}	

				fprintf(outputFile, "%-40s 	%-20s \n", eventType, status);
		}
		fprintf(outputFile, "\n\n");
	}
	printf("Log file created!!! \n");

}

createTimeTableFile(char *fromDate, char *toDate, char *date[14], char *time[14],char *startTime, char *endTime ,char *fileName, char *algorithm, char *data[14][6], int totalTime, int totalDate) {
	FILE *outputFile;
	char *strbuf[30];
	int i, j, len;

	outputFile = fopen(fileName, "w");

	if (outputFile == NULL) 
	{ 
		// Error
		printf("Failed to open the file");
	} else
	{
		fprintf(outputFile, "\n\nAlice Timetable\n\n");
		fprintf(outputFile, "Period: %s to %s \n", fromDate, toDate);
		fprintf(outputFile, "Algorithm: %s \n\n", algorithm);
		fprintf(outputFile, "%-20s", "Date");

		// print the first line (titles and time)
		for(i = 0; i < totalTime; i++) {
			fprintf(outputFile, "%-20s", startTime);
		}

		// print actual timetable
		for(i = 0; i < totalDate; i++) {
			fprintf(outputFile, "%-20s", date[i]);
			for(j = 0; j < totalTime; j++) {
				fprintf(outputFile, "%-20s", data[i][j]);
			}
			fprintf(outputFile, "\n");
		}
	}
	printf("Timetable file created!!! \n");

}

createReportFile(char *fileName, char *algorithm, char *data[56][6], int totalTime, int totalDate, int totalRequest) {
	FILE *outputFile;
	char *strbuf[30];
	int i, j, len, totalAccept = 0, totalReject = 0, timeSlots = 0;

	outputFile = fopen(fileName, "w");

	if (outputFile == NULL) 
	{ 
		// Error
		printf("Failed to open the file");
	} else
	{
		for (i = 0; i < totalRequest; i++) {

			if (data[i][0] != NULL) {
				// accepted
				if (strcmp(data[i][4], "1") == 0) {
					timeSlots += atoi(data[i][2]);
					totalAccept++;
				//rejected
				} else if (strcmp(data[i][4],"0") == 0) {
					totalReject++;
				}
			}
		}
		float percent = 100.0f * timeSlots / (totalDate * totalTime);
		fprintf(outputFile, "\n\n***Summary Report***\n\n");
		fprintf(outputFile, "Algorithm used: %s \n\n", algorithm);
		fprintf(outputFile, "There are %d requests\n\n", totalRequest);
		fprintf(outputFile, "Number of request accepted: %d\n", totalAccept);
		fprintf(outputFile, "Number of request rejected: %d\n\n", totalReject);
		fprintf(outputFile, "Number of time slots used: %d (%.0f%%)\n\n", timeSlots, percent);
	}
	printf("Timetable file created!!! \n");

}
