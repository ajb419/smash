# smash

DESCRIPTION:
limited functionality shell

USES:
smash handles internal and external commands as well as IO redirections

INTERNAL COMMANDS:
exit - exits program
cd - change directory
history - displays last 10 commands including exit stautus'. A new history is initiated every time the program is started.

EXTERNAL COMMANDS:
smash searches PATH to execute external commands and prints a usage message if the command is not encountered

