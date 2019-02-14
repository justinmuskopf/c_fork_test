#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MIN_RANDOM 1
#define MAX_RANDOM 500

#define BUFFER_LEN 4 // Max Random strlen + 1

#define NUM_FORKS 3

#define DEFAULT_FILENAME "sequence_num"

enum {OK, ERROR};

struct flock w_lock = { F_WRLCK, SEEK_SET, 0, 0, 0 };
struct flock u_lock = { F_UNLCK, SEEK_SET, 0, 0, 0 };

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

// Randomly generates an integer
// min <= n <= max
int randint(int min, int max)
{
    int offset = max - min;
    if (offset < 0)
    {
        return ERROR;
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
        return ERROR;
    }

    // File pointer's file descriptor
    int fd = fileno(fp);

    // Wait until we can acquire file lock
    fcntl(fd, F_SETLKW, &w_lock);

    char buffer[BUFFER_LEN];

    // Read number into buffer
    fread(buffer, sizeof(char), BUFFER_LEN - 1, fp);

    // Attempt to unlock file
    fcntl(fd, F_SETLK, &u_lock);

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
        return ERROR;
    }

    // File pointer's file descriptor
    int fd = fileno(fp);

    // Wait until we can acquire the lock
    fcntl(fd, F_SETLKW, &w_lock);

    // Write sequence number to file
    ret = fprintf(fp, "%d\n", sequenceNumber);
    if (ret < 0)
    {
        return ERROR;
    }

    // Attempt to unlock file
    fcntl(fd, F_SETLK, &u_lock);
    
    fclose(fp);

    return OK;
}

int generateSequenceFile(char *filename)
{
    int sequenceNumber = randint(MIN_RANDOM, MAX_RANDOM);

    if (writeSequenceToFile(filename, sequenceNumber) == -1)
    {
        printf("Failed to write sequence to file!\n");

        return ERROR;
    }

    return OK;
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
            exit(ERROR);
        }

        printf("N: %d PID: %d\n", sequenceNumber, getpid());

        ret = writeSequenceToFile(filename, sequenceNumber + 1);
        if (ret < 0)
        {
            printf("Failed to write sequence to file '%s'!\n", filename);
            exit(ERROR);
        }

        exit(OK);
    }
}


int main(int argc, char *argv[])
{
    // Seed RNG
    srand(time(NULL));

    char *filename = DEFAULT_FILENAME;

    if (argc != 2)
    {
        // Generate random sequence file
        printf("No sequence file provided, generating a random one!\n");
        generateSequenceFile(filename);
    }
    else
    {
        // Get filename from arguments
        filename = argv[1];
    }

    // Call fork for each child
    for (int i = 0; i < NUM_FORKS; i++)
    {
        doChildRoutine(filename);
    }

    // Wait on children processes to finish
    wait(NULL);

    return 0;
}
