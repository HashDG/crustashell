#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <linux/limits.h>

#include "ligne_commande.h"

#define HOSTNAME_MAX 253
#define USERNAME_MAX 256

/* regcomp */

void affiche_prompt(void);
void execute_ligne_commande(char***, int, int);

int main() {
  char*** command;
  int flag, nb;
  while (1) {
    affiche_prompt();
    fflush(stdout);
    command = ligne_commande(&flag, &nb);

    while (waitpid(-1, NULL, WNOHANG) > 0);
    
    if (flag >= 0 && nb > 0) {
      if (strcmp(command[0][0], "exit") == 0) break;
      
      affiche(command);
      execute_ligne_commande(command, 0, flag);
      libere(command);
    }
  }
  
  return 0;
}

void affiche_prompt(void) {
  char pwd[PATH_MAX], hostname[HOSTNAME_MAX], *username, *home;
  
  if (getcwd(pwd, PATH_MAX) == NULL) {
    strcpy(pwd, "???");
  } else {
    if ( (home = getenv("HOME")) != NULL ) {
      if ( strncmp(pwd, home, strlen(home)) == 0) {
	memmove(pwd, &pwd[strlen(home)], strlen(home));
      }
    }
  }
  
  if (gethostname(hostname, HOSTNAME_MAX) < 0) strcpy(hostname, "???");
  
  if ( (username = getenv("USER")) == NULL ) strcpy(username, "???");

  printf("%s@%s:%s> ", username, hostname, pwd);
}

void execute_ligne_commande(commandes, nb, arriere_plan)
     char*** commandes;
     int nb;
     int arriere_plan;
{
  int pid;
  char* command, ** args;
  
  pid = fork();
  command = *commandes[nb];
  args = commandes[nb];

  if (pid == 0) {
    execvp(command, args);
    printf("%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (!arriere_plan) waitpid(pid, NULL, 0);
}
