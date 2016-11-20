#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#define errMsg(x) printf("tmuxSharing: unknown option -- %s\n" , x)
#define usage() puts("usage: tmuxSharing [-ahpru]")
#define help()     printf("%s\n%s\n%s\n%s\n%s\n" ,\
            "-u specific user" , \
            "-p set password" , \
            "-r read-only mod" ,\
            "-a let attach once" ,\
            "-h help menu"\
            )
int readOnly = 0 , oneTime = 0 , passwd = 0 , specificUser = 0;
char whoami[100] , cmd[100] , userName[100];
void giveExe(void);
void parseFlags(int argc , char *argv[]);
void getAns(char * , int *);

int main(int argc , char *argv[]){
    //whoami
    getlogin_r(whoami, sizeof(whoami));
    //check tmux existing
    if(system("tmux -V") != 0)
        puts("tmuxSharing: tmux not found") , exit(1);
    //parse flags
    parseFlags(argc , argv);
    //generate session
    system("tmux -S /tmp/.`whoami`-S new -d -s `whoami`");
    system("chmod 666 /tmp/.`whoami`-S");
   
    //give attach exe
    giveExe();

    //set ACL for specificUser
    if(specificUser)
        system("chmod 600 /tmp/.`whoami`-S") , 
        sprintf(cmd , "setfacl -m u:%s:rw- /tmp/.`whoami`-S" , userName) , 
        system(cmd) == 0 ? puts("ACL setted") : puts("ACL ERR") , exit(1) , 
        system("chmod 700 /tmp/`whoami`-ShrSpt") , 
        sprintf(cmd , "setfacl -m u:%s:--x /tmp/`whoami`-ShrSpt" , userName) , 
        system(cmd) == 0 ? puts("ACL setted") : puts("ACL ERR") , exit(1);
 
    //attach self session
    system("tmux -S /tmp/.`whoami`-S attach -t `whoami`");
    if(system("tmux -S .`whoami`-S ls") != 0){
        system("rm -rf /tmp/.`whoami`-S");
        puts("rm tmp session");
    }
    return 0;
}

void parseFlags(int argc , char *argv[]){
    int i , j;
    if(argc > 1){
        for(i = 1 ; i < argc ; i++){
            if(argv[i][0] == '-')
                for(j = 1 ; j < strlen(argv[i]) ; j++){
                    switch(argv[i][j]){
                        case 'r':
                            readOnly = 1;
                            break;
                        case 'a':
                            oneTime = 1;
                            break;
                        case 'p':
                            passwd = 1; 
                            break;
                        case 'u':
                            specificUser = 1;
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
    fputs("#include<stdio.h>\n" , attach);
    fputs("#include<stdlib.h>\n" , attach);
    fputs("#include<string.h>\n" , attach);
    fputs("int main(void){\n" , attach);

    if(passwd){
        char wd[100];
        printf("set password:") , fgets(wd , sizeof(wd) , stdin);
        wd[strlen(wd) - 1] = 0;
        fputs("char wd[100];" , attach);
        fputs("printf(\"password:\");" , attach);
        fputs("fgets(wd , sizeof(wd) , stdin);" , attach);
        fputs("wd[strlen(wd) - 1] = 0;" , attach);
        fprintf(attach , "if(strcmp(wd , \"%s\")){" , wd);
        fprintf(attach , "    puts(\"incorrect password\");");
        fprintf(attach , "    exit(1);");
        fprintf(attach , "}");
    }

    if(oneTime){
        sprintf(cmd , "tmux -S /tmp/.%s-S send-keys -t %s 'rm -f /tmp/%s-ShrSpt' C-m" , whoami , whoami , whoami);
        fprintf(attach , "system(\"%s\");\n" , cmd);
    }

    sprintf(cmd , "tmux -S /tmp/.%s-S attach -t %s" , whoami , whoami);
    if(readOnly) strcat(cmd , " -r ");
    fprintf(attach , "system(\"%s\");\n" , cmd);
 
    fputs("return 0;\n" , attach);
    fputs("}" , attach);
    fclose(attach);
    if(system("gcc -o /tmp/`whoami`-ShrSpt /tmp/attach.c") != 0)
        puts("compile failed") , exit(1);
    system("rm -rf /tmp/attach.c");
    return;
}

