#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#define MAX_TASKS 100
#define MAX_TASK_LEN 100

// task format should be like this: [ ] task_name dueDate startDate finishDate
// [ ] - task not done
// [x] - task done

typedef struct task
{
    // not implementing an id bc i don't need it
    char task[MAX_TASK_LEN];
    bool done;
    char *finishDate;
    struct task *next;
    struct task *prev; // next and prev are for doubly linked list
} Task;                // task struct

typedef struct tasks
{
    Task *head;
    Task *tail;
} Tasks; // tasks struct
// this creates a linked list of tasks

// current->next accesses next pointer of current task
// tasks->head points to first task in list
// tasks->tail points to last task in list

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
                // if no marker, add "[ ]" to the beginning
                char temp[MAX_TASK_LEN];                      // create a temp buffer
                snprintf(temp, sizeof(temp), "[ ] %s", line); // buffer is the line with [ ] added in front
                strcpy(current->task, temp);                  // copy the temp buffer to the task struct
            }
            else
            {
                strcpy(current->task, line); // if it has a marker, copy the line to the task struct
            }

            current->done = (strncmp(line, "[x]", 3) == 0); // if it's done, set done to true
            current->finishDate = NULL;                     // set finishDate to null

            // add to tasks list
            if (tasks->head == NULL)
            {
                tasks->head = current; // if list is empty, set head to current
                tasks->tail = current; // and tail to current too
                // we do this because we want to start at the beginning
            }
            else // if list is not empty
            // this also fires after we add the first task, this is why we do the above
            {
                tasks->tail->next = current;
                tasks->tail = current;
            }
            current->next = NULL; // after all the lines are done, current is the last task
        }
    }

    // if file is empty create a default task
    void addTask(Tasks * tasks, const char *taskText); // function declaration
    if (tasks->head == NULL)
    {
        addTask(tasks, "default");
    }

    fclose(file);
}

void saveTasks(Tasks *tasks)
{
    FILE *file = fopen("tasks.txt", "w"); // open file for writing
    if (file == NULL)                     // if file is not found
    {
        printf("Error opening tasks.txt for writing\n");
        return; // exit
    }

    Task *current = tasks->head; // first task
    while (current != NULL)      // while there are tasks
    {
        fprintf(file, "%s\n", current->task); // write task + newline
        current = current->next;              // move to next task
    }

    fclose(file); // close file
}

void addTask(Tasks *tasks, const char *taskText)
{
    Task *newTask = malloc(sizeof(Task)); // allocate memory for new task
    if (newTask == NULL)                  // if memory allocation fails
    {
        printf("Memory allocation failed\n");
        return; // exit
    }

    time_t now = time(NULL);
    char timestamp[26];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now)); // year-month-day hour:minute:second

    snprintf(newTask->task, MAX_TASK_LEN, "[ ] %s (Created: %s)", taskText, timestamp);
    newTask->done = false;
    newTask->finishDate = NULL;
    newTask->next = NULL;

    if (tasks->head == NULL) // if list is empty
    {
        tasks->head = newTask;
        tasks->tail = newTask; // head and tail are both new task, first task
    }
    else // if list is not empty
    {
        tasks->tail->next = newTask; // add new task to end of list
        tasks->tail = newTask;       // update tail to new task
    }

    saveTasks(tasks);
}

void finishTask(Tasks *tasks, const char *taskName)
{
    Task *current = tasks->head; // start at head of list
    time_t now = time(NULL);
    char timestamp[26]; // YYYY-MM-DD HH:MM:SS
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    // don't need to use pointer for timestamp bc arrays decay to pointers

    while (current != NULL) // while there are tasks
    {
        char *taskContent = strstr(current->task + 4, taskName); // skip the "[ ] " prefix
        if (taskContent != NULL && !current->done)
        {
            current->done = true;
            current->finishDate = timestamp;
            // make the prefix "[x] " for the task
            char temp[MAX_TASK_LEN]; // create a temp buffer
            snprintf(temp, sizeof(temp), "[x] %s (Finished: %s)", taskContent, timestamp);
            // replace [ ] with [x], add finish time at the end
            strcpy(current->task, temp); // copy the temp buffer to the task struct
            saveTasks(tasks);
            printf("Task containing '%s' marked as finished\n", taskName);
            return;
        }
        current = current->next; // move to next task
    }

    printf("No unfinished task containing '%s' found\n", taskName); // if no task is found
    return;
}

void deleteTask(Tasks *tasks, const char *taskName)
{
    Task *current = tasks->head; // start at head of list
    Task *prev = NULL;           // previous task pointer

    while (current != NULL)
    {
        char *taskContent = strstr(current->task + 4, taskName); // skip the "[ ] " prefix
        if (taskContent != NULL)
        {
            if (prev == NULL) // if the task is the first one
            {
                // handle deletion at head
                tasks->head = current->next; // set head to next task
                if (tasks->tail == current)
                {
                    // if the task is the last one, set tail to NULL
                    tasks->tail = NULL;
                }
            }
            else
            {
                // handle deletion in middle or end
                prev->next = current->next; // set previous task's next to current task's next
                if (current == tasks->tail)
                {
                    tasks->tail = prev; // set tail to previous task
                }
            }
            free(current); // fix double free error, free memory allocated for current task
            saveTasks(tasks);
            printf("Task '%s' deleted\n", taskName);
            return;
        }
        prev = current;          // update previous task
        current = current->next; // move to next task
    }

    printf("No task containing '%s' found\n", taskName); // if no task is found
}

void showStatistics(Tasks *tasks)
{
    int total = 0, completed = 0;
    Task *current = tasks->head; // start at head of list

    while (current != NULL) // while there are tasks
    {
        total++;
        if (current->done)
            completed++;
        current = current->next; // move to next task
    }

    printf("Total tasks: %d\n", total);
    printf("Completed: %d (%.1f%%)\n", completed, (float)completed / total * 100);
    printf("Pending: %d\n", total - completed);
}

void searchTasks(Tasks *tasks, const char *keyword)
{
    Task *current = tasks->head; // start at head of list
    while (current != NULL)      // while there are tasks
    {
        if (strstr(current->task, keyword)) // if the task contains the keyword
        {
            printf("Found: %s\n", current->task); // print the task
        }
        current = current->next; // move to next task
    }
}

void filterByStatus(Tasks *tasks, bool isDone)
{
    // isDone is true if we want to filter by done tasks, false if we want to filter by pending tasks
    Task *current = tasks->head; // start at head of list
    while (current != NULL)      // while there are tasks
    {
        if (current->done == isDone) // if the task is done
        {
            printf("%s\n", current->task); // print the task
        }
        current = current->next; // move to next task
    }
}

void printTasks(Tasks *tasks)
{
    Task *current = tasks->head; // start at head of list
    while (current != NULL)      // while there are tasks
    {
        printf("%s\n", current->task); // print the task
        current = current->next;       // move to next task
    }
}

int main(int argc, char *argv[])
{
    Tasks tasks;
    tasks.head = NULL;
    tasks.tail = NULL;

    readTasks(&tasks); // read tasks from file

    if (argc > 1)
    {
        int c;
        while ((c = getopt(argc, argv, "a:f:d:s:tcp")) != -1)
        // tcp don't require arguments
        {
            switch (c)
            {
            case 'a':
                addTask(&tasks, optarg);
                printTasks(&tasks);
                break;
            case 'f':
                finishTask(&tasks, optarg);
                printTasks(&tasks);
                break;
            case 'd':
                deleteTask(&tasks, optarg);
                printTasks(&tasks);
                break;
            case 't':
                showStatistics(&tasks);
                break;
            case 's':
                searchTasks(&tasks, optarg);
                break;
            case 'c':
                filterByStatus(&tasks, true);
                break;
            case 'p':
                filterByStatus(&tasks, false);
                break;
            default:
                printf("Usage: %s [-a, -f, -d, -t, -s, -c, -p]\n", argv[0]);
                break;
            }
        }
    }
    else
    {
        printTasks(&tasks);
    }
}