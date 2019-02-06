#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define MIN_RANDOM 1
#define MAX_RANDOM 500

#define BUFFER_LEN 4 // Max Random strlen + 1

#define NUM_FORKS 3

// Checks if a filepointer is null, and if it is,
// Alerts user and returns true
bool fileFail(FILE *fp, char *filename)
{
    if (fp == NULL)
    {
        printf("Failed to open %s!\n", filename);
        return true;
    }

    return false;
}

int randint(int min, int max)
{
    int offset = max - min;
    if (offset < 0)
    {
        return -1;
    }

    int random = (rand() % offset) + min;

    return random;
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
    int ret;

    // Open the sequence file and check for error
    FILE *fp = fopen(filename, "w");
    if (fileFail(fp, filename))
    {
        return -1;
    }

    // Write sequence number to file
    ret = fprintf(fp, "%d\n", sequenceNumber);
    if (ret < 0)
    {
        return -1;
    }

    fclose(fp);

    return 0;
}

// Performs the child routine
// 1. Gets sequence number from file
// 2. Creates child process
// 3. Writes sequence number to sequence file
void doChildRoutine(char *filename)
{
    // Child process
    if (fork() == 0)
    {
        int ret;
        int sequenceNumber;

        sequenceNumber = getSequenceFromFile(filename);
        if (sequenceNumber <= 0)
        {
            printf("Could not retrieve sequence number from file '%s'! Exiting child process.\n", filename);
            exit(1);
        }

        printf("N: %d PID: %d\n", sequenceNumber, getpid());

        ret = writeSequenceToFile(filename, sequenceNumber + 1);
        if (ret < 0)
        {
            printf("Failed to write sequence to file '%s'!\n", filename);
        }

        exit(0);
    }
}


int main(int argc, char *argv[])
{
    // Check for proper execution
    if (argc != 2)
    {
        printf("USAGE: %s filename\n", argv[0]);
        exit(1);
    }

    // Get filename from arguments
    char *filename = argv[1];

    // Seed RNG
    srand(time(NULL));

    // Call fork for each child
    for (int i = 0; i < NUM_FORKS; i++)
    {
        doChildRoutine(filename);
    }

    // Wait on children processes to finish
    wait(NULL);

    return 0;
}
