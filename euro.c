#include <stdio.h>
#include <stdlib.h>
 
int main()
{
 char temp[20];
 char gp[32];
 char *teamA[16];
 char *teamB[16];
 char tempTeamA[20];
 char tempTeamB[20];
 int tempScoreA;
 int tempScoreB;
 int scoreA[32];
 int scoreB[32];
 //for (i = 0; i < 6; i++){
int i;
int j;
int k;


for (i=0;i<16;i++) {
		teamA[i] = "x";
		teamB[i] = "x";
}


for (i=0;i<32;i++) {
		scoreA[i] = "x";
		scoreB[i] = "x";
}
	 int c = 0;
	 FILE *file;
	 file = fopen("score18Round1", "r");
	 if (file == NULL) {
		 printf("Failed to load file!");
	 } else {
	  while (fscanf(file,"%s %s %s %d %d %s", temp, gp, tempTeamA, &tempScoreA, &tempScoreB, tempTeamB) != EOF) { 
					teamA[0] = (char *)malloc(strlen()+1);
					strcpy(*teamA[0], tempTeamA);
					printf("%s %s", *teamA[0], tempTeamA);

			for (j=0; j< 16; j++) {

				if (strcmp(tempTeamA,teamA[j]) != 0) {
					printf("%s ", tempTeamA);
					scoreA[c] = tempScoreA;
					c++;
				}
				if (strcmp(tempTeamB,teamB[j]) != 0) {
					strcpy(teamB[c], tempTeamB);
					scoreB[c] = tempScoreB;
					c++;

				}
			}  
	  }
	  fclose(file);
	 }

	 for(k=0;k<15;k++) {
		 printf("%s %s vs %s %s\n", teamA[k],scoreA[k], scoreB[k] , teamB[k]);
	 }

// } 
}
