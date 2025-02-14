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
    struct task *prev;
} Task; // task struct

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
    FILE *file = fopen("tasks.txt", "w");
    if (file == NULL)
    {
        printf("Error opening tasks.txt for writing\n");
        return;
    }

    Task *current = tasks->head; // first task
    while (current != NULL)
    {
        fprintf(file, "%s\n", current->task); // write task + newline
        current = current->next;              // move to next task
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

void finishTask(Tasks *tasks, const char *taskName)
{
    Task *current = tasks->head;
    time_t now = time(NULL);
    char timestamp[26]; // YYYY-MM-DD HH:MM:SS
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    // don't need to use pointer bc arrays decay to pointers

    while (current != NULL)
    {
        char *taskContent = strstr(current->task + 4, taskName); // skip the "[ ] " prefix
        if (taskContent != NULL && !current->done)
        {
            current->done = true;
            current->finishDate = timestamp;
            // make the prefix "[x] " for the task
            char temp[MAX_TASK_LEN];
            snprintf(temp, sizeof(temp), "[x] %s (Finished: %s)", taskContent, timestamp);
            // replace [ ] with [x], add finish time at the end
            strcpy(current->task, temp);
            // save tasks to file
            saveTasks(tasks);
            printf("Task containing '%s' marked as finished\n", taskName);
            return;
        }
        current = current->next;
    }

    printf("No unfinished task containing '%s' found\n", taskName);
    return;
}

void deleteTask(Tasks *tasks, const char *taskName)
{
    Task *current = tasks->head;
    Task *prev = NULL;

    while (current != NULL)
    {
        char *taskContent = strstr(current->task + 4, taskName); // skip the "[ ] " prefix
        if (taskContent != NULL)
        {
            if (prev == NULL)
            {
                // handle deletion at head
                tasks->head = current->next;
                if (tasks->tail == current)
                {
                    // if the task is the last one, set tail to NULL
                    tasks->tail = NULL;
                }
            }
            else
            {
                // handle deletion in middle or end
                prev->next = current->next;
                if (current == tasks->tail)
                {
                    tasks->tail = prev;
                }
            }
            free(current); // fix double free error
            saveTasks(tasks);
            printf("Task '%s' deleted\n", taskName);
            return;
        }
        prev = current;
        current = current->next;
    }

    printf("No task containing '%s' found\n", taskName);
}

void showStatistics(Tasks *tasks)
{
    int total = 0, completed = 0;
    Task *current = tasks->head;

    while (current != NULL)
    {
        total++;
        if (current->done)
            completed++;
        current = current->next;
    }

    printf("Total tasks: %d\n", total);
    printf("Completed: %d (%.1f%%)\n", completed, (float)completed / total * 100);
    printf("Pending: %d\n", total - completed);
}

void searchTasks(Tasks *tasks, const char *keyword)
{
    Task *current = tasks->head;
    while (current != NULL)
    {
        if (strstr(current->task, keyword))
        {
            printf("Found: %s\n", current->task);
        }
        current = current->next;
    }
}

void filterByStatus(Tasks *tasks, bool isDone)
{
    Task *current = tasks->head;
    while (current != NULL)
    {
        if (current->done == isDone)
        {
            printf("%s\n", current->task);
        }
        current = current->next;
    }
}

void printTasks(Tasks *tasks)
{
    Task *current = tasks->head;
    while (current != NULL)
    {
        printf("%s\n", current->task);
        current = current->next;
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