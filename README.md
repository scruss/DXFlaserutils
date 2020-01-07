# DXFlaserutils
tools to clean/modify [OpenSCAD](https://www.openscad.org/) (and other) DXF output for reuse. Originally intended for driving simple laser cutters.

## Source
This is a verbatim copy of

    http://www.shaddack.twibright.com/projects/sw_DXFlaserutils/dxflaserutils.tar.gz
    
from 2015. The website is no longer available and I had the archive lurking on my computer.

## Author
Unknown: no obvious credits are in the source or what's left of the website on the Internet Archive. I (*scruss*) did **not** write this, but I have found the code useful on occasion. You might, too.

## Licence
Also unknown. The documentation notes:

> … *No warranty. License to be determined, something open, no idea yet what flavor.*

## Building
A few hints are in the source code:

    cc -lm -o dxf2mvg dxf2mvg.c
    cc -lm -o dxfline2poly dxfline2poly.c
    cc -o dxfsize dxfsize.c

`dxflist` is a symbolic link to `dxfsize` on systems that support it.

## Documentation
is archived here: [OpenSCAD-DXF-MoshiDraw tool](https://web.archive.org/web/20190408092530/http://www.shaddack.twibright.com/projects/sw_DXFlaserutils/)

## Potential use case
`dxfline2poly` can be used to transform OpenSCAD DXF output to a form that the [Fiona](https://github.com/Toblerity/Fiona) spatial library can understand. By default, `fiona dump file.dxf` produces a very simple JSON stream that can be put to further use more easily than trying to understand DXF.

## No issues/pull requests accepted
Please fork and maintain your own derivatives.
