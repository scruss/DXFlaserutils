// compile as:
// cc -lm -o dxfline2poly dxfline2poly.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

FILE*fi,*fo;

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
int nopoly=0;

int printouter=1;
int printinner=1;
int printcentercross=0;
int printonlycross=0;

#define DEFAULT_CROSSLEN 1.5;
#define DEFAULT_MINCROSSLEN 1;
double crosslen=DEFAULT_CROSSLEN;
double mincrosslen=DEFAULT_MINCROSSLEN;
int crosslenperc=0;


double minx,miny,maxx,maxy;
long lines=0;




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
  if(d==0.0)d=0.0;
  return d;
}
double str2y(char*s)
{ double d;
  d=atof(s);
  if(x_tozero)d=d-miny;
  d=(d+preaddy)*multiy;
  if(x_flipvert){d=maxy-d;if(x_tozero)d-=miny;}
  d=d+addy;
//  d=d+0.0;//get rid of signed zero
  if(d==0.0)d=0.0;
  return d;
}

#define MAXXY 999999999

void scanfile()
{ char s1[512],s2[512],*sp;
  int i,t;
  double d;
  minx=MAXXY;maxx=-MAXXY;
  miny=MAXXY;maxy=-MAXXY;
  rewind(fi);
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


#define MAXSEGMENTS 16384
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
  if(d<0.000001)if(d>-0.000001)d=0.0;//get rid of the fucking negative zeroes!
  printf("%f\n",d);
}


void printheader(int insection)
{
  if(!insection)printtuples(0,"SECTION");
  printtuples(2,"HEADER");
  printtuples(9,"$ACADVER");
  printtuples(1,"AC1015");
  printtuples(9,"$INSUNITS");
  printtuplei(70,4);
  printtuples(0,"ENDSEC");
  if(insection)printtuples(0,"SECTION");
}

void printline(double x1,double y1,double x2,double y2)
{
  printtuples(0,"LINE");
  printtuplei(8,0);
  printtupled(10,x1);
  printtupled(20,y1);
  printtupled(11,x2);
  printtupled(21,y2);
/*  0|LINE
  8|0
 10|37.3164
 11|43.0682
 20|9.95571
 21|11.6341
*/
}

void printcross(double x,double y,double size)
{ 
  printline(x-size,y,x+size,y);
  printline(x,y-size,x,y+size);
}

void printpolyline()
{ int t;
  int isouter=0;
  int doprint=0;
  int xsegnum;
  int xminx,xmaxx,xminy,xmaxy;
  double polyminx=999999,polyminy=999999,polymaxx=0,polymaxy=0;
  xsegnum=segnum;segnum=0;//to allow return without having to always zero this
  if(xsegnum==0)return;
  xminx=minx;xmaxx=maxx;xminy=miny;xmaxy=maxy;
  if(tozero){xminx-=minx;xmaxx-=minx;xminy-=miny;xmaxy-=maxy;}
//  if(printouter && printinner)doprint=1;
//  else{
    for(t=0;t<xsegnum;t++){
      //find segment limits
      if(segments[t][0]<polyminx)polyminx=segments[t][0];
      if(segments[t][1]<polyminy)polyminy=segments[t][1];
      if(segments[t][0]>polymaxx)polymaxx=segments[t][0];
      if(segments[t][1]>polymaxy)polymaxy=segments[t][1];
      //find if segment on edge
      if(segments[t][0]==xminx){isouter=1;}
      if(segments[t][0]==xmaxx){isouter=1;}
      if(segments[t][1]==xminy){isouter=1;}
      if(segments[t][1]==xmaxy){isouter=1;}
    }
//  }
  if(printouter)if( isouter)doprint=1;
  if(printinner)if(!isouter)doprint=1;
//printf("999\nprintouter:%i printinner:%i isouter:%i doprint:%i\n",printouter,printinner,isouter,doprint);
  if(doprint)
  {
  if(xsegnum==2)
    {
     if(printonlycross)return;
     printtuples(0,"LINE");
     printtuplei(8,0);
     printtupled(10,segments[0][0]);
     printtupled(11,segments[0][1]);
     printtupled(20,segments[1][0]);
     printtupled(21,segments[1][1]);
     return;
    }
  if(printcentercross)
  {  //TODO: allow variable cross length to match the size of the object
     char s[256];
     double crossx,crossy,crossl;
     printtuples(999,"----CROSSBEGIN");
     crossx=(polymaxx+polyminx)/2;
     crossy=(polymaxy+polyminy)/2;
     if(crosslenperc){
       double dx,dy,d;
       dx=polymaxx-polyminx;
       dy=polymaxy-polyminy;
       d=dx;if(d>dy)d=dy;
       crossl=d*crosslen/100;
       if(crossl<mincrosslen)crossl=mincrosslen;
     }
     else crossl=crosslen;
//     snprintf(s,256,"CROSSCOORD:%f,%f POLYDIM:%f,%f len=%f segnum=%i minmax=%f,%f,%f,%f",crossx,crossy,polymaxx-polyminx,polymaxy-polyminy,crossl,xsegnum,polyminx,polyminy,polymaxx,polymaxy);
     snprintf(s,256,"CROSSCOORD:%f,%f POLYDIM:%f,%f len=%.1f segnum=%i minmax=%.3f,%.3f,%.3f,%.3f",crossx,crossy,polymaxx-polyminx,polymaxy-polyminy,crossl,xsegnum,polyminx,polyminy,polymaxx,polymaxy);
     printtuples(999,s);
     printcross(crossx,crossy,crossl/2);
     printtuples(999,"----CROSSEND");
  }
  if(printonlycross)return;
  printtuples(0,"LWPOLYLINE");
  printtuplei(90,xsegnum);
  printtuplei(70,0);
  for(t=0;t<xsegnum;t++)
    {printtupled(10,segments[t][0]);
     printtupled(20,segments[t][1]);
    }
  }
  return;
}

void addsegment(double d1,double d2)
{ int d1a,d2a;
//printf("Addsegment:%f,%f\n",d1,d2);
  if(segnum>=MAXSEGMENTS-1)
   {//print segment sequence as polyline short of one segment, save that segment as a potential line to avoid losing a segment
    d1a=segments[segnum-1][0];
    d2a=segments[segnum-1][1];
    segnum--;
    printpolyline();
    segnum=0;//explicitly show
    segments[0][0]=d1a;
    segments[0][1]=d2a;
    segnum++;
   }
  segments[segnum][0]=d1;
  segments[segnum][1]=d2;
  segnum++;
}

void processfile()
{ char s1[512],s2[512],*sp,movecol[32]="yellow";
  int i,t;
  double x1=0,y1=0,x2=0,y2=0,xo=-9999999,yo=-9999999,d;
  int isline=0,ispoly=0,polyseg=0;
  int washeader=0;

  x_fliphorz=fliphorz;x_flipvert=flipvert;x_tozero=tozero;

  //printheader(0);

  if(printcentercross){
    double d;
    d=maxx;if(tozero)d-=minx;
    snprintf(s2,255,"CROSSCOORDMAXX:%f",d);
    printtuples(999,s2);
    d=maxy;if(tozero)d-=miny;
    snprintf(s2,255,"CROSSCOORDMAXY:%f",d);
    printtuples(999,s2);
  }

  rewind(fi);
  while(!feof(fi)){
    fgets(s1,510,fi);sp=strchr(s1,0x0d);if(sp)sp[0]=0;sp=strchr(s1,0x0a);if(sp)sp[0]=0;
    fgets(s2,510,fi);sp=strchr(s2,0x0d);if(sp)sp[0]=0;sp=strchr(s2,0x0a);if(sp)sp[0]=0;
     if(ferror(fi)){fprintf(stderr,"[Error %i at read]",ferror(fi));return;}

    i=atoi(s1);
    if(i==2)if(!strcmp(s2,"HEADER"))washeader=1;
    if(i==2)if(!strcmp(s2,"BLOCKS"))if(!washeader){printheader(1);washeader=1;}
    if(i==2)if(!strcmp(s2,"ENTITIES"))if(!washeader){printheader(1);washeader=1;}
    if(i==0){if(!strcmp(s2,"LINE")){isline=1;ispoly=0;continue;}
             if(!strcmp(s2,"LWPOLYLINE")){isline=0;ispoly=1;polyseg=0;continue;}
             isline=0;ispoly=0;printpolyline();
            }
    if(!isline)if(!ispoly)
               {printpolyline();
                if((i==10)||(i==11)){d=str2x(s2);printtupled(i,d);}
                else if((i==20)|(i==21)){d=str2y(s2);printtupled(i,d);}
                else printtuples(i,s2);
                continue;}

    if(isline){
      if(i==10)x1=str2x(s2);else
      if(i==11)x2=str2x(s2);else
      if(i==20)y1=str2y(s2);else
      if(i==21)y2=str2y(s2);

      if(i==21){
        if((xo!=x1)||(yo!=y1)||(nopoly))
                 {
                  printpolyline();
                  addsegment(x1,y1);
                 }
        addsegment(x2,y2);
        xo=x2;yo=y2;
       }
    }
    if(ispoly){
      if(!polyseg)printpolyline();
      if(i==10)x1=str2x(s2);else
      if(i==20){y1=str2y(s2);
                addsegment(x1,y1);
                //if(polyseg){//printpolyline();
                //            addsegment(x1,y1);}
                polyseg++;
               }
    }
  }
//  printpolyline();
}


void help()
{ printf("dxfline2poly - converts sequential LINE entities to a LWPOLYLINE\n"
         "          (does not support entities other than LINE and LWPOLYLINE)\n"
         "Usage: dxfline2poly [options] <input file>\n"
         "                 outputs to stdout\n"
         " Size and orientation:\n"
         "  -s <n>         scaling factor\n"
         "  -sx <n>        scaling factor horizontal\n"
         "  -sy <n>        scaling factor vertical\n"
         "  -dx <mm>       shift horizontal\n"
         "  -dy <mm>       shift vertical\n"
         "  -fh            flip horizontal\n"
         "  -fv            flip vertical\n"
         "  -z             if offset, align to zero before shifting\n"
         " Polyline selection:\n"
         "  -nopoly        do not form polylines\n"
         "  -noouter       do not print outline\n"
         "  -noinner       print only outline\n"
         " Annotations and cross marks:\n"
         "  -cross         print crosses in centers of polylines\n"
         "  -crossonly     print only center crosses, omit the lines\n"
         "  -crosslen <n>[%] sets the length of the cross, in mm or in percents of the\n"
         "                 shorter dimension of the polyline\n"
         "  -crossmin <n>  sets the minimum length of the cross, in mm\n"
         " Other:\n"
         "  -nocomment     disable embedding of commandline as comment\n"
         "  -h, -help, --help\n"
         "                 this help\n"
        );
  exit(0);
}


int main(int argc,char*argv[])
{
  char*fni;
  int t;
  int nocomment=0;

//outfile="";
  if(argc<2)help();
  for(t=1;t<argc;t++){
    if(!strcmp(argv[t],"-h"))help();
    if(!strcmp(argv[t],"-help"))help();
    if(!strcmp(argv[t],"--help"))help();
    if(!strcmp(argv[t],"-nocomment")){nocomment=0;continue;}
    //
    if(!strcmp(argv[t],"-s" )){t++;multix=atof(argv[t]);multiy=multix;continue;}
    if(!strcmp(argv[t],"-sx")){t++;multix=atof(argv[t]);continue;}
    if(!strcmp(argv[t],"-sy")){t++;multiy=atof(argv[t]);continue;}
    if(!strcmp(argv[t],"-dx")){t++;addx=atof(argv[t]);continue;}
    if(!strcmp(argv[t],"-dy")){t++;addy=atof(argv[t]);continue;}
    if(!strcmp(argv[t],"-fh")){fliphorz=1;continue;}
    if(!strcmp(argv[t],"-fv")){flipvert=1;continue;}
    if(!strcmp(argv[t],"-z" )){tozero=1;continue;}
    //
    if(!strcmp(argv[t],"-nopoly")){nopoly=1;continue;}
    if(!strcmp(argv[t],"-noouter")){printouter=0;continue;}
    if(!strcmp(argv[t],"-noinner")){printinner=0;continue;}
    //
    if(!strcmp(argv[t],"-cross")){printcentercross=1;continue;}
    if(!strcmp(argv[t],"-crossonly")){printcentercross=1;printonlycross=1;continue;}
    if(!strcmp(argv[t],"-crosslen")){t++;crosslen=atof(argv[t]);if(strchr(argv[t],'%'))crosslenperc=1;continue;}
    if(!strcmp(argv[t],"-crossmin")){t++;mincrosslen=atof(argv[t]);continue;}
    //
    if(!strncmp(argv[t],"-",1)){fprintf(stderr,"Unknown option: %s\n",argv[t]);return 1;}
    fni=argv[t];
  }

  fi=fopen(fni,"r");if(!fi){fprintf(stderr,"Cannot open file '%s'.\n",fni);return 1;}
  fo=stdout;
  if(nocomment==0){
    char*sp;
    printf("999\nCommandline:");
    for(t=0;t<argc;t++){
      while(sp=strchr(argv[t],'\n'))sp[0]=' ';
      while(sp=strchr(argv[t],'\r'))sp[0]=' ';
      sp=strchr(argv[t],' ');if(sp)printf(" \"%s\"",argv[t]);else printf(" %s",argv[t]);
    }
    printf("\n");
  }
  scanfile();
  processfile();

  fclose(fi);
  return 0;
}