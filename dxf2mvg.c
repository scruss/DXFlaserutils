// compile as:
// cc -lm -o dxf2mvg dxf2mvg.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

FILE*fi,*fo;

#define DEFAULT_DPI 1000
int DPI=DEFAULT_DPI;
#define DEFAULT_STROKEWID 2
#define DEFAULT_OUTFILE "dxf.png"
#define DEFAULT_LINECOL "black"
#define DEFAULT_BGCOL "white"

#define DEFAULT_FONTSIZE 10
#define DEFAULT_FONTCOLOR "black"
#define COLLEN 32
#define DEFAULT_PRECISION 1
#define DEFAULT_FONTOFFS 3
int fontsize=DEFAULT_FONTSIZE;
int fontoffsx=DEFAULT_FONTOFFS;
int fontoffsy=DEFAULT_FONTOFFS;
char fontcolor[COLLEN+2]=DEFAULT_FONTCOLOR;
int showcoords=0;
int pixelcoords=0;
int coordcenterref=0;
char textxysep[COLLEN+2]="\n";
char textpartssep[COLLEN+2]="\n";
char sizexysep[COLLEN+2]="\n";
int canvasaddx=0;
int canvasaddy=0;
int canvasaddl=0;
int canvasaddt=0;
int coordprec=DEFAULT_PRECISION;

double crosscoordmaxx=0,crosscoordmaxy=0;

double multix=1.0;
double multiy=1.0;
double addx=0.0;
double addy=0.0;
double preaddx=0.0;
double preaddy=0.0;

char bgcol[COLLEN+2]=DEFAULT_BGCOL;
char linecol[COLLEN+2]=DEFAULT_LINECOL;
char crosscol[COLLEN+2]=DEFAULT_LINECOL;
char movecol1[COLLEN+2]="yellow";
char movecol2[COLLEN+2]="orange";
char movecol3[COLLEN+2]="green";
int strokewid=DEFAULT_STROKEWID;
int showmoves=0;
int showentities=0;
int fliphorz=0;
int flipvert=0;
int x_fliphorz=0;
int x_flipvert=0;
int tozero=0;
int x_tozero=0;
int showcrosscoord=0;   // show coordinates of cross mark centers
int showlinecoord=1;    // show coordinates of individual line endpoints - default 1, disabling in options
int showcrosssize=0;    // show dimensions of cross-center polylines 
int showonlycrosssize=0;// show dimensions, suppress coordinates
int showcrossdimhalf=0; // show half-size of dimensions, radius instead of diameter

int nomvg=0; // suppress MVG output, for other things
int outputcoordvars=0; // output coordinates of crosses as bash variables
int outputcoordcompact=0;

char movecol[COLLEN+2];
long linenum=0;
char*outfile=DEFAULT_OUTFILE;

int pixelsx,pixelsy;


double minx,maxx,miny,maxy;
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
  return d;
}
double str2y(char*s)
{ double d;
  d=atof(s);
  if(x_tozero)d=d-miny;
  d=(d+preaddy)*multiy;
  if(x_flipvert)d=maxy-d;
  d=d+addy;
  return d;
}

int mm2pix(double d)
{ return round(d/25.4*DPI);
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
//printf("%i:%s\n",i,s2);
    if((i==10)||(i==11)){d=str2x(s2);if(d<minx)minx=d;if(d>maxx)maxx=d;}
    if((i==20)||(i==21)){d=str2y(s2);if(d<miny)miny=d;if(d>maxy)maxy=d;}
    if(!strncmp(s2,"LINE",4))lines++;
    if(i==999){
      double d;
      if(!strncmp(s2,"CROSSCOORDMAXX:",15))crosscoordmaxx=atof(s2+15);
      if(!strncmp(s2,"CROSSCOORDMAXY:",15))crosscoordmaxy=atof(s2+15);
    }
  } 

  //overrride of maximum dimensions by crosses
  if(crosscoordmaxx>maxx)maxx=crosscoordmaxx;
  if(crosscoordmaxy>maxy)maxy=crosscoordmaxy;

  pixelsx=mm2pix(maxx)+strokewid;
  pixelsy=mm2pix(maxy)+strokewid;
  if(tozero){pixelsx-=mm2pix(minx);pixelsy-=mm2pix(miny);}
}

void printxycoord_o(double x,double y,int txtox,int txtoy,int showcoords,int showheigwid,double wid,double heig)
{ double showx,showy;
  char fmt[32]="";
  showx=x;showy=y;
  char scoord[64]="",swide[64]="",ssep[32]="";
  if(showcoords){
//    snprintf(fmt,31,"text %%i,%%i '%%.%if%%s%%.%if'\n",coordprec,coordprec);
    snprintf(fmt,31,"%%.%if%%s%%.%if",coordprec,coordprec);
//        999|CROSSCOORD:28.824385,19.141370 POLYDIM:57.648770,38.282740 len=1.500000 segnum=55 minmax=0.000000,-0.000000,57.648770,38.282740
 
    if(coordcenterref){showx=showx-maxx/2;showy=showy-maxy/2;}
    if(pixelcoords)snprintf(scoord,63,"%i%s%i",mm2pix(showx),textxysep,mm2pix(showy));
    else snprintf(scoord,63,fmt,showx,textxysep,showy);
  }
  if(showheigwid){
    snprintf(fmt,31,"W:%%.%if%%sH:%%.%if",coordprec,coordprec);
    if(pixelcoords)snprintf(swide,63,"W:%i%sH:%i",mm2pix(wid),sizexysep,mm2pix(heig));
    else snprintf(swide,63,fmt,wid,textxysep,heig);
  }
  if(showcoords)if(showheigwid)strcpy(ssep,textxysep);

  printf("text %i,%i '%s%s%s'\n",txtox+mm2pix(x)+canvasaddl,txtoy+mm2pix(y)+canvasaddt,scoord,ssep,swide);
/*
  if(showcoords){
    if(s[0]){strcat(fmt,"%s");}
    if(coordcenterref){showx=showx-maxx/2;showy=showy-maxy/2;}
    if(pixelcoords)printf("text %i,%i '%i%s%i%s'\n",txtox+mm2pix(x)+canvasaddl,txtoy+mm2pix(y)+canvasaddt,mm2pix(showx),textxysep,mm2pix(showy),s);
//            else printf("text %i,%i '%.2f%s%.2f'\n",mm2pix(x)+canvasaddl,mm2pix(y)+canvasaddt,showx,textxysep,showy);
              else printf(fmt,,s);
   }
  else{snprintf(fmt,31,"text %%i,%%i '%%s'\n");}
  }*/
}

void printxycoord(double x,double y)
{ printxycoord_o(x,y,fontoffsx,fontoffsy,1,0,0.0,0.0);
}
void printxycoordS(double x,double y,double wid,double heig)
{
//fprintf(stderr,"XXXXXXXXXXXXX\n");
  int coords=1;
  if(showonlycrosssize)coords=0;
  if(showcrossdimhalf){wid/=2;heig/=2;}
   printxycoord_o(x,y,fontoffsx,fontoffsy,coords,showcrosssize,wid,heig);
//fprintf(stderr,"XXXXXXXXXXXXX\n");
}

void printcoordvars(double showx,double showy,double wid,double heig,int n)
{ 
  if(showcrossdimhalf){wid/=2;heig/=2;}
  if(coordcenterref){showx=showx-maxx/2;showy=showy-maxy/2;}
  if(outputcoordcompact){
    if(pixelcoords){
      printf("co%02ix=%i;\t",n,mm2pix(showx));
      printf("co%02iy=%i;\t",n,mm2pix(showy));
      printf("co%02iw=%i;\t",n,mm2pix(wid));
      printf("co%02ih=%i;\t",n,mm2pix(heig));
    }else{
      printf("co%02ix=%f;\t",n,showx);
      printf("co%02iy=%f;\t",n,showy);
      printf("co%02iw=%f;\t",n,wid);
      printf("co%02ih=%f;\t",n,heig);
    }
    printf("#");
  }
  else{
    if(pixelcoords){
      printf("co%02ix=%i\n",n,mm2pix(showx));
      printf("co%02iy=%i\n",n,mm2pix(showy));
      printf("co%02iw=%i\n",n,mm2pix(wid));
      printf("co%02ih=%i\n",n,mm2pix(heig));
    }else{
      printf("co%02ix=%f\n",n,showx);
      printf("co%02iy=%f\n",n,showy);
      printf("co%02iw=%f\n",n,wid);
      printf("co%02ih=%f\n",n,heig);
    }
  }
  printf("\n");
}

void scanfile_printcoords()
{ char s1[512],s2[512],*sp;
  int i,t;
  int iscross=0;
  double d;
  double x,y;
  int crossnum=0;
  rewind(fi);
  if(!nomvg){printf("fill 'black'\n");
             printf("stroke 'black'\n");
             printf("stroke-width 0\n");
             printf("font-size %i\n",fontsize);
             printf("gravity northwest\n");}
  else if(outputcoordvars){
             printf("#DXF coordinate output variables\n");
             printf("#Dimensions:  ");if(showcrossdimhalf)printf("halved\n");else printf("full\n");
             printf("#Units:       ");if(pixelcoords)printf("pixels\n");else printf("millimeters\n");
             if(pixelcoords)
             printf("#DPI:         %i\n",DPI);
             printf("#CoordsRelTo: ");if(coordcenterref)printf("center\n");else printf("left-top\n");
             printf("\n");
             }
  while(!feof(fi)){
    fgets(s1,510,fi);sp=strchr(s1,0x0d);if(sp)sp[0]=0;
    fgets(s2,510,fi);sp=strchr(s2,0x0d);if(sp)sp[0]=0;sp=strchr(s2,0x0a);if(sp)sp[0]=0;
     if(ferror(fi)){fprintf(stderr,"[Error %i at read]",ferror(fi));return;}

    i=atoi(s1);
    if(i==999){
      if(!strcmp(s2,"----CROSSBEGIN"))iscross=1;else
      if(!strcmp(s2,"----CROSSEND"))iscross=0;
    }
    if(showcrosscoord){
      if(i==999){
        if(!strncmp("CROSSCOORD:",s2,11)){
          if(!nomvg)printf("#COMMENT: %s\n",s2);
          double cx,cy,wid=-1.1,heig=-1.1;
          char*sp,*sp1,*sp2,*sp3;
          sp=strchr(s2,':');sp1=strchr(s2,',');
          if(sp)if(sp1){cx=str2x(sp+1);cy=str2y(sp1+1);
             //printxycoord(cx,cy);
             if(sp1){sp2=strstr(sp1,"POLYDIM:");if(sp2){
               sp2=strchr(sp2,':');sp2++;
               sp3=strchr(sp2,',');if(sp3){
                 sp3++;wid=atof(sp2);heig=atof(sp3);
             }}}
             if(!nomvg)printxycoordS(cx,cy,wid,heig);
             if(nomvg)if(outputcoordvars)
                { if(!outputcoordcompact)printf("#%s\n",s2);
                  printcoordvars(cx,cy,wid,heig,crossnum);
                }
             crossnum++;
            }
        }
      }
    }
//fprintf(stderr,"%i %i %i %s\n",showlinecoord,iscross,i,s2);
    if(showlinecoord)if(!iscross)
    {
      if((i==10)||(i==11)){x=str2x(s2);}
      if((i==20)||(i==21)){y=str2y(s2);
                           if(!nomvg)printxycoord(x,y);
                           }
    }
  } 
  pixelsx=mm2pix(maxx)+strokewid;
  pixelsy=mm2pix(maxy)+strokewid;
}

void printmoveline(double x1,double y1,double x2,double y2)
{ //if(movecol[0]==0)return;
  if(!showmoves)return;
  if(x1==y1)if(x2==y2)return;
  //if(movecol1[0])
  printf("stroke '%s' line %li,%li %li,%li stroke '%s'\n",movecol,mm2pix(x1)+canvasaddl,mm2pix(y1)+canvasaddt,mm2pix(x2)+canvasaddl,mm2pix(y2)+canvasaddt,linecol);
  if(!strcmp(movecol,movecol1))strcpy(movecol,movecol2);else strcpy(movecol,movecol1);
  linenum++;
}

void printline(double x1,double y1,double x2,double y2)
{
  if(x1==y1)if(x2==y2)return;
  if(linenum==0)printmoveline(0,0,x1,x2);
  printf("            line %li,%li %li,%li\n",mm2pix(x1)+canvasaddl,mm2pix(y1)+canvasaddt,mm2pix(x2)+canvasaddl,mm2pix(y2)+canvasaddt);
  linenum++;
}



void processfile()
{ char s1[512],s2[512],*sp;
  int i,t;
  double x1=0,y1=0,x2=0,y2=0,xo=0,yo=0;
  int isline=0,ispoly=0;
  int polyseg=0;
  int iscross=0;

  x_fliphorz=fliphorz;x_flipvert=flipvert;x_tozero=tozero;

  printf("#box %f %f %f %f\n",minx,miny,maxx,maxy);
//  printf("#viewbox %li %li %li %li\n",mm2pix(minx),mm2pix(miny),mm2pix(maxx),mm2pix(maxy));
  printf("##xysize %lix%li\n",pixelsx+canvasaddx+canvasaddl,pixelsy+canvasaddy+canvasaddt);
  printf("##DPI %i\n",DPI);
  printf("##BGCOL %s\n",bgcol);
  if(outfile && outfile[0])printf("##outfile %s\n",outfile);
                      else printf("##outfile %s\n",DEFAULT_OUTFILE);
  printf("\n");
  printf("stroke %s stroke-width %i\n",linecol,strokewid);

  rewind(fi);
  while(!feof(fi)){
    fgets(s1,510,fi);sp=strchr(s1,0x0d);if(sp)sp[0]=0;sp=strchr(s1,0x0a);if(sp)sp[0]=0;
    fgets(s2,510,fi);sp=strchr(s2,0x0d);if(sp)sp[0]=0;sp=strchr(s2,0x0a);if(sp)sp[0]=0;
     if(ferror(fi)){fprintf(stderr,"[Error %i at read]",ferror(fi));return;}

    i=atoi(s1);
    if(i==0){if(showentities)printf("#%s\n",s2);
             if(!strcmp(s2,"LINE")){isline=1;ispoly=0;}
             else
             if(!strcmp(s2,"LWPOLYLINE")){ispoly=1;isline=0;polyseg=0;}
             else
             {isline=0;ispoly=0;}
            }
    if(i==999){
      if(!strcmp(s2,"----CROSSBEGIN")){iscross=1;printf("stroke '%s'\n",crosscol);}
      else
      if(!strcmp(s2,"----CROSSEND")){iscross=0;printf("stroke '%s'\n",linecol);}
    }


    if(!isline)if(!ispoly)continue;

    if(isline){
        if(i==10)x1=str2x(s2);else
        if(i==11)x2=str2x(s2);else
        if(i==20)y1=str2y(s2);else
        if(i==21)y2=str2y(s2);

        if(i==21){printmoveline(xo,yo,x1,y1);
                  printline(x1,y1,x2,y2);
                  xo=x2;yo=y2;}
      }
    else
    if(ispoly){
        if(i==10)x1=str2x(s2);else
        if(i==20)y1=str2y(s2);
        if(i==20){if(polyseg==0)printmoveline(xo,yo,x1,y1);
                           else printline(xo,yo,x1,y1);
                  xo=x1;yo=y1;
                  polyseg++;
                 }
      }

    //if(!strncmp(s,"LINE",4))
  }
  if(showmoves)printf("stroke '%s' line %li,%li %li,%li\n",movecol3,mm2pix(xo),mm2pix(yo),0,0);
//  printf("  -density 1000 -units PixelsPerInch \\\n");
//  printf("dxf.png\n");
}


void help(int forwrapper)
{ char s[32];
  if(!forwrapper)
  {strcpy(s,"dxf2mvg");
  printf("dxf2mvg - converts an OpenSCAD DXF file to a ImageMagick MVG vector file\n"
         "          (does not support entities other than LINE and LWPOLYLINE)\n");
  }else
  {strcpy(s,"dxf2png.sh");
  printf("dxf2png.sh - converts an OpenSCAD DXF file to a bitmap image file\n"
         "                  (does not support entities other than LINE and LWPOLYLINE)\n"
         "                  (calls dxf2mvg, uses a temporary ImageMagick MVG file in /tmp)\n");
  }
  printf("\nUsage: %s [options] <input file>\n",s);
  printf(" Modes:\n");
  if(forwrapper){
  printf("  -tomvg          output mvg to stdout instead of generating image (MUST be first argument)\n"
         "  -nocomment      disables adding commandline parameters as image comment (MUST be first argument)\n"
         "  -q              run quietly, don't say the output file (MUST be first argument, or second with nocomment)\n"
        );
  }
  printf(
         "  -coordvars      do not generate graphics, instead output coords of cross marks and object dimensions\n"
         "                  in bash variable syntax (MUST be first argument)\n"
         "  -coordvarsc     same, in compact form (MUST be first argument)\n"
         " Dimensions and orientation:\n"
         "  -s <n>          scaling factor\n"
         "  -sx <n>         scaling factor horizontal\n"
         "  -sy <n>         scaling factor vertical\n"
         "  -dx <mm>        shift horizontal\n"
         "  -dy <mm>        shift vertical\n"
         "  -fh             flip horizontal\n"
         "  -fv             flip vertical\n"
         "  -z              if offset, align to zero before shifting\n"
         " Output and appearance:\n"
         "  -o <fname>      output file name (default \"%s\")\n"
         "  -d <n>          DPI (default %i)\n"
         "  -lw <n>         line width (default %i)\n"
         "  -lcol <color>   line color (default %s)\n"
         "  -bgcol <color>  background color (default %s)\n"
         "  -lm             show movement lines\n"
         "  -vc             show DXF entities in MVG comments\n"
         "  -margx <n>      add n pixels to the right side\n"
         "  -margy <n>      add n pixels to the bottom side\n"
         "  -margt <n>      add n pixels to the top side\n"
         "  -margl <n>      add n pixels to the left side\n"
         " Annotations and coordinates:\n"
         "  -cp             show coordinates in pixels\n"
         "  -cm             show coordinates in millimeters, with %i decimal places\n"
         "  -cmprec <n>     show coordinates in millimeters, with n decimal places\n"
         "  -cc             show coordinates relative to object center (default: object left top)\n"
         "  -cr             show coordinates of cross marks\n"
         "  -cro            show only coordinates of cross marks, not of lines\n"
         "  -crdim          show coords and polyline width/height at cross mark\n"
         "  -crdimo         show only dimensions of cross marked object, no coords\n"
         "  -crdimh         show half of the dimensions value (radius vs diameter)\n"
         "  -crcol <color>  color of cross marks, if present\n"
         "  -cfs <size>     coordinates font size (default %i)\n"
         "  -cfcol <color>  coordinates font color (default \"%s\")\n"
         "  -csep <sep>     coordinates separated by character (default newline)\n"
         "  -dsep <sep>     dimensions separated by character (default newline)\n"
         " Other:\n"
         "  -h, -help, --help\n"
         "                  this help\n"

        ,DEFAULT_OUTFILE,DEFAULT_DPI,DEFAULT_STROKEWID,DEFAULT_LINECOL,DEFAULT_BGCOL,DEFAULT_PRECISION,DEFAULT_FONTSIZE,DEFAULT_FONTCOLOR);
  if(forwrapper)
  printf("\nUsage: %s -mvg <file> <tofile>\n"
         "                  generate image from MVG file <file>, output to image <tofile>\n",s);
  exit(0);
}

int main(int argc,char*argv[])
{
  //fi=stdin;
  char*fni;
  int t;

  outfile="";
  if(argc<2)help(0);
  strcpy(movecol,movecol1);
  for(t=1;t<argc;t++){
    if(!strcmp(argv[t],"-h"))help(0);
    if(!strcmp(argv[t],"-hp"))help(1);
    if(!strcmp(argv[t],"-help"))help(0);
    if(!strcmp(argv[t],"--help"))help(0);
    //sizing:
    if(!strcmp(argv[t],"-s" )){t++;multix=atof(argv[t]);multiy=multix;continue;}
    if(!strcmp(argv[t],"-sx")){t++;multix=atof(argv[t]);continue;}
    if(!strcmp(argv[t],"-sy")){t++;multiy=atof(argv[t]);continue;}
    if(!strcmp(argv[t],"-dx")){t++;addx=atof(argv[t]);continue;}
    if(!strcmp(argv[t],"-dy")){t++;addy=atof(argv[t]);continue;}
    if(!strcmp(argv[t],"-fh")){fliphorz=1;continue;}
    if(!strcmp(argv[t],"-fv")){flipvert=1;continue;}
    if(!strcmp(argv[t],"-z" )){tozero=1;continue;}
    //outputs:
    if(!strcmp(argv[t],"-o" )){t++;outfile=argv[t];continue;}
    if(!strcmp(argv[t],"-d" )){t++;DPI=atoi(argv[t]);continue;}
    if(!strcmp(argv[t],"-lw")){t++;strokewid=atoi(argv[t]);continue;}
    if(!strcmp(argv[t],"-lcol")){t++;strncpy(linecol,argv[t],COLLEN);continue;}
    if(!strcmp(argv[t],"-crcol")){t++;strncpy(crosscol,argv[t],COLLEN);continue;}
    if(!strcmp(argv[t],"-bgcol")){t++;strncpy(bgcol,argv[t],COLLEN);continue;}
    if(!strcmp(argv[t],"-lm")){showmoves=1;continue;}
    if(!strcmp(argv[t],"-vc")){showentities=1;continue;}
    if(!strcmp(argv[t],"-margx")){t++;canvasaddx=atoi(argv[t]);continue;}
    if(!strcmp(argv[t],"-margy")){t++;canvasaddy=atoi(argv[t]);continue;}
    if(!strcmp(argv[t],"-margt")){t++;canvasaddt=atoi(argv[t]);continue;}
    if(!strcmp(argv[t],"-margl")){t++;canvasaddl=atoi(argv[t]);continue;}
    //
    if(!strcmp(argv[t],"-cp" )){showcoords=1;pixelcoords=1;continue;}
    if(!strcmp(argv[t],"-cm" )){showcoords=1;continue;}
    if(!strcmp(argv[t],"-cmprec")){t++;showcoords=1;coordprec=atoi(argv[t]);continue;}
    if(!strcmp(argv[t],"-cc" )){coordcenterref=1;continue;}
    if(!strcmp(argv[t],"-cr" )){showcrosscoord=1;continue;}
    if(!strcmp(argv[t],"-cro" )){showcrosscoord=1;showlinecoord=0;continue;}
    if(!strcmp(argv[t],"-crdim")){showcrosssize=1;showcoords=1;continue;}
    if(!strcmp(argv[t],"-crdimo")){showcrosssize=1;showcoords=1;showonlycrosssize=1;continue;}
    if(!strcmp(argv[t],"-crdimh")){showcrosssize=1;showcoords=1;showcrossdimhalf=1;continue;}
    if(!strcmp(argv[t],"-cfs")){t++;fontsize=atoi(argv[t]);continue;}
    if(!strcmp(argv[t],"-cfcol")){t++;strncpy(fontcolor,argv[t],COLLEN);continue;}
    if(!strcmp(argv[t],"-csep")){t++;strncpy(textxysep,argv[t],COLLEN);continue;}
    if(!strcmp(argv[t],"-coordvars")){showcoords=1;nomvg=1;outputcoordvars=1;showcrosscoord=1;continue;}
    if(!strcmp(argv[t],"-coordvarsc")){showcoords=1;nomvg=1;outputcoordvars=1;showcrosscoord=1;outputcoordcompact=1;continue;}
    if(!strcmp(argv[t],"-nocomment")){continue;}
    if(!strncmp(argv[t],"-",1)){fprintf(stderr,"Unknown option: %s\n",argv[t]);return 1;}
    fni=argv[t];
  }

  fi=fopen(fni,"r");if(!fi){fprintf(stderr,"Cannot open file '%s'.\n",fni);return 1;}
  fo=stdout;
  scanfile();
  if(!nomvg)processfile();
  //if(showcoords)scanfile_printcoords(showcrosscoord,showlinecoord);
  if(showcoords)scanfile_printcoords();
//  printf("x: %f..%f\n",minx,maxx);
//  printf("y: %f..%f\n",miny,maxy);
//  printf("lines:%li\n",lines);

  fclose(fi);
  return 0;
}