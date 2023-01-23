#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#define MAX_USER_INPUT 128

char* get_fifo_file_name(char *process_name_to_find, char *fifo_name) {
    
    int success_find_process_name = 0;
    char line[20], c; 
    char process_name_in_config[4];
    int config_file;
    if((config_file = open("config.txt", O_RDONLY)) < 0) perror("Failed to open configuration file!");
    int k = 0;
    while(read(config_file, &c, 1) == 1) {
        line[k++] = c;
        if(c =='\n') {
            k = 0;
            printf("%s", line);
            for(int i = 0; line[i] != '\0'; i++) {
                if(line[i] == ' ') {
                    strncpy(process_name_in_config, line, 4);
                    process_name_in_config[4] = 0; 

                    if(!strcmp(process_name_to_find, process_name_in_config)) {
                        success_find_process_name = 1;
                        
                        int position = 0;
                        for(int j = i+1; sizeof(line); j++) {
                            if(line[j] == '\0') {
                                break;
                            }
                            if(line[j] == '\n') {
                                break;
                            }
                            
                            fifo_name[position] = line[j];
                            position++;
                        }
                        fifo_name[9] = 0; // ??
                        
                    } 
                } 
            }
        }
    }
        if(!success_find_process_name) {
            printf("Process name don't exist in configuration file!\n");
            exit(EXIT_FAILURE);
        }
    
    memset(line, 0, sizeof line);
    close(config_file);
    return fifo_name;
}

char* get_parameters_from_user_input(char *input_from_user, char *parameter, int parameter_code) {
    
    static char executive_process_name[5];
    static char command[128];
    static char executive_fifo_name[64];
    int command_indexes[2], fifo_queque_execution_name_indexes[2], quote_number = 0;
    for(int i = 0; input_from_user[i] != '\0'; i++) {
        //get executiove process name
        if(i < 5) {
            executive_process_name[i] = input_from_user[i];
            if(i == 4) {
                executive_process_name[4] = 0;
            }
        }
        //get command
        if(input_from_user[i] == '"') {
            if(quote_number >= 4) {
                printf("Inavalid input. Use pattern: process_name \"command\" \"fifo_result_name\"");
            }
            if(quote_number == 0) {
                command_indexes[0] = i;
                quote_number++;
                continue;
            }
            if(quote_number == 1) {
                command_indexes[1] = i;
                quote_number++;
                continue;
            }
            if(quote_number == 2) {
                fifo_queque_execution_name_indexes[0] = i+1;
                quote_number++;
                continue;
            }
            if(quote_number == 3) {
                fifo_queque_execution_name_indexes[1] = i-1;
                break;
            }
        }
    }
    char exit_command[4] = "exit";
    static char empty[] = "";
    if (quote_number < 3) {
        if(!strcmp(executive_process_name, exit_command)) {
            return executive_process_name;
        }
        printf("Wrong input, try again!\n");
        return empty;
    }
    int k = 0;
    for(int j = command_indexes[0]+1 ; j < command_indexes[1]; j++) {
        command[k] = input_from_user[j];
        k++;
    }
    command[k] = 0;
    k = 0;
    for(int j = fifo_queque_execution_name_indexes[0]; j <= fifo_queque_execution_name_indexes[1]; j++) {
        executive_fifo_name[k] = input_from_user[j];
        k++;
    }
    executive_fifo_name[k] = 0;

    if(parameter_code == 0) {
        return executive_process_name;
    }
    if(parameter_code == 1) {
        return command;
    }
   if(parameter_code == 2) {
        return executive_fifo_name;
    }
    
}



int main (int argc, char *argv[]) {
    
    if (argc < 1) {
        printf("Use user process name as parameter! For example: ./a.out usr0 \n");
    }
    char fifo_name[10]; 
    char *main_process_fifo_name = get_fifo_file_name(argv[1], fifo_name);
    char constant_process_name[10];
    strcpy(constant_process_name, main_process_fifo_name);

    if(mkfifo(main_process_fifo_name, 0640) < 0) {
        printf("Queue already exists, try again!\n");
        exit(EXIT_FAILURE);
    }
    int forked_process = fork();
    while(1) {
        
        if(forked_process != 0) {
            printf("Enter command according to the pattern: \nprocess_name_to_execute_comand \"command\" \"fifo_queue_name_to_send_result\"\n> ");
            char user_input[MAX_USER_INPUT];
            fgets(user_input, MAX_USER_INPUT, stdin);
            //setting change form \n to \0 on last char
            if ((strlen(user_input) > 0) && (user_input[strlen (user_input) - 1] == '\n'))
                    user_input[strlen (user_input) - 1] = '\0';

            int command_indexes_in_input[2], result_fifo_name_indexes_in_input[2], quotion_mark_nummber = 0;
            char parameter[64];

            char *process_to_execute_command_name = get_parameters_from_user_input(user_input, parameter, 0);
            char exit_command[] = "exit";
            char empty[] = "";
            if(!strcmp(process_to_execute_command_name, exit_command)) {
                printf("Goodbye :)\n");
                break;
            }
            if(!strcmp(process_to_execute_command_name, empty)) {
                continue;
            }

            char *command = get_parameters_from_user_input(user_input, parameter, 1);
            char *execute_command_fifo_name = get_parameters_from_user_input(user_input, parameter, 2);

            if(mkfifo(execute_command_fifo_name, 0640) < 0) {
                perror("Failed to create fifo queue");
                continue;
            }
            char *process_to_execute_command_own_fifo = get_fifo_file_name(process_to_execute_command_name, fifo_name);
            printf("Process fifo name to send command: %s\n", process_to_execute_command_own_fifo);

            int queue = open(process_to_execute_command_own_fifo, O_WRONLY);
            if( queue < 0) {
                perror("Failed open executive process FIFO queue, try again\n");
                unlink(execute_command_fifo_name);
                continue;
            }
            int write_result = write(queue, execute_command_fifo_name, 128);
            if(write_result < 0) {
                perror("Failed to send name of result fifo queue, try again\n");
                unlink(execute_command_fifo_name);
                continue;
            }
            // memset(execute_command_fifo_name, 0, sizeof execute_command_fifo_name);
            sleep(1);
            write_result = write(queue, command, 128);
            if(write_result < 0) {
                printf("Failed to send execution command, try again!\n");
                continue;
            }
            close(queue);

            char result_buffer[256];
            int command_result = open(execute_command_fifo_name, O_RDONLY);
            int read_result = read(command_result, result_buffer, sizeof result_buffer);
            printf("\nResult:\n %s\n", result_buffer);
            memset(result_buffer, 0, sizeof result_buffer);
            close(command_result);

            if(read_result < 0) {
                printf("Something goes wrong with reading command result, please try again later (when bug will be fixed)\n");
                printf("if the error persists, please contact us: bestAnsiCDeveloper@gmail.com\n");
                exit(EXIT_FAILURE);
            }

            if(unlink(execute_command_fifo_name) != 0) perror("Failed to delete FIFO file!");


        } else {
            int process_fifo = open(main_process_fifo_name, O_RDONLY);
            if(process_fifo < 0) {
                perror("Failed to open fifo queue!");
                exit(EXIT_FAILURE);
            }
            char fifo_to_send[128];
            read(process_fifo, fifo_to_send, sizeof fifo_to_send);
            printf("Fifo name to send result: %s\n", fifo_to_send);

            char command_to_execute[128];
            read(process_fifo, command_to_execute, sizeof command_to_execute);
            close(process_fifo);
            printf("Command to execute: %s\n", command_to_execute);
            
            close(1);
            int result_fifo = open(fifo_to_send, O_WRONLY);
            if(fork() == 0) {
                execl("/bin/sh", "/bin/sh", "-c", command_to_execute, NULL);
            } else {
                close(result_fifo);
                dup(1);
                continue;
            }
        
        }
    }
if(unlink(constant_process_name) != 0) perror("Failed to delete main process FIFO file!");

}
