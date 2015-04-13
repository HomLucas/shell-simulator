//By Lucas Hom & Eugene Fairley

#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <termios.h>
#include <ctype.h>
#include <cctype>
#include <errno.h>
#include <vector>
#include <cstring>
#include <dirent.h>
#include <string>
#include <cstdlib>
#include <cmath>
#include <sys/stat.h>
#include <sys/types.h>
using namespace std;
char E_MSG[38] = "Error Changing Changing Directory.\n";


void ResetCanonicalMode(int fd, struct termios *savedattributes){
    tcsetattr(fd, TCSANOW, savedattributes);
}

void SetNonCanonicalMode(int fd, struct termios *savedattributes){
    struct termios TermAttributes;
    char *name;
    
    // Make sure stdin is a terminal.
    if(!isatty(fd)){
        fprintf (stderr, "Not a terminal.\n");
        exit(0);
    }
    
    // Save the terminal attributes so we can restore them later.
    tcgetattr(fd, savedattributes);
    
    // Set the funny terminal modes.
    tcgetattr (fd, &TermAttributes);
    TermAttributes.c_lflag &= ~(ICANON | ECHO); // Clear ICANON and ECHO.
    TermAttributes.c_cc[VMIN] = 1;
    TermAttributes.c_cc[VTIME] = 0;
    tcsetattr(fd, TCSAFLUSH, &TermAttributes);
}

int main(int argc, char *argv[]){
    vector <string> v;
    struct termios SavedTermAttributes;
    char RXChar[100];
    DIR *dirp;
    struct dirent *dp;
    char bell = 0x07;
    char upArrow[3] = {0x1B, 0x5B, 0x41};
    char dArrow[3] = {0x1B, 0x5B, 0x42};
    int cPos;
    SetNonCanonicalMode(0, &SavedTermAttributes);
    string curDir;
    while(1){
        curDir = get_current_dir_name();
        if(curDir.length() > 16)
        {
            int last = curDir.find_last_of("/");
            curDir = "/..." + curDir.substr(last);
        }
        curDir += 0x3E;
        write(1, curDir.c_str(), curDir.length());
        int i;
        int curCom = v.size() - 1;
        int vectorPosition = v.size();
        for(i = 0; i < 100; ){
            read(0, &RXChar[i], 1);
            if(RXChar[i] == 0x1B)
            {
                read(0, &RXChar[i], 1);
                if (RXChar[i] == 0x5B)
                {
                    read(0, &RXChar[i], 1);
                    if (RXChar[i] == 0x41)
                    {
                        if(vectorPosition <= 0){
                            write(1,"\a",1);
                        }
                        else{
                            while(i > 0)
                            {
                                write(1,"\b \b",3);
                                i--;
                            }
                            vectorPosition--;
                            memcpy(RXChar,v[vectorPosition].c_str(),v[vectorPosition].size());
                            write(1,v[vectorPosition].c_str(),v[vectorPosition].length() );
                            i = v[vectorPosition].length();
                        }
                    }
                    else if(RXChar[i] == 0x42)
                    {
                        int x = vectorPosition - int(v.size()) +1;
                        if(x < 0){
                            while(i > 0)
                            {
                                write(1,"\b \b",3);
                                i--;
                            }
                            vectorPosition++;
                            memcpy(RXChar,v[vectorPosition].c_str(),v[vectorPosition].size());
                            write(1,v[vectorPosition].c_str(),v[vectorPosition].length() );
                            i = v[vectorPosition].length();
                        }
                        else if(vectorPosition == v.size())
                            write(1,"\a",1);
                        else{
                            vectorPosition = v.size();
                            while(i > 0)
                            {
                                write(1,"\b \b",3);
                                i--;
                            }
                        }
                    }
                    
                }
                
            }
            else if(RXChar[i] == 0x7F){
                if(i==0) write(1, &bell, 1);
                else{
                    write (1,"\b \b",3);
                    i -= 1;
                }
            }
            else{
                write(1, &RXChar[i], 1);
                if(RXChar[i] == 0x0A)
                {
                    break;
                }
                ++i;
            }
        }
        RXChar[i] = 0x00;
        string text(RXChar);
        if(RXChar[0]==0x00)
        {
            i = 0;
            continue;
            
        }
        v.push_back(text);
        vectorPosition = v.size()-1;
        if(RXChar[0] == 'c' && RXChar[1] == 'd'){
            cPos = 0;
            char sArg[15];
            int j;
            for(j = 2; j < i; ++j){
                if(!isspace(RXChar[j]))
                    break;
            }
            if(j == i){
                chdir(getenv("HOME"));
            }
            else{
                int k;
                for(k = j; k < i; ++k){
                    if(isspace(RXChar[k])) break;
                    sArg[k-j] = RXChar[k];
                }
                sArg[k-j] = 0x00;
                if(chdir(sArg)!=0)
                    write(1, E_MSG, 35);
            }
        }
        else if(RXChar[0] == 'p'  && RXChar[1] == 'w' && RXChar[2] == 'd'){
            char* red = get_current_dir_name();
            for(i = 0; ;++i){
                if(!isprint(red[i])) break;
            }
            red[i] = 0x0A;
            write(1,red,i+1);
        }
        else if(RXChar[0] == 'h'  && RXChar[1] == 'i' && RXChar[2] == 's' && RXChar[3] == 't'){
            int m = min(10,int(v.size()));
            for(int i = 0; i < m ; ++i){
                char dig = (char)(((int)'0')+i);
                write(1, &(dig),1);
                write(1, " ", 1);
                write(1, v[v.size()-m+i].c_str(), v[v.size()-m+i].length());
                write(1, "\n" , 1);
            }
        }
        else if(RXChar[0] == 'e'  && RXChar[1] == 'x' && RXChar[2] == 'i' && RXChar[3] == 't')
            break;
        else if(RXChar[0] == 'l' && RXChar[1] == 's'){

            cPos = 0;
            char sArg[15];
            int j;
            for(j = 2; j < i; ++j){
                if(!isspace(RXChar[j]))
                    break;
            }
            if(j == i){
                 if ((dirp = opendir(".")) == NULL) {
                return 0;
                }
            }
            else{
                int k;
                for(k = j; k < i; ++k){
                    if(isspace(RXChar[k])) break;
                    sArg[k-j] = RXChar[k];
                }
                sArg[k-j] = 0x00;
                if ((dirp = opendir(sArg)) == NULL) {
                return 0;
               }
            }
            while((dp = readdir(dirp)) != NULL) {
                struct stat fileStat;
                stat(dp->d_name,&fileStat);
                
                if(S_ISDIR(fileStat.st_mode))
                    write(1,"d", 1);
                else
                    write(1, "-", 1);
                
                if(fileStat.st_mode & S_IRUSR)
                    write(1, "r" ,1);
                else
                    write(1,"-", 1);
                if(fileStat.st_mode & S_IWUSR)
                    write(1,"w",1);
                else
                    write(1, "-", 1);
                if(fileStat.st_mode & S_IXUSR)
                    write(1,"x",1);
                else
                    write(1,"-",1);
                if(fileStat.st_mode & S_IRGRP)
                    write(1,"r" , 1);
                else
                    write(1, "-", 1);
                if(fileStat.st_mode & S_IWGRP)
                    write(1, "w", 1);
                else
                    write(1, "-", 1);
                if(fileStat.st_mode & S_IXGRP)
                    write(1, "x", 1);
                else
                    write(1, "-", 1);
                if(fileStat.st_mode & S_IROTH)
                    write(1, "r", 1);
                else
                    write(1, "-", 1);
                if(fileStat.st_mode & S_IWOTH)
                    write(1,"w", 1);
                else
                    write(1, "-", 1);
                if(fileStat.st_mode & S_IXOTH)
                    write(1, "x", 1);
                else
                    write(1,"-",1);
                write(1," ", 1);
                
                for(int i = 0; i < dp->d_reclen && dp->d_name[i]!= 0x00; ++i){
                    if(isprint(dp->d_name[i]))
                    {
                        
                        write(1, &(dp->d_name[i]), 1);
                    }
                }
                RXChar[0] = 0x0A;
                write(1, &RXChar[0], 1);
            }
        }
        else{
            char** strArray = new char*[20*sizeof(char*)];
            char* temp;
            int i;
            int rDir;
            int stat;
            char* s = RXChar;
            int count =-1;
            char** arr = new char*[20*sizeof(char*)];
            int pipes[20];
            char ** pipeArgs[20];
            int numPipes = -1;
            int lastPipe = 0;
            for(i=0, temp=strtok(s, " "); temp!=NULL;++i ) {
                strArray[i]=temp;
                temp = strtok(NULL, " ");
            }
            for(int j =0; j<i;++j){
                if(*(strArray[j]) != '|' && *(strArray[j]) != '<' && *(strArray[j]) != '>'){
                    arr[++count] = strArray[j];
                }
                else if(*(strArray[j]) == '>'){
                    ++j;
                    arr[++count] = strArray[j];
                    rDir=open(arr[count],O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                    dup2(rDir,1);
                    close(rDir);
                }
                else if(*(strArray[j]) == '<'){
                    ++j;
                    arr[++count] = strArray[j];
                    rDir=open(arr[count],O_RDONLY);
                    dup2(rDir,0);
                    close(rDir);
                }
                else{ 
                    pipe(pipes + (++numPipes*2));
                    for(int k = lastPipe; k < j; ++k){
                        *(pipeArgs[numPipes]+k-lastPipe) = strArray[k];
                    }
                    lastPipe = j+1;
                }
            }
            if(numPipes>-1){
                if(fork()==0){
                    dup2(pipes[1],1);
                    for(int k = 0;k<=numPipes; ++k){
                        close(pipes[k]);
                    }
                    execvp(*(pipeArgs[0]),pipeArgs[0]);
                    exit(1);
                }
                else{
                    for(int k = 0; k <= numPipes -1; ++k){
                        if(fork() == 0){
                            dup2(pipes[k],0);
                            dup2(pipes[k+2],1);
                            for(int l = 0;l<=numPipes; ++l){
                                close(pipes[l]);
                            }
                            execvp(*(pipeArgs[k+1]),pipeArgs[k+1]);
                            exit(1);
                        }
                    }
                }
                if(fork() == 0){
                    dup2(pipes[numPipes],0);
                    for(int k = 0;k<=numPipes; ++k){
                        close(pipes[k]);
                    }
                    execvp(*(pipeArgs[numPipes]),pipeArgs[numPipes]);
                    exit(1);
                }
                for(int l = 0; l <= numPipes; ++l){
                    close(pipes[l]);
                }
                for(int r = 0; r <= numPipes; ++r){
                    wait(&stat);
                }
                continue;
            }
            pid_t child;
            int cstatus; 
            pid_t childPid;
            
            if ((child = fork()) == 0) { 
                if(execvp(arr[0], arr)<0){
                    write(1,"Failed to execute ", 18);
                    write(1, RXChar, strlen(RXChar));
                    write(1,"\n",1);
                    exit(1);
                }
            }
            else { 
                if(child != (pid_t)(-1)){
                    childPid = wait(&cstatus); 
                }
            }
        }
    
    }
    
}
