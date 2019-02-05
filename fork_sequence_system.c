#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define MIN_RANDOM 1
#define MAX_RANDOM 500

#define BUFFER_LEN 4 // Max Random strlen + 1

#define SEQUENCE_FILE "sequence.txt"

#define NUM_FORKS 3

// Returns a random integer n, min <= n <= max
int getRandom(int min, int max)
{
    int offset = max - min;

    int random = (rand() % offset) + min;

    return random;
}

// Checks if a filepointer is null, and if it is,
// Alerts user and returns true
bool fileFail(FILE *fp, char *filename)
{
    if (fp == NULL)
    {
        printf("Failed to open %s!\n", SEQUENCE_FILE);
        return true;
    }

    return false;
}

// Returns the sequence number from a file, or -1 on error
int getSequenceFromFile(char *filename)
{
    // Open the sequence file and check for error
    FILE *fp = fopen(filename, "r");
    if (fileFail(fp, filename))
    {
        return -1;
    }

    char buffer[BUFFER_LEN];

    // Read number into buffer
    fread(buffer, sizeof(char), BUFFER_LEN - 1, fp);

    fclose(fp);

    return atoi(buffer);
}

// Writes a sequence number to the sequence file
int writeSequenceToFile(char *filename, int sequenceNumber)
{
    // Open the sequence file and check for error
    FILE *fp = fopen(filename, "w");
    if (fileFail(fp, filename))
    {
        return -1;
    }

    // Write sequence number to file
    fprintf(fp, "%d\n", sequenceNumber);

    fclose(fp);

    return 0;
}

// Performs the child routine
// 1. Gets sequence number based on childIdx
// 2. Creates child process
// 3. Writes sequence number to sequence file
void doChildRoutine(int childIdx)
{
    // Assign sequence number based on index,
    // If 0, then this is the first child
    int sequenceNumber;
    if (childIdx == 0)
    {
        sequenceNumber = getRandom(MIN_RANDOM, MAX_RANDOM);
    }
    else
    {
        sequenceNumber = getSequenceFromFile(SEQUENCE_FILE);
    }

    // Child process
    if (fork() == 0)
    {
        printf("N: %d PID: %d\n", sequenceNumber, getpid());

        writeSequenceToFile(SEQUENCE_FILE, sequenceNumber + 1);

        exit(0);
    }
}


int main(void)
{
    // Seed RNG
    srand(time(NULL));

    // True for the first child; tells it to get a random number
    bool firstChild = true;

    // Call fork for each child
    for (int i = 0; i < NUM_FORKS; i++)
    {
        doChildRoutine(i);
    }

    // Wait on children processes to finish
    wait(NULL);

    return 0;
}
