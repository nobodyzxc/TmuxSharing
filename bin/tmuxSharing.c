#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#define errMsg(x) printf("tmuxSharing: unknown option -- %s\n" , x)
#define usage() puts("usage: tmuxSharing [-h] [-r] [-i] [-f file]")
int readOnly = 0 , interactive = 0 , oneTime = 0 , sourceFile = 0 , passwd = 0;
char whoami[100] , cmd[100];
void giveExe(void);
void help(void);
void parseFlags(int argc , char *argv[]);

int main(int argc , char *argv[]){
    //whoami
    getlogin_r(whoami, sizeof(whoami));
    //check tmux existing
    if(system("tmux -V") != 0)
        puts("tmuxSharing: tmux not found") , exit(1);
    //parse flags
    parseFlags(argc , argv);
    //generate session
    system("tmux -S /tmp/`whoami`-tmuxSharing new -d -s `whoami`");
    system("chmod 777 /tmp/`whoami`-tmuxSharing");
    
    //give attach exe
    giveExe();
    //attach self session
    system("tmux -S /tmp/`whoami`-tmuxSharing attach -t `whoami`");
    system("rm -rf /tmp/`whoami`-tmuxSharing");
    return 0;
}

void parseFlags(int argc , char *argv[]){
    int i , j;
    if(argc > 1){
        for(i = 1 ; i < argc ; i++){
            if(argv[i][0] == '-')
                if(argv[i][1] == '-')
                    if(!strcmp("--help" , argv[i])) usage() , exit(0);
                    else
                        for(j = 1 ; j < strlen(argv[i]) ; j++){
                            switch(argv[i][j]){
                                case 'r':
                                    readOnly = 1;
                                    break;
                                case 'i':
                                    interactive = 1;
                                    break;
                                case 'f':
                                    sourceFile = 1;
                                    break;
                                case 'a':
                                    oneTime = 1;
                                    break;
                                case 'l':
                                    passwd = 1; 
                                    break;
                                case 'h': 
                                    help() , exit(0);
                                    break;
                                default:
                                    errMsg(argv[i]) , usage() , exit(1);
                            }
                        }
                else errMsg(argv[i]) , usage() , exit(1);
        }
    }
    return;
}

void giveExe(void){
    FILE *attach = fopen("/tmp/attach.c" , "w"); 
    fputs("#include<stdlib.h>\n" , attach);
    fputs("int main(void){\n" , attach);
     
    if(oneTime){
        sprintf(cmd , "tmux -S /tmp/%s-tmuxSharing send-keys -t %s 'rm -f /tmp/tmuxSharing-%s' C-m" , whoami , whoami , whoami);
        fprintf(attach , "system(\"%s\");\n" , cmd);
    }     
    sprintf(cmd , "tmux -S /tmp/%s-tmuxSharing attach -t %s" , whoami , whoami);
    if(readOnly) strcat(cmd , " -r ");
    fprintf(attach , "system(\"%s\");\n" , cmd);
 
    fputs("return 0;\n" , attach);
    fputs("}" , attach);
    fclose(attach);
    system("gcc -o /tmp/tmuxSharing-`whoami` /tmp/attach.c");
    system("rm -rf /tmp/attach.c");
    return;
}

void help(){
    printf("%s\n%s\n%s\n%s\n%s\n" ,
            "-i interactive setting mod" ,
            "-r read-only mod" ,
            "-f use source file" , 
            "-a let attach once" , 
            "-h | --help help menu"
            );
    return;
}
