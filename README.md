### PSiW
Created by Benedykt Huszcza (benedykt.huszcza@gmail.com)
#### Final project on the Poznan Univeristy of Technology
Project was created to pass the subject "system and concurrent programming" (pl: Programowanie systemowe i Współbieżne).
#### Tutorial
How to run:
```
gcc -o project main.c
./project usr1
```
Pattern: \\
process_name_to_execute_comand "command" "fifo_queue_name_to_send_result"
Example commands:
```
> usr1 "ls" "fifoQueueName"
> exit
```
if you want to clear all created fifo files during the program you should exit program by typing exit.

#### How it works
The program takes argument (in our example "user1") open the config file and searching for fifo file name which is connected with argument name.
Then program is waiting for commands. After input commands program split it to three parts: process name which should execute command, command and name of FIFO file where result of the command should be send. After that it creates and opens fifo file, search for the fifo file for process which chould be execute command and send o this queue comand and namoe of the fifo where result should be send. In the same time child of main process waiting for showing informations in own fifo file. When command and fifo name appears it execute command and send the result by fifo queue.
