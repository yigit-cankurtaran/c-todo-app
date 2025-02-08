#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#define MAX_TASKS 100
#define MAX_TASK_LEN 100

// task format should be like this: [ ] task_name dueDate startDate finishDate
// [ ] - task not done
// [x] - task done

typedef struct task
{
    char task[MAX_TASK_LEN];
    bool done;
    char *finishDate;
    struct task *next;
    struct task *prev;
} Task; // task struct

typedef struct tasks
{
    Task *head;
    Task *tail;
} Tasks; // tasks struct
// this creates a linked list of tasks

void readTasks(Tasks *tasks)
{
    FILE *file = fopen("tasks.txt", "r"); // read from tasks.txt file
    if (file == NULL)
    {
        // create file if it doesn't exist
        file = fopen("tasks.txt", "w");
        if (file == NULL)
        {
            fprintf(stderr, "Error creating tasks.txt\n");
            exit(-2);
        }
        fclose(file);
        // exit(-1);
    }

    char line[MAX_TASK_LEN]; // line buffer, we create a buffer of 100 characters
    Task *current = NULL;    // current task is null

    while (fgets(line, sizeof(line), file)) // read line by line from file
    {
        // remove newline
        line[strcspn(line, "\n")] = 0;

        if (strlen(line) > 0) // if line is not empty
        {
            current = malloc(sizeof(Task)); // allocate memory for new task
            if (current == NULL)            // if memory allocation fails
            {
                fprintf(stderr, "Memory allocation for task failed\n");
                exit(-1);
            }

            // check if task already has status marker
            char *trimmed = line;
            while (*trimmed == ' ')
                trimmed++;
            if (strncmp(trimmed, "[ ]", 3) != 0 && strncmp(trimmed, "[x]", 3) != 0)
            {
                // add "[ ]" to the beginning
                char temp[MAX_TASK_LEN];
                snprintf(temp, sizeof(temp), "[ ] %s", line);
                strcpy(current->task, temp);
            }
            else
            {
                strcpy(current->task, line);
            }

            current->done = (strncmp(line, "[x]", 3) == 0);
            current->finishDate = NULL;

            // add to tasks list
            if (tasks->head == NULL)
            {
                tasks->head = current;
                tasks->tail = current;
            }
            else
            {
                tasks->tail->next = current;
                tasks->tail = current;
            }
            current->next = NULL;
        }
    }

    fclose(file);
}

void saveTasks(Tasks *tasks)
{
    FILE *file = fopen("tasks.txt", "w");
    if (file == NULL)
    {
        printf("Error opening tasks.txt for writing\n");
        return;
    }

    Task *current = tasks->head;
    while (current != NULL)
    {
        fprintf(file, "%s\n", current->task);
        current = current->next;
    }

    fclose(file);
}

void addTask(Tasks *tasks, const char *taskText)
{
    Task *newTask = malloc(sizeof(Task));
    if (newTask == NULL)
    {
        printf("Memory allocation failed\n");
        return;
    }

    time_t now = time(NULL);
    char timestamp[26];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

    snprintf(newTask->task, MAX_TASK_LEN, "[ ] %s (Created: %s)", taskText, timestamp);
    newTask->done = false;
    newTask->finishDate = NULL;
    newTask->next = NULL;

    if (tasks->head == NULL)
    {
        tasks->head = newTask;
        tasks->tail = newTask;
    }
    else
    {
        tasks->tail->next = newTask;
        tasks->tail = newTask;
    }

    saveTasks(tasks);
}

int main(int argc, char *argv[])
{
    Tasks tasks;
    tasks.head = NULL;
    tasks.tail = NULL;

    readTasks(&tasks); // read tasks from file
    Task *current = tasks.head;
    while (current != NULL)
    {
        printf("%s\n", current->task);
        current = current->next;
    }
    if (argc > 1)
    {
        int c;
        while ((c = getopt(argc, argv, "a:")) != -1)
        {
            switch (c)
            {
            case 'a':
                addTask(&tasks, optarg);
                break;
            default:
                printf("Usage: %s [-a task]\n", argv[0]);
                break;
            }
        }
    }
}