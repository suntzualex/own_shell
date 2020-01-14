/**
 * Shell
 * Operating Systems
 * v18.12.19
 */

/**
 Hint: F1 (or Control-click) op een functienaam om naar de 'man'-pagina te gaan.
 Hint: F2 (or Control-click) op een functienaam of variabele om naar de definitie te gaan.
 Hint: Ctrl-space voor auto complete functie- en variable-namen.
 */
#include "shell.h"

/**
* Functie declaraties ingebouwde shell commando's
*/
int change_directory(char* dir);
int exit_shell();

// Voer een Command datastructuur uit.
// Als deze niet uitgevoerd kan worden, geef dan een foutcode terug.
// Als het commando uitgevoerd wordt,
// wordt het huidige proces vervangen door het nieuwe proces.
int executeCommand(const Command& cmd) {
 auto& parts = cmd.parts;
 return execvp(parts);
}

// ------------------------------------------------------------

// TODO: recursieve functie schrijven en alle andere functies
// aanpassen
// -------------------------------------------------------------



/**
* @brief executeAllCommands: chained commands and file input/output
* @param expression a struct representing an expression
* @return int representing success or failure (-1)
*/
int executeAllCommands(Expression& expression){

   bool background = expression.background; // set wait according to this bool.
   int fd[2]; // the filedescriptors
   pid_t pid; // place to hold pid
   int fdd = 0; // backup file descriptor
   ulong i = 0; // start index
   // create a pipeline.
   // do while the vector parts in the struct Command[i] is not empty
   while(i < expression.commands.size() && !expression.commands[i].parts.empty()){

       int p = pipe(fd); // create a pipe.
       if(p == -1){
           perror("pipe failed");
           exit(EXIT_FAILURE);
       }
       pid = fork(); // create a new process.
       if(pid == -1){
           perror("fork failed");
           exit(EXIT_FAILURE);
       }
       // child pid == 0 executing in child.
       if(pid == 0){
           // check of er een fromFile is.
           if(expression.fromFile != ""){
               // read from file.
               FILE *inFile = fopen(expression.fromFile.c_str(), "r");
               if(inFile == NULL){
                   perror("Kan bestand niet lezen.\n");
                   exit(EXIT_FAILURE);
               }
               int filein = fileno(inFile);
               int filedup = dup2(filein, 0);
               if(filedup == -1){
                   perror("failed dup2 on file read.\n");
                   exit(EXIT_FAILURE);
               }
               close(filein); // sluit bestand na lezen
               expression.fromFile = ""; // unset fromfile
               dup2(fdd,0); // zonder checks TODO aanpassen
           } else { // no file to read in.
           int dupr = dup2(fdd,0);
           if(dupr < 0){
               perror("dup2 failed");
               exit(EXIT_FAILURE);
             }
           }
           // if there is a next expression
           if((i+1) < expression.commands.size() && !expression.commands[i+1].parts.empty()){
              dup2(fd[1],1); // schrijven.
           }
           // if there is no next expression but fileTo is set then write to a file
          if((i+1) == expression.commands.size() && expression.toFile != ""){
              FILE *file = fopen(expression.toFile.c_str(), "w");
              if(file == NULL){
                  perror("Kan niet naar bestand schrijven.\n");
              }
              int filefd = fileno(file);
              int filedup = dup2(filefd, 1); // write to file.
              if(filedup == -1){
                  perror("failed dup2 on file write.\n");
                  exit(EXIT_FAILURE);
              }
              close (filefd);
          }
           close(fd[0]); // close read.
           executeCommand(expression.commands[i]); // execute command
           exit(1);
       } else { // code executing in parent, if background is true (not 0) then do no wait.
           close(fd[1]); // close write
           fdd = fd[0]; // backup file descriptor
           i++; // increment index
           if(!background){
             waitpid(pid, nullptr, 0);
           }
       }
   }
   return 0;
}

/**
* @brief executeExpression: execute the expression consisting of commands.
* @param expression, has a vector of commands which are Command structs containing parts
* @return an integer indicating success or failure.
*/

int executeExpression(Expression& expression) {

 // check for empty expression
 if (expression.commands.size() == 0)
   return EINVAL;

 // kijk of een intern commando afgehandeld moet worden (zoals 'cd' en 'exit')
 if (expression.commands.size() == 1
   && expression.commands[0].parts.size() == 2
   && expression.commands[0].parts[0] == "cd") {
   // handel 'cd' af
       string d = expression.commands[0].parts[1];
       char *cstr = new char[d.length() + 1];
       strcpy(cstr, d.c_str());
       change_directory(cstr);
   return 0;
   }
   if (expression.commands.size() == 1
       && expression.commands[0].parts.size() == 1
       && expression.commands[0].parts[0] == "exit")
   {
       return exit_shell();
   }

   // externe commando's, die uitgevoerd moeten met fork():
   // loop over alle commando's, en knoop de invoer en uitvoer aan elkaar
 // voor nu een dummy aanroep om een semi werkend programma te hebben
   // executeCommand(expression.commands[0]);

   return executeAllCommands(expression);
}

int shell(bool showPrompt) {

 while (cin.good()) {
   string commandLine = requestCommandLine(showPrompt);
   Expression expression = parseCommandLine(commandLine);
   int rc = executeExpression(expression);
   if (rc != 0)
     cout << strerror(rc) << endl;
 }
   return 0;
}

/**
* @brief change_directory verzorgt het wisselen van map/directory.
* @param dir de directory waarnaar moet worden veranderd.
* @return een integer: 0 succes -1 falen.
*/
int change_directory(char* dir){

   if(dir == NULL){
       fprintf(stderr,"Opgegeven pad kon niet worden gevonden.\n");
       return -1;
   }
   if(chdir(dir) != 0){
       perror("cd commando fout.\n");
       return -1;
   }
   return 0;
}


/**
 * @brief exit verzorgt de ingebouwde exit functie van de shell.
 * @return altijd 0 terugkeren.
 *
 */
int exit_shell(){
   printf("U verlaat de shell.\n");
   exit(0);
}



// NIET VERANDEREN, nodig voor het test raamwerk
int main(int argc, char** argv) {
 bool showPrompt = argc == 1;
 return shell(showPrompt);
}
