/*
 * =====================================================================================
 *
 *       Filename:  test1.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/23/2012 08:39:48 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Chen Yuheng (Chen Yuheng), chyh1990@163.com
 *   Organization:  Tsinghua Unv.
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <regex.h>

#define TEST_PTHREAD
#ifdef TEST_PTHREAD
#include <pthread.h>
#include <semaphore.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <linux/input.h>
#include <math.h>

#include <linux/fb.h>
#include "goldfish_logo.h"

#define CALL_TEST(func) do {printf(">>> Call Test " #func " @ %s:%d\n", __FILE__, __LINE__); \
      test_##func (); \
      printf(">>> Test " #func " Done.\n"); }while(0);

#define DEFINE_TEST(func) static void test_##func ()

DEFINE_TEST(file)
{
  FILE* f = fopen("abcd.txt", "w");
  if(!f){
    printf("Failed to open for w\n");
    return ;
  }
  fprintf(f, "Hello %d\n", 123);
  fclose(f);

  f = fopen("abcd.txt", "r");
  if(!f){
    printf("failed to open for write\n");
    return ;
  }
  int x;
  char buf[32];
  fscanf(f, "%s%d", buf, &x);
  printf("read: %s, %d\n", buf, x);
  fclose(f);
}

DEFINE_TEST(stdio)
{
  char buf[32];
  fprintf(stdout, "Hello!\n");
  printf("Input : \n");
  //scanf("%s", buf);
  //printf("%s", buf);
  fprintf(stderr, "Hello from STDERR\n");
  return;
}

DEFINE_TEST(regex)
{
  regex_t regex;
  int reti;
  char msgbuf[100];

  /* Compile regular expression */
  reti = regcomp(&regex, "^a[[:alnum:]]", 0);
  if( reti ){ fprintf(stderr, "Could not compile regex\n"); exit(1); }

  /* Execute regular expression */
  reti = regexec(&regex, "abc", 0, NULL, 0);
  if( !reti ){
    puts("Match");
  }
  else if( reti == REG_NOMATCH ){
    puts("No match");
  }
  else{
    regerror(reti, &regex, msgbuf, sizeof(msgbuf));
    fprintf(stderr, "Regex match failed: %s\n", msgbuf);
    exit(1);
  }

  /* Free compiled regular expression if you want to use the regex_t again */
  regfree(&regex);

}

DEFINE_TEST(perror)
{
  read(0x324543, NULL, 1);
  perror("test perror");
}

DEFINE_TEST(memalign)
{
  void *p = (void*)memalign (getpagesize (), 3<<12);
  if(!p)
    perror("failed");
  free(p);
}

DEFINE_TEST(sbrk)
{
  void *p = (void*)sbrk(0);
  printf("Current brk: 0x%08x\n", p);
}

DEFINE_TEST(dir)
{
  DIR *dp;
  struct dirent *ep;

  dp = opendir ("./");
  if (dp != NULL)
  {
    while (ep = readdir (dp))
      puts (ep->d_name);
    (void) closedir (dp);
  }
  else
    perror ("Couldn't open the directory");

}

DEFINE_TEST(fstat)
{
  struct stat fileStat;
  const char *fn = "/bin/ls";
  printf("sizeof(struct stat)=%d\n", sizeof(stat));
  if(stat(fn,&fileStat) < 0){ 
    perror("stat:");
    return ;
  }

  printf("Information for %s\n", fn);
  printf("---------------------------\n");
  printf("File Size: \t\t%d bytes\n",fileStat.st_size);
  printf("Number of Links: \t%d\n",fileStat.st_nlink);
  printf("File inode: \t\t%d\n",fileStat.st_ino);

  printf("File Permissions: \t");
  printf( (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
  printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
  printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
  printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
  printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
  printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
  printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
  printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
  printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
  printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");
  printf("\n\n");

  printf("The file %s a symbolic link\n", (S_ISLNK(fileStat.st_mode)) ? "is" : "is not");
}

DEFINE_TEST(env)
{
  char *p = getenv("PATH");
  if(!p){
    printf("No $PATH\n");
    return ;
  }
  printf("PATH=%s\n", p);
}



DEFINE_TEST(fork)
{
#define FORK_CNT 3
  int i=0;
  pid_t pid = -1;
  pid_t pids[FORK_CNT];
  for(i=0;i<FORK_CNT;i++){
    pid = fork();
    if(pid < 0){
      perror("fork()");
      return ;
    }
    if(pid == 0){
      printf("I'm child\n");
      exit(1);
    }else{
      pids[i] = pid;
      printf("Fork Child %d\n", pid);
    }
  }
  int status;
#if 1
  for(i=0;i<FORK_CNT;i++){
    waitpid(pids[i], &status, 0);
    printf("Child %d end %d\n", pids[i], WEXITSTATUS(status));
  }
#else
  while(1)
    sched_yield();
#endif
  printf("OK...\n");
}

DEFINE_TEST(execv)
{
  const char* fn = "/bin/ls";
  char *arg[] = {"/bin/ls", NULL};
  if(fork()){
    int status;
    int pid = wait(&status);
    if(pid > 0)
      printf("DONE\n");
    else
      perror("wait()");
  }else{
    execv(fn, arg);
  }
}

void mssleep(int ms)
{
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = ms * 1000000;
  nanosleep(&ts, &ts);
}

DEFINE_TEST(sleep)
{
  printf("sleep 2s\n");
  sleep(2);
  printf("usleep 5ms\n");
  mssleep(5);
  printf("usleep 10ms\n");
  mssleep(10);
  printf("usleep 50ms\n");
  mssleep(50);
  printf("usleep 400ms\n");
  mssleep(400);
}

void *worker_thread( void *ptr )
{
  int id = (int)ptr;
  int i = 0;
  for(i=0;i<10;i++){
    printf("I'm %d %d\n", id, i);
    mssleep(20);
  }
  sleep((id+1));
  printf("I'm %d, dying \n", id);
  return NULL;
}
DEFINE_TEST(pthread)
{
#ifdef TEST_PTHREAD
  static const int THREAD_CNT = 3;
  pthread_t thread[THREAD_CNT];
  int i;
  for(i=0;i<THREAD_CNT;i++){
    int ret = pthread_create( thread+i, NULL, worker_thread, (void*)i );
    if(ret){
      perror("pthread_create()");
      return;
    }
    printf("pthread_create: tid %d\n", thread[i]);
  }
  printf("create done\n");
  for(i=0;i<THREAD_CNT;i++)
    pthread_join(thread[i], NULL);
    
#else
  printf("No test\n");
#endif
}

DEFINE_TEST(mmap)
{
  //int fd = open("fb0:", O_RDWR);
  struct fb_var_screeninfo vinfo;
  int i,j;
  int w = 800;
  int h = 600;
  int x = (w - logo_width)/2;
  int y = (h - logo_height)/2;
  int v = 2, u = 2;
  int fd = open("fb0:", O_RDWR);
  if(fd < 0){
    perror("open()");
    return ;
  }
  if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) <0){
    printf("ioctl error\n");
    return ;
  }else{
    printf("fb0 vinfo: %dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);
    w = vinfo.xres;
    h = vinfo.yres;
  }

  assert(logo_height < h && logo_width < w);

  int size = w*h*vinfo.bits_per_pixel/8;

  unsigned short *buf = (unsigned short*)mmap(NULL, size,0,0,fd,0);
  printf("mmap get %08x\n", buf);
  if(buf)
    memset(buf,0xff, size);
  close(fd);

}

DEFINE_TEST(signal)
{

}

static sem_t sem1, sem2;
void *pthread_a( void *argv )
{
    sem_wait( & sem2 );
    printf(" time = %s, pthread [%s]\n", __TIME__, __FUNCTION__);
    *(int*)argv =  20;
    return (void*)0;
}
void *pthread_b( void *argv )
{
    sem_wait( & sem1 );
    printf(" time = %s, pthread [%s]\n", __TIME__, __FUNCTION__);
    sem_post( & sem2 );
    return (void*)1;
}
void *pthread_c( void *argv )
{
    sem_post( (sem_t*)argv );
    printf(" time = %s, pthread [%s]\n", __TIME__, __FUNCTION__);
    return (void*)2;
}
DEFINE_TEST(sem)
{
    pthread_t a[3] ;
    int i ;
    int mm = 10;
    /*void *(*thread_func[3])(void *argv );*/
    sem_init( &sem1, 0, 0 );
    sem_init( &sem2, 0, 0 );
    pthread_create( &a[0], NULL , pthread_a, &mm);
    pthread_create( &a[1], NULL , pthread_b, NULL );
    pthread_create( &a[2], NULL , pthread_c, &sem1 );
    pthread_join ( a[2], (void * )&i ) ;
    printf(" argv = %d \n", i );
    pthread_join ( a[0], (void * )&i ) ;
    printf(" argv = %d \n", i );
    pthread_join ( a[1], (void * )&i ) ;
    printf(" argv = %d \n", i );
    printf(" mm = %d\n", mm);
}

void myprintf(float f)
{
  int a = (int)(f*1000.0f);
  printf("f*1000=%d\n", a);
}

DEFINE_TEST(float)
{
  float x = 1.32;
  float b = x/2;
  int  ix = (int)sin(x);
  printf("%f\n", 1.32f);
  printf("x=%f b=%f sin(x)=%f\n", x,b, sin(x));
  printf("ix=%d\n", ix);
  myprintf(x);
  myprintf(sin(x));
}

DEFINE_TEST(input)
{
  char *device = "event0:";
  int fd = open(device, O_RDWR);
  if(fd < 0){
    perror("open()");
    return;
  }
  //Print Device Name
  char name[128] = "Unknown";
  ioctl (fd, EVIOCGNAME (sizeof (name)), name);
  printf ("Reading From : %s (%s)\n", device, name);
  int rd, value, size = sizeof (struct input_event);
  struct input_event ev[64];
  int i;

#if 1
  int x, y;
  while (1){
    mssleep(200);
    if ((rd = read (fd, ev, size * 64)) < 0)
      perror("read()");      
    if(rd == 0){
      continue;
    }

    printf("read %d\n", rd);
    for(i=0;i<rd/sizeof(struct input_event);i++){
      printf(">> %d %d %d\n", ev[i].type, ev[i].code, ev[i].value);
    }
    for(i=0;i<rd/sizeof(struct input_event);i+=2){
      if(ev[i].type == EV_ABS && ev[i+1].type == EV_ABS){
        x = ev[i].value;
        y = ev[i+1].value;
        printf("INPUT: %d %d \n", x,y);
      }
    }

  }
#else
  while(1){
    printf("waiting input...\n");
    sleep(1);
  }
#endif

  close(fd);
}

int main(int argc, char *argv[])
{
#if 1
  int pid = getpid();
  int a;
  printf("Hello mypid = %d!!\n", pid);
  //scanf("%d", &a);
  //printf("It's %d\n", a);

  CALL_TEST(stdio);

  printf("test malloc & free\n");
  unsigned char* buf = (unsigned char*)malloc(1202);
  buf[0] = 'A';
  free(buf);
  buf = (unsigned char*)malloc(4096*2-32);
  buf[0] = 'A';
  printf("BUF = %p\n", buf);
  free(buf);

  CALL_TEST(file);
  CALL_TEST(regex);
  CALL_TEST(perror);
  CALL_TEST(memalign);
  CALL_TEST(sbrk);
  CALL_TEST(dir);
  CALL_TEST(fstat);
  CALL_TEST(env);
#endif
  CALL_TEST(fork);
  CALL_TEST(execv);
  //CALL_TEST(sleep);

  CALL_TEST(signal)
    CALL_TEST(mmap);
  CALL_TEST(pthread);
  CALL_TEST(float);
  CALL_TEST(sem);
  CALL_TEST(input);

  //while(1);
  return 0;
}

