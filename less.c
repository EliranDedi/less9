#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <bio.h>
#include <regexp.h>
#include <keyboard.h>
#include "dat.h"

/*
DESCRIPTION
	less ripoff
USAGE
	argv0 file1 [file2 ...]
	cat file | argv0
	man | less
TODO
	add ed keys
		y - write to snarf buffer
		m - mark
	flags
		-m read from manpage
		-i read from stdin
NOT DONE
	everything
	searching
	use one big allocimage rather than screw around with positioning
	l - look, display only lines that match regex
	? - seek backwards
	read from stdin/file
	searching
	interactive mode
*/

void
eresized(int)
{
	sysfatal("eresized");
}

void
grep(void)
{
	char **p, **g;

	if(strcmp(kbuf, "") == 0){
		nfound = 1;
		return;
	}
	g = grepstrings;
	for(p = stack; p < sp; ++p)
		if(strstr(*p, kbuf) > nil){
			grepfound[g-grepstrings] = p-stack;
			*g++ = *p;
		}
	*g = nil;
	nfound = g-grepstrings-1;
	grepped = 1;
}

void
drawlines(char **lines)
{
	char **p;
	Point pt;
	char *str;

	if(lines == nil)
		sysfatal("lines: nil");
	if(nfound <= 0)
		sysfatal("drawlines: nfound <= 0, buf %s", kbuf);
	pt = screen->r.min;
	draw(undo, screen->r, screen, nil, ZP);
	draw(screen, screen->r, display->white, nil, ZP);
	for(p = lines; *p != nil; ++p){
		if(printlinenumber){
			str = smprint("%d%s", p-lines, *p);
			if(str == nil)
				sysfatal("drawlines: str: nil: %r");
		}
		else
			str = *p;
		string(screen, pt, display->black, pt, font, str);
		pt.y += font->height;
		if(printlinenumber)
			free(str);
	}
	flushimage(display, 1);
}

void
drawprompt(void)
{
	drawp = string(screen, drawp, display->black, drawp, font, "/");
	flushimage(display, 1);
}

Image*
allocstringline(char *s, Image *bgcol)
{
	Image *img;
	Point zp, sl;

	if(s == nil || bgcol == nil)
		sysfatal("allocstringline: s/img: nil");
	zp = ZP;
	sl = stringsize(font, s);
	sl.x += stringwidth(font, " "); /* delimiter for user input */
	img = allocimage(display, Rpt(zp,sl), screen->chan, 0, DWhite);
	if(img == nil)
		sysfatal("allocstring: img: nil");
	sl.x -= stringwidth(font, " "); /* delimiter for user input */
	draw(img, Rpt(zp,sl), bgcol, nil, ZP);
	string(img, ZP, display->black, ZP, font, s);
	return img;
}

void
kbdfilter(int kbd)
{
	switch(kbd){
	case Kdel:
		exits("Kdel");
	case Kack:
		break;
	}
}

void
prompt2(void)
{
	Point bottom, dp; /* drawprompt point */
	int kbd;

	bottom = screen->r.min;
	bottom.y = screen->r.max.y - font->height;
	draw(screen, Rpt(bottom, screen->r.max), fprompt, nil, ZP);
	flushimage(display, 1);
	dp = bottom;
	dp.x = fprompt->r.max.x + scrminx;
	//print("%P\n", dp);
	for(;;){
		kbd = ekbd();
		kbdfilter(kbd);
		if(kbd == Kesc){
			draw(screen, screen->r, bigscreen, nil, bigpoint);
			flushimage(display, 1);
			return;
		}
//		if(kbd == '?')
//			placeholder(fork and load argv0 manpage)
		if(kbd == '\n'){
			if(grepped && grepf-grepfound < nfound)
				++grepf;
			if(grepf-grepfound > nfound)
				grepf = &grepfound[nfound];
			if(grepped == 0)
				grep();
			bigpoint.y = *grepf * font->height;
			drawlines(grepstrings + *grepf);
			break;
		}
		if(kbd == Kbs){	/* screws up variable width fonts */
			dp.x -= stringwidth(font, " ");
			dp.x = dp.x <= fprompt->r.max.x + scrminx ? fprompt->r.max.x + scrminx : dp.x;
			draw(screen, Rpt(dp, screen->r.max), display->white, nil, ZP);
			flushimage(display, 1);
			if(kbpos > kbuf)
				--kbpos;
			*kbpos = 0;
			continue;
		}
		dp = stringn(screen, dp, display->black, dp, font, (char*)&kbd, 1);
		*kbpos++ = kbd;
		*kbpos = 0;
	}
	memset(kbuf, 0, sizeof kbuf);
	kbpos = kbuf;

}

void
alignbigpoint(void)
{
	if(bigpoint.y >= bigrect.max.y)
		bigpoint.y = bigrect.max.y;
	if(bigpoint.y < bigrect.min.y)
		bigpoint.y = bigrect.min.y;
}

void
eventloop(void)
{
	event(&e);
	switch(e.kbdc){
		case Kdel:
			exits("Kdel");
			break;
		case Kdown:
			bigpoint.y += font->height * Rioscroll;
			break;
		case Kup:
			bigpoint.y -= font->height * Rioscroll;
			break;
		case 'r':
			draw(screen, screen->r, bigscreen, nil, bigrect.min);
			flushimage(display, 1);
			return;
		case 'l':
			printlinenumber = 1;
			if(grepped){
				drawlines(grepstrings);
				flushimage(display, 1);
			}
			printlinenumber = 0;
			return;
		case '/':
			prompt2();
			drawlines(stack + *grepf);
			return;
		case 'n':
			if(grepped && grepf-grepfound < nfound)
				++grepf;
			if(grepf-grepfound > nfound)
				grepf = &grepfound[nfound];
			bigpoint.y = *grepf * font->height;
			drawlines(stack + *grepf);
			return;
		case '?': case 'p':
			if(grepped && grepf-1 > nil)
				--grepf;
			drawlines(stack + *grepf);
			return;
		default:
			return;
	}
	if(ptinrect(bigpoint, bigrect) == 0 || bigpoint.y < 0)
		alignbigpoint();
 	draw(screen, screen->r, display->white, nil, bigpoint);
 	draw(screen, screen->r, bigscreen, nil, bigpoint);
 	flushimage(display, 1);
}

void
guisetup(void)
{
	Point bottom;
	Point len;
	int i;
	char *smp;
	Image *cols[]={yellow,red,green};
	Image *tmp;

	bottom.x = screen->r.min.x;
	bottom.y = screen->r.max.y - font->height;

	green = allocimage(display, Rect(0,0,1,1), screen->chan, 1, DGreen);
	red = allocimage(display, Rect(0,0,1,1), screen->chan, 1, DRed);
	yellow = allocimage(display, Rect(0,0,1,1), screen->chan, 1, DYellow);
	if(green == nil || red == nil || yellow == nil)
		sysfatal("allocimage: colors: %r");
	eprompt = allocstringline(Error, red);
	sprompt = allocstringline(Search, yellow);
	fprompt = allocstringline(Found, green);
	bigpoint = screen->r.max;
	bigpoint.y = screen->r.min.y + font->height* (ushort)(sp-stack);
	bigrect = Rpt(screen->r.min, bigpoint);
	bigscreen = allocimage(display, bigrect, screen->chan, 0, DWhite);
	if(bigscreen == nil)
		sysfatal("allocimage: bigscreen: nil: %r");
	undo = allocimage(display, screen->r, screen->chan, 0, DWhite);
	if(undo == nil)
		sysfatal("allocimage: undo: nil: %r");

}

main(void)
{
	if(initdraw(0,0,0) < 0)
		sysfatal("draw:%r");
	bio = Bfdopen(0, OREAD);
	if(bio == nil)
		sysfatal("bio: %r");
	while(Bstr = Brdstr(bio, '\n', 1)){
		if(sp-stack >= nelem(stack))
			sysfatal("too many strings: max %d",sp-stack);
		*sp++ = strdup(Bstr);
		if(sp[-1] == nil)
			sysfatal("strdup: %r");
	}
	guisetup();
	bigpoint = bigrect.min;
	for(spi = stack; spi < sp; ++spi){
		string(bigscreen, bigpoint, display->black, bigpoint, font, *spi);
		bigpoint.y += font->height;
	}
	bigpoint.y = bigrect.min.y;
	draw(screen, screen->r, bigscreen, nil, bigpoint);
	flushimage(display, 1);
	einit(Ekeyboard);
	for(;;)
		eventloop();
}
