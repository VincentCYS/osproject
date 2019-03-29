#include <stdio.h>
#include <stdlib.h>
 
int main()
{
	// 8 to 21
	// 19:00 to 22:00
	int NUM_INPUT = 4;
	int i, j, len;
	char *algorithm[10];// used algorithm
	char *eventType[30];// event type: assignment...
	char *status[15];	// store accetped or rejected
	char *strbuf[30];
	char *data[14][6] = { 
		// event name	datetime			duration	priority	A/R		index
		{ "COMP2432A1", "2019-04-18", 		"12", 		"2", 		"1", 	"1"}, 
		{ "COMP2422P1", "2019-04-20", 		"26", 		"1", 		"1", 	"2"}, 
		{ "COMP2000", 	"2019-04-14 19:00", "2", 		"3", 		"1", 	"3" },
		{ "Meeting", 	"2019-04-18 20:00", "2", 		"4", 		"1", 	"4" }
	};

	strcpy(algorithm, "FCFS"); // can be removed after the merged


	printf("\n\n*** Log File %s *** \n\n", algorithm);
	printf("%-5s	%-40s	%-20s\n", "ID", "Event", "Accepted/Rejected");
	printf("=================================================================\n");

	for(i = 0; i < NUM_INPUT; i++) 
	{
			// print uid
			printf("%04d	", i + 1);
			
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

			printf("%-40s 	%-20s \n", eventType, status);
	}
	printf("\n\n");



	//algorithm input:  Char *2dArray[][] = [event name, due date, duration, priority,accept(0 or 1, default 0), index ] 
}
