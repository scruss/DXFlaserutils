// compile as:
// cc -o dxfsize dxfsize.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

double minx,maxx,miny,maxy;
long lines=0,polylines=0,crosses=0,dxfitems=0;
int showentities=0;
int listall=0;
int listfmt=0;
int listfmtsec=0;
int stripcomments=0;
int stripcrosses=0;
int ignoreerrors=0;
FILE*fi;
int dxferr=0;
int isdxf=0;
long maxl=100;

#define MAXXY 999999999

void scanfile()
{ char s1[512],s2[512],*sp;
  int i,t,iscross=0;
  int seclev=0;
  double d;
//  long lines=0;
  dxferr=0;
  isdxf=0;
  lines=0;polylines=0;crosses=0;dxfitems=0;
  minx=MAXXY;miny=MAXXY;maxx=-MAXXY;maxy=-MAXXY;
  while(!feof(fi)){
    fgets(s1,510,fi);
    fgets(s2,510,fi);sp=strchr(s2,0x0d);if(sp)sp[0]=0;sp=strchr(s2,0x0a);if(sp)sp[0]=0;
    if(ferror(fi)){printf("[Error %i at read]",ferror(fi));return;}
//printf("%li:",lines);
    //if(!ignoreerrors)if(i==0)if(strncmp(s1,"  0",3)){dxferr=1;return;}
    dxfitems++;
//    if(lines>maxl){printf("More lines than %li, aborting.",maxl);return;}
    i=atoi(s1);
    if(!strcmp(s2,"ENDSEC")){seclev--;}

    if(i==999){
       if(!strcmp(s2,"----CROSSBEGIN")){iscross=1;crosses++;if(stripcrosses)continue;}
       if(!strcmp(s2,"----CROSSEND")){iscross=0;if(stripcrosses)continue;}
       if(!strncmp(s2,"CROSSCOORD",10)){if(stripcrosses)continue;}
       if(stripcomments)continue;
    }

    if(i==0)if(showentities)printf("%s\n",s2);
    if(i==0){if(!strcmp(s2,"LINE")){isdxf=1;lines++;}else
             if(!strcmp(s2,"LWPOLYLINE")){polylines++;isdxf=1;}
            }
    if(listall){if(listfmt)if(i)printf("\t");
                if(listfmtsec){int t;for(t=0;t<seclev;t++)printf("\t");}
                printf("%3i|%s\n",i,s2);
               }
    if(!strcmp(s2,"SECTION")){seclev++;isdxf=1;}
    if((i==10)||(i==11)){d=atof(s2);if(d<minx)minx=d;if(d>maxx)maxx=d;}
    if((i==20)||(i==21)){d=atof(s2);if(d<miny)miny=d;if(d>maxy)maxy=d;}
  }
}

void entitiestoline()
{ char s1[512],s2[512],*sp;
  int i;
  int firstline=0;
  int iscross=0;
  if(!fi)return;
  rewind(fi);
  while(!feof(fi)){
    fgets(s1,510,fi);if(strlen(s1)==0)break;
    fgets(s2,510,fi);sp=strchr(s2,0x0d);if(sp)sp[0]=0;sp=strchr(s2,0x0a);if(sp)sp[0]=0;
    if(ferror(fi)){printf("[Error %i at read]",ferror(fi));return;}
    i=atoi(s1);
    if(!ignoreerrors)if(i==0)if(strncmp(s1,"  0",3)){dxferr=1;return;}
//    if(i==0)if(!strcmp(s2,"")){dxferr=1;return;}
    if(!strcmp(s2,"SECTION"))isdxf=1;
    //strip comments
    if(i==999){
       if(!strcmp(s2,"----CROSSBEGIN")){iscross=1;if(stripcrosses)continue;}
       if(!strcmp(s2,"----CROSSEND")){iscross=0;if(stripcrosses)continue;}
       if(!strncmp(s2,"CROSSCOORD",10)){if(stripcrosses)continue;}
       if(stripcomments)continue;
    }
    if(iscross)if(stripcrosses)continue;
    if((i==0)||(i==999))
             {if(firstline)printf("\n");}
        else printf("|");
    printf("%3i|%s",i,s2);
//            }
//    else {
//         }
    firstline++;
  }
  if(firstline)printf("\n");
}

void help()
{ printf("dxfsize - shows dimensions of OpenSCAD DXF file parts, optionally lists the entities in the file\n"
         "          (does not support entities other than LINE and LWPOLYLINE)\n"
         "Usage: dxfsize [options] <input file>\n"
         "          if no file specified, feed it from stdin\n"
         "  -vc         show DXF entities\n"
         "  -f          force operation even if it does not look like a DXF file\n"
         "  -nf         don't show filename\n"
         "  -fn         show filename (if no -fn or -nf, show filename unless -show is used)\n"
         "  -l          reformat group codes and data to single lines, replace \"|\" with \"\\n\" to undo\n"
         "  -ll         like -l, offsets parameters\n"
         "  -lll        like -ll, also offsets by section depth\n"
         "  -le         reformat group codes and data to single lines, whole entities per line\n"
         "  -sc         strip comments\n"
         "  -scr        strip crosses\n"
         "  -var <var>  show variable value\n"
         "              variables: minx, maxx, miny, maxy, xsize/width, ysize/height, lines, polys, crosses,\n"
         "              items (dxf line pairs), dims (dimensions), dimscoord (dimensions and starting coordinates),\n"
         "              alllines (lines+poly), linesnocross (lines-2*cross), alllinesnocross (lines+poly-2*cross) \n"
         "  -h, --help  this help\n"
         );
  exit(0);
}

int main(int argc,char*argv[])
{
  char*fni="";
  int showfile=1;
  int forceshowfile=0;
  int forcehidefile=0;
  int listentities=0;
  int doshow=0;
  int t,t1,t2;
  int fnames=0;

  if(argc<2)help();
  for(t=1;t<argc;t++){
    if(!strcmp(argv[t],"-h"))help();
    if(!strcmp(argv[t],"--help"))help();
    if(!strcmp(argv[t],"-vc")){showentities=1;continue;}
    if(!strcmp(argv[t],"-l")){listall=1;continue;}
    if(!strcmp(argv[t],"-ll")){listall=1;listfmt=1;continue;}
    if(!strcmp(argv[t],"-lll")){listall=1;listfmt=1;listfmtsec=1;continue;}
    if(!strcmp(argv[t],"-f")){ignoreerrors=1;continue;}
    if(!strcmp(argv[t],"-fn")){forceshowfile=1;continue;}
    if(!strcmp(argv[t],"-nf")){forcehidefile=1;continue;}
    if(!strcmp(argv[t],"-sc")){stripcomments=1;continue;}
    if(!strcmp(argv[t],"-scr")){stripcrosses=1;continue;}
    if(!strcmp(argv[t],"-le")){listentities=1;continue;}
    if(!strcmp(argv[t],"-var")){t++;doshow=1;continue;}
    if(!strcmp(argv[t],"-")){forcehidefile=1;continue;}
    if(!strncmp(argv[t],"-",1)){fprintf(stderr,"Unknown option: %s\n",argv[t]);return 1;}
    //fni=argv[t];
    fnames++;
  }

//  if(fnames>1)if(showfile==0)showfile=1;
//  if(showfile==2)showfile=0;
  if(fnames==1)if(listall||listentities||doshow)showfile=0; // don't show filename if -show is, for one name only 

  if(forceshowfile==1)showfile=1;
  if(forcehidefile==1)showfile=0;

  for(t1=1;t1<argc;t1++){
    if(!strncmp(argv[t1],"-",1))if(strlen(argv[t1])>1)
        {if(!strcmp(argv[t1],"-var"))t1++;
         continue;
        }
    fni=argv[t1];

    if(!strcmp(fni,"-"))fi=stdin;
    else
    if(fni[0]){
      fi=fopen(fni,"r");if(!fi){fprintf(stderr,"Cannot open file '%s'.\n",fni);continue;}
    }
//    else fi=stdin;

    if(showfile)
      {if(listentities||listall)printf("\n");
       printf("%s:",fni);
       if(listentities||listall)printf("\n");else printf("\t");}

    if(listentities){entitiestoline();}
    else scanfile();
    if(fi!=stdin)fclose(fi);
    if(!isdxf){printf("not a DXF file?\n");continue;}
    if(listentities)continue;
    if(listall)continue;
    if(!doshow) {printf("%.2fx%.2f",maxx-minx,maxy-miny);
                 printf(" viewport:%.2f,%.2f:%.2f,%.2f",minx,miny,maxx,maxy);
                 printf(" lines:%li polylines:%li crosses:%li items:%li",lines,polylines,crosses,dxfitems);
                 printf("\n");}
    if(doshow){
  //printf("SHOW:\n");
       int ocnt=0;
       for(t=1;t<argc;t++){
         if(strncmp(argv[t],"-var",4))continue;
         t++;
         if(ocnt)printf(" ");
         if(!strcmp(argv[t],"minx")){printf("%f",minx);continue;}
         if(!strcmp(argv[t],"miny")){printf("%f",maxx);continue;}
         if(!strcmp(argv[t],"maxx")){printf("%f",miny);continue;}
         if(!strcmp(argv[t],"maxy")){printf("%f",maxy);continue;}
         if(!strcmp(argv[t],"xsize")){printf("%f",maxx-minx);continue;}
         if(!strcmp(argv[t],"width")){printf("%f",maxx-minx);continue;}
         if(!strcmp(argv[t],"ysize")){printf("%f",maxy-miny);continue;}
         if(!strcmp(argv[t],"height")){printf("%f",maxy-miny);continue;}
         if(!strcmp(argv[t],"lines")){printf("%li",lines);continue;}
         if(!strcmp(argv[t],"polys")){printf("%li",polylines);continue;}
         if(!strcmp(argv[t],"items")){printf("%li",dxfitems);continue;}
         if(!strcmp(argv[t],"linesnocross")){printf("%li",lines-2*crosses);continue;}
         if(!strcmp(argv[t],"alllines")){printf("%li",lines+polylines);continue;}
         if(!strcmp(argv[t],"alllinesnocross")){printf("%li",lines+polylines-2*crosses);continue;}
         if(!strcmp(argv[t],"crosses")){printf("%li",crosses);continue;}
         if(!strcmp(argv[t],"dims")){printf("%fx%f",maxx-minx,maxy-miny);continue;}
         if(!strcmp(argv[t],"dimscoord")){printf("%fx%f at %f,%f",maxx-minx,maxy-miny,minx,miny);continue;}
         printf("[Unknown variable: '%s']",argv[t]);
         ocnt++;
       }
      printf("\n");
    }
  }
  return 0;
}