#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#define ASSERTCMD(cmd) \
    system(cmd) ? (printf("tmuxSharing:%s\n" , cmd) , errQuit()) : 0==0
//assert and call back
#define ASSERTCMD_CB(cmd , func , msg) \
    system(cmd) ?  puts(msg) , func , errQuit() : 0==0
#define errMsg(x) printf("tmuxSharing: unknown option -- %s\n" , x)
#define usage() puts("usage: tmuxSharing [-ahpr] [-u user...] [-g group...]")
#define help()     printf("%s\n%s\n%s\n%s\n%s\n%s\n" ,\
            "-p set password" , \
            "-r read-only mod" , \
            "-a let attach once" , \
            "-u specific user" , \
            "-g specific group" , \
            "-h help menu" \
            )
int readOnly = 0 , oneTime = 0 , passwd = 0 , specificUser = 0 , 
        specificGroup = 0 , errRtn = 0;
char whoami[100] , cmd[100];
void giveExe(void);
void setFacl(int , char *[]);
void parseFlags(int argc , char *argv[]);
void getAns(char * , int *);
void errQuit(void);

int main(int argc , char *argv[]){
    //whoami
    getlogin_r(whoami, sizeof(whoami));
    //check tmux existing
    ASSERTCMD_CB("tmux -V" , (1 == 1) , "tmuxSharing: tmux not found");
    //parse flags
    parseFlags(argc , argv);
    //generate session
    ASSERTCMD("tmux -S /tmp/.`whoami`-S new -d -s `whoami`");
    ASSERTCMD("chmod 666 /tmp/.`whoami`-S");
   
    //give attach exe
    giveExe();

    //set ACL for specific*
    if(specificUser || specificGroup)
        setFacl(argc , argv); 
 
    //attach self session
    ASSERTCMD("tmux -S /tmp/.`whoami`-S attach -t `whoami`");
    ASSERTCMD_CB("echo \"! tmux -S /tmp/.`whoami`-S ls\" | bash" , exit(0) , 
            "detached . will not remove session");
    //! tmux -S /tmux .... ( ! op may not work in some sh version)
    ASSERTCMD("rm -f /tmp/.`whoami`-S");
    puts("remove session");
    ASSERTCMD("rm -f /tmp/`whoami`-ShrSpt");
    puts("remove shareSpot");
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
                        case 'u': 
                            specificUser = i + 1; 
                            while((i + 1 < argc)&&(argv[i + 1][0] != '-')) i++;
                            BREAK = 1;
                            break;
                        case 'g': 
                            specificGroup = i + 1; 
                            while((i + 1 < argc)&&(argv[i + 1][0] != '-')) i++;
                            BREAK = 1;
                            break;
                        case 'r': readOnly = 1; break;
                        case 'a': oneTime = 1; break;
                        case 'p': passwd = 1; break;
                        case 'h': help() , exit(0); break;
                        default:
                            errMsg(argv[i]) , usage() , errQuit();
                    }
                    if(BREAK) break;
                }
            else errMsg(argv[i]) , usage() , errQuit();
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
    ASSERTCMD_CB("gcc -o /tmp/`whoami`-ShrSpt /tmp/attach.c",
            (1 == 1) , "compile error");
    ASSERTCMD("rm -rf /tmp/attach.c");
    return;
}

void setFacl(int argc , char *argv[]){
    ASSERTCMD("chmod 600 /tmp/.`whoami`-S");
    ASSERTCMD("chmod 700 /tmp/`whoami`-ShrSpt");
    if(specificUser){
        while(specificUser < argc && argv[specificUser][0] != '-'){
            sprintf(cmd , "setfacl -m u:%s:rw- /tmp/.`whoami`-S" ,
                    argv[specificUser]);
            ASSERTCMD(cmd);
            sprintf(cmd , "setfacl -m u:%s:--x /tmp/`whoami`-ShrSpt" ,
                    argv[specificUser]);
            ASSERTCMD(cmd);
            specificUser++;
        }
    }
    if(specificGroup){
        while(specificGroup < argc && argv[specificGroup][0] != '-'){
            sprintf(cmd , "setfacl -m g:%s:rw- /tmp/.`whoami`-S" ,
                    argv[specificGroup]);
            ASSERTCMD(cmd);
            sprintf(cmd , "setfacl -m g:%s:--x /tmp/`whoami`-ShrSpt" ,
                    argv[specificGroup]);
            ASSERTCMD(cmd);
            specificGroup++;
        }
    }
    return;
}

void errQuit(void){
    ASSERTCMD("rm -f /tmp/.`whoami`-S");
    ASSERTCMD("rm -f /tmp/attach.c");
    ASSERTCMD("rm -f /tmp/`whoami`-ShrSpt");
    puts("ERR QUIT");
    exit(1);
    return;
}
