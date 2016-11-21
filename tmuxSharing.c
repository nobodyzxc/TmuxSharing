#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#define ASSERTCMD(cmd) \
    system(cmd) ? (printf("tmuxSharing:%s\n" , cmd) , errQuit()) : 0==0
//assert and call back
#define ASSERTCMD_CB(cmd , func , msg) \
    system(cmd) ?  puts(msg) , func , errQuit() : 0==0
#define errMsg(x) printf("tmuxSharing: unknown option -- %c\n" , x)
#define usage() puts("usage: tmuxSharing [-mhpr] [-u user...] [-g group...] [-c]")
#define help()     printf("%s\n%s\n%s\n%s\n%s\n%s\n" ,\
            "-p set password" , \
            "-r read-only mod" , \
            "-m let session can be attached serveral times" , \
            "-u [user ...] specific user" , \
            "-g [group ...] specific group" , \
            "-c clean socket file" , \
            "-h help menu" \
            )
int readOnly = 0 , multiple = 0 , passwd = 0 , specificUser = 0 , 
        specificGroup = 0 , errRtn = 0;
char whoami[100] = {0} , cmd[100];
void giveExe(void);
void setFacl(int , char *[]);
void parseFlags(int argc , char *argv[]);
void getAns(char * , int *);
void cleanFiles(void);
void errQuit(void);

int main(int argc , char *argv[]){
    //whoami
    
    getlogin_r(whoami, sizeof(whoami));
    puts(whoami);
    if(whoami[0] == 0){
        FILE *myname = popen("whoami" , "r");
        fgets(whoami , sizeof(whoami) , myname);
        whoami[strlen(whoami) - 1] = 0;
        if(whoami[0] == 0) 
            puts("cannot get name") , exit(1);
    } 
    
    //parse flags
    parseFlags(argc , argv);
    //check tmux existing
    ASSERTCMD_CB("tmux -V" , (1 == 1) , "tmuxSharing: tmux not found");
    //generate session
                    //-S socket path    //-s session name
    ASSERTCMD("tmux -S $HOME/.`whoami`-S new -d -s `whoami`");
    ASSERTCMD("chmod 666 $HOME/.`whoami`-S");
   
    //give attach exe
    giveExe();

    //set ACL for specific*
    if(specificUser || specificGroup)
        setFacl(argc , argv); 

    //let session remove socket and exe file before exit
    ASSERTCMD("tmux -S $HOME/.`whoami`-S send-keys -t `whoami` 'function tmuxExit { rm -f $HOME/.`whoami`-S; rm -f $HOME/`whoami`-ShrSpt; }' C-m");
    ASSERTCMD("tmux -S $HOME/.`whoami`-S send-keys -t `whoami` 'trap tmuxExit EXIT' C-m"); 
    ASSERTCMD("tmux -S $HOME/.`whoami`-S send-keys -t `whoami` 'clear' C-m"); 

    //attach self session
    ASSERTCMD("tmux -S $HOME/.`whoami`-S attach -t `whoami`");
    ASSERTCMD("echo \"if [ -S $HOME/.`whoami`-S ];then echo 'be careful! session is still running';fi\" | bash");
    return 0;
}

void parseFlags(int argc , char *argv[]){
    int i , j , BREAK = 0;
    if(argc > 1){
        for(i = 1 ; i < argc ; i++){
            if(argv[i][0] == '-')
                for(j = 1 ; j < strlen(argv[i]) ; j++){
                    BREAK = 0;
                    switch(argv[i][j]){
                        case 'c':
                            cleanFiles();
                            puts("socket file removed");
                            exit(0); break;
                        case 'u':
                            multiple = 1;
                            specificUser = i + 1; 
                            while((i + 1 < argc)&&(argv[i + 1][0] != '-')) i++;
                            BREAK = 1;
                            break;
                        case 'g': 
                            multiple = 1;
                            specificGroup = i + 1; 
                            while((i + 1 < argc)&&(argv[i + 1][0] != '-')) i++;
                            BREAK = 1;
                            break;
                        case 'r': readOnly = 1; break;
                        case 'm': multiple = 1; break;
                        case 'p': passwd = 1; break;
                        case 'h': help() , exit(0); break;
                        default:
                            errMsg(argv[i][j]) , usage() , errQuit();
                    }
                    if(BREAK) break;
                }
            else usage() , errQuit();
        }
    }
    return;
}

void giveExe(void){
    char *HOMEDIR = getenv("HOME"); //marked , I can use this too.
    FILE *attach = fopen("attach.c" , "w"); 
    if(!attach) puts("failed") , errQuit();
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

    if(!multiple){
        sprintf(cmd , "tmux -S %s/.%s-S send-keys -t %s 'rm -f %s/%s-ShrSpt' C-m" , HOMEDIR , whoami , whoami , HOMEDIR , whoami);
        fprintf(attach , "system(\"%s\");\n" , cmd);
    }
    sprintf(cmd , "tmux -S %s/.%s-S attach -t %s" , HOMEDIR , whoami , whoami);
    if(readOnly) strcat(cmd , " -r ");
    fprintf(attach , "system(\"%s\");\n" , cmd);
 
    fputs("return 0;\n" , attach);
    fputs("}" , attach);
    fclose(attach);
    ASSERTCMD_CB("gcc -o $HOME/`whoami`-ShrSpt attach.c",
            (1 == 1) , "compile error");
    ASSERTCMD("rm -rf attach.c");
    return;
}

void setFacl(int argc , char *argv[]){
    ASSERTCMD("chmod 600 $HOME/.`whoami`-S");
    ASSERTCMD("chmod 700 $HOME/`whoami`-ShrSpt");
    if(specificUser){
        while(specificUser < argc && argv[specificUser][0] != '-'){
            sprintf(cmd , "setfacl -m u:%s:rw- $HOME/.`whoami`-S" ,
                    argv[specificUser]);
            ASSERTCMD(cmd);
            sprintf(cmd , "setfacl -m u:%s:--x $HOME/`whoami`-ShrSpt" ,
                    argv[specificUser]);
            ASSERTCMD(cmd);
            specificUser++;
        }
    }
    if(specificGroup){
        while(specificGroup < argc && argv[specificGroup][0] != '-'){
            sprintf(cmd , "setfacl -m g:%s:rw- $HOME/.`whoami`-S" ,
                    argv[specificGroup]);
            ASSERTCMD(cmd);
            sprintf(cmd , "setfacl -m g:%s:--x $HOME/`whoami`-ShrSpt" ,
                    argv[specificGroup]);
            ASSERTCMD(cmd);
            specificGroup++;
        }
    }
    return;
}

void cleanFiles(void){
    ASSERTCMD("rm -f $HOME/.`whoami`-S");
    ASSERTCMD("rm -f $HOME/attach.c");
    ASSERTCMD("rm -f $HOME/`whoami`-ShrSpt");
    return;
}

void errQuit(void){
    cleanFiles();
    exit(1);
    return;
}
