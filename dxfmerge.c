#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

FILE*fi,*fo;

#define MAXXY 999999999

double multix=1.0;
double multiy=1.0;
double addx=0.0;
double addy=0.0;
double preaddx=0.0;
double preaddy=0.0;

int fliphorz=0;
int flipvert=0;
int tozero=0;
int x_fliphorz=0;
int x_flipvert=0;
int x_tozero=0;

int shownhdr=0;
int shownftr=0;
//int autohdr=1;

double minx,maxx,miny,maxy;
double lastminx=0,lastmaxx=0,lastminy=0,lastmaxy=0;
long lines=0;


void resetdefaults()
{
  multix=1.0;
  multiy=1.0;
  addx=0.0;
  addy=0.0;
  preaddx=0.0;
  preaddy=0.0;
  fliphorz=0;
  flipvert=0;
  tozero=0;
}

double getdouble()
{ char s[512];
  fgets(s,510,fi);
  return atof(s);
}


double str2x(char*s)
{ double d;
  d=atof(s);
  if(x_tozero)d=d-minx;
  d=(d+preaddx)*multix;
  if(x_fliphorz)d=maxx-d;
  d=d+addx;
  if(d>lastmaxx)lastmaxx=d;else if(d<lastminx)lastminx=d;
  return d;
}
double str2y(char*s)
{ double d;
  d=atof(s);
  if(x_tozero)d=d-miny;
  d=(d+preaddy)*multiy;
  if(x_flipvert){d=maxy-d;if(x_tozero)d-=miny;}
  d=d+addy;
  if(d>lastmaxy)lastmaxy=d;else if(d<lastminy)lastminy=d;
  return d;
}


void resetlast()
{ lastminx=MAXXY;lastmaxx=0;
  lastminy=MAXXY;lastmaxy=0;
}


void scanfile()
{ char s1[512],s2[512],*sp;
  int i,t;
  double d;
  minx=MAXXY;maxx=-MAXXY;
  miny=MAXXY;maxy=-MAXXY;
  rewind(fi);
  //fliphorz=0;flipvert=0;tozero=0;
  x_fliphorz=0;x_flipvert=0;x_tozero=0;
  while(!feof(fi)){
    fgets(s1,510,fi);sp=strchr(s1,0x0d);if(sp)sp[0]=0;
    fgets(s2,510,fi);sp=strchr(s2,0x0d);if(sp)sp[0]=0;
     if(ferror(fi)){fprintf(stderr,"[Error %i at read]",ferror(fi));return;}
    i=atoi(s1);
    if((i==10)||(i==11)){d=atof(s2);if(d<minx)minx=d;if(d>maxx)maxx=d;}
    if((i==20)||(i==21)){d=atof(s2);if(d<miny)miny=d;if(d>maxy)maxy=d;}
    if(!strncmp(s2,"LINE",4))lines++;
  } 
}


#define MAXSEGMENTS 1024
double segments[MAXSEGMENTS+2][2];
int segnum=0;

void printtuples(int i,char*s)
{ printf("%3i\n",i);
  printf("%s\n",s);
}

void printtuplei(int i,int i2)
{ printf("%3i\n",i);
  printf("%i\n",i2);
}

void printtupled(int i,double d)
{ printf("%3i\n",i);
  printf("%f\n",d);
}


void printheader(int printsec)
{
  shownhdr=1;
  printtuples(0,"SECTION");
  printtuples(2,"HEADER");
  printtuples(9,"$ACADVER");
  printtuples(1,"AC1015");
  printtuples(9,"$INSUNITS");
  printtuplei(70,4);
  printtuples(0,"ENDSEC");
  if(printsec){
    printtuples(0,"SECTION");
    printtuples(2,"ENTITIES");
  }
}

void printfooter(int printsec)
{
  shownftr=1;
  if(printsec){
   printtuples(0,"ENDSEC");}
  printtuples(0,"SECTION");
  printtuples(2,"OBJECTS");
  printtuples(0,"DICTIONARY");
  printtuples(0,"ENDSEC");
  printtuples(0,"EOF");
  printtuples(0,"EOF");
}

void processfile(char*fn)
{ char s1[512],s2[512],*sp;
  int i,t;
//  double x1=0,y1=0,x2=0,y2=0,xo=0,yo=0,;
  double d;
  int isline=0;
  int isentsection=0;
//  FILE*f;

  snprintf(s1,255,"Included file: %s",fn);
  printtuples(999,s1);
  fi=fopen(fn,"r");if(!fi){snprintf(s1,255,"ERROR: Cannot open file '%s', skipped");
                           printtuples(999,s1);
                           fprintf(stderr,"%s\n",s1);
                           return;}

  scanfile();
  snprintf(s1,255,"fliphorz:%i flipvert:%i tozero:%i addx:%.2f addy:%.2f scalex:%.2f scaley:%.2f minx:%.2f miny: %.2f maxx:%.2f maxy:%.2f",
                  fliphorz,flipvert,tozero,addx,addy,multix,multiy,minx,miny,maxx,maxy);
  printtuples(999,s1);

  x_fliphorz=fliphorz;x_flipvert=flipvert;x_tozero=tozero;

  rewind(fi);
  resetlast();
  while(!feof(fi)){
    fgets(s1,510,fi);sp=strchr(s1,0x0d);if(sp)sp[0]=0;sp=strchr(s1,0x0a);if(sp)sp[0]=0;
    fgets(s2,510,fi);sp=strchr(s2,0x0d);if(sp)sp[0]=0;sp=strchr(s2,0x0a);if(sp)sp[0]=0;
     if(ferror(fi)){fprintf(stderr,"[Error %i at read]",ferror(fi));return;}

    i=atoi(s1);
    if(i==2)if(!strcmp(s2,"ENTITIES")){isentsection=1;continue;}
    if(i==0)if(!strcmp(s2,"ENDSEC")){isentsection=0;continue;}
    if(!isentsection)continue;

    if((i==10)||(i==11)){d=str2x(s2);printtupled(i,d);continue;}
    if((i==20)||(i==21)){d=str2y(s2);printtupled(i,d);continue;}
    printtuples(i,s2);
  }

  fclose(fi);
  fliphorz=0;flipvert=0;tozero=0;
}

double getlastsize(char*s)
{ 
  if(!strcmp(s,"lastx"))return lastmaxx-lastminx;
  if(!strcmp(s,"lasty"))return lastmaxy-lastminy;
  return atof(s);
}

void help()
{ printf("dxfmerge - converts sequential LINE entities to a LWPOLYLINE\n"
         "          (does not support entities other than LINE and LWPOLYLINE)\n"
         "Usage: dxfmerge [options] <input file> [options] <input file> ...\n"
         "          outputs to stdout\n"
         "  -nocomment  disables adding commandline parameters as DXF comment (MUST be first argument)\n"
         "  -nohdr      suppress header and footer\n"
         "  -hdr        output header\n"
         "  -ftr        output footer\n"
         "  -f <fname>  output ENTITIES section of file named <fname> (may be omitted)\n"
         "  -s <n>      scaling factor\n"
         "  -sx <n>     scaling factor horizontal\n"
         "  -sy <n>     scaling factor vertical\n"
         "  -dx <mm>    shift horizontal\n"
         "  -dy <mm>    shift vertical\n"
         "  -ddx <mm>   shift horizontal from current shift position (can be used multiple times)\n"
         "  -ddy <mm>   shift vertical from current shift position\n"
         "              for -ddx and -ddy use 'lastx' or 'lasty' for the width or depth of previous file\n"
//       "  -o <fname>  output file name (default \"%s\")\n"
         "  -fh         flip horizontal\n"
         "  -fv         flip vertical\n"
         "  -z          if offset, align to zero before shifting\n"
         "  -r          reset scaling/shifting to defaults\n"
         "  -autor      auto-reset scaling/shifting for each file\n"
         "  -h, -help, --help\n"
         "              this help\n"
         "Arguments are processed sequentially, multiple scaling/shifting/flipping directives\n"
         "can be used, prepended to the files; their effects are stacking.\n"
        );
  exit(0);
}


int main(int argc,char*argv[])
{
  char*fni;
  int t;
  int autoreset=0;
  int nocomment=0;

//outfile="";
  if(argc<2)help();

  if(!strcmp(argv[1],"-nocomment"))nocomment=1;
  if(nocomment==0){
    char*sp;
    printf("999\nGenerated by:");
    for(t=0;t<argc;t++){
      while(sp=strchr(argv[t],'\n'))sp[0]=' ';
      while(sp=strchr(argv[t],'\r'))sp[0]=' ';
      sp=strchr(argv[t],' ');if(sp)printf(" \"%s\"",argv[t]);else printf(" %s",argv[t]);
    }
    printf("\n");
  }

  for(t=1;t<argc;t++){
    if(!strcmp(argv[t],"-h"))help();
    if(!strcmp(argv[t],"-help"))help();
    if(!strcmp(argv[t],"--help"))help();
    if(!strcmp(argv[t],"-nohdr")){shownhdr=1;shownftr=1;continue;}
    if(!strcmp(argv[t],"-hdr")){printheader(1);shownftr=1;continue;}
    if(!strcmp(argv[t],"-ftr")){printfooter(1);shownhdr=1;continue;}
    if(!strcmp(argv[t],"-r")){resetdefaults();continue;}
    if(!strcmp(argv[t],"-autor")){autoreset=1;continue;}

    if(!strcmp(argv[t],"-f")){continue;} // ignore -f argument, process filename further

    if(!strcmp(argv[t],"-s" )){t++;multix=atof(argv[t]);multiy=multix;continue;}
    if(!strcmp(argv[t],"-sx")){t++;multix=atof(argv[t]);continue;}
    if(!strcmp(argv[t],"-sy")){t++;multiy=atof(argv[t]);continue;}
    if(!strcmp(argv[t],"-dx")){t++;addx=atof(argv[t]);continue;}
    if(!strcmp(argv[t],"-dy")){t++;addy=atof(argv[t]);continue;}
    if(!strcmp(argv[t],"-ddx")){t++;addx+=getlastsize(argv[t]);continue;}
    if(!strcmp(argv[t],"-ddy")){t++;addy+=getlastsize(argv[t]);continue;}
//  if(!strcmp(argv[t],"-o" )){t++;outfile=argv[t];continue;}
    if(!strcmp(argv[t],"-fh")){fliphorz=1;continue;}
    if(!strcmp(argv[t],"-fv")){flipvert=1;continue;}
    if(!strcmp(argv[t],"-z" )){tozero=1;continue;}
    if(!strcmp(argv[t],"-nocomment" )){nocomment=1;continue;}
    if(!strncmp(argv[t],"-",1)){fprintf(stderr,"Unknown option: %s\n",argv[t]);return 1;}
//     fni=argv[t];

    if(!shownhdr)printheader(1);processfile(argv[t]);if(autoreset)resetdefaults();

 }

//  fi=fopen(fni,"r");if(!fi){fprintf(stderr,"Cannot open file '%s'.\n",fni);return 1;}
//  fo=stdout;
//  scanfile();
//  processfile();
//
//  fclose(fi);
  if(!shownftr)printfooter(1);
  return 0;
}