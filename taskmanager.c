#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

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
} Task;

typedef struct tasks
{
    Task *head;
    Task *tail;
} Tasks;

void readTasks(Tasks *tasks)
{
    FILE *file = fopen("tasks.txt", "r");
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
        exit(-1);
    }

    char line[MAX_TASK_LEN];
    Task *current = NULL;

    while (fgets(line, sizeof(line), file))
    {
        // remove newline
        line[strcspn(line, "\n")] = 0;

        if (strlen(line) > 0)
        {
            current = malloc(sizeof(Task)); // allocate memory for new task
            if (current == NULL)            // if memory allocation fails
            {
                fprintf(stderr, "Memory allocation failed\n");
                continue;
            }

            // check if task already has status marker
            if (strncmp(line, "[ ]", 3) != 0 && strncmp(line, "[x]", 3) != 0)
            {
                // add "[ ]" to the beginning
                char temp[MAX_TASK_LEN];
                snprintf(temp, sizeof(temp), "[ ] %s", line);
                strcpy(current->task, temp);
                temp[0] = '\0';
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

int main(int argc, char *argv[])
{
    Tasks tasks;
    tasks.head = NULL;
    tasks.tail = NULL;

    readTasks(&tasks);

    Task *current = tasks.head;
    while (current != NULL)
    {
        printf("%s\n", current->task);
        current = current->next;
    }

    return 0;
}