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
int lance_commande(int, int, const char*, char**);

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

      execute_ligne_commande(command, nb, flag);
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
  int lastpid;
  char* command, ** args;
  int tube1[2], tube2[2], *tubes[2];
  const int size = nb;
  
  while (nb--) {    
    command = **commandes;
    args = *(commandes++);

    if ((size-nb)%2==0) {
      pipe(tube2);
      tubes[0] = tube1;
      tubes[1] = tube2;
    } else {
      pipe(tube1);
      tubes[0] = tube2;
      tubes[1] = tube1;
    }
    
    lastpid = lance_commande(nb==size-1 ? 0 : tubes[0][0],
		     nb == 0 ? 1 : tubes[1][1],
		     command,
		     args);
  }

  close(tube1[0]);
  close(tube1[1]);
  close(tube2[0]);
  close(tube2[1]);
  
  if (!arriere_plan) waitpid(lastpid, NULL, 0);
}

int lance_commande(in, out, command, args)
     int in;
     int out;
     const char* command;
     char** args;
{
  int pid;

  printf("I : %i, O : %i\n", in, out);

  pid = fork();

  if (pid == 0) {
    if (in != 0) {
      close(0);
      dup(in);
      close(in);
    }

    if (out != 1) {
      close(1);
      dup(out);
      close(out);
    }

    execvp(command, args);
    printf("%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  } else {
    if (in != 0) close(in);
    if (out != 1) close(out);
  }
  return pid;
}
