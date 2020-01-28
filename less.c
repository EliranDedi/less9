#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <bio.h>
#include <regexp.h>
#include <keyboard.h>
#include "dat.h"

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
newline(void)
{
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
drawprompt2(void)
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
fillrect(Rectangle r, Image *i, int f)
{
	if(badrect(r) || i == nil)
		sysfatal("fillrect: bad args: %R / %p", r, i);
	draw(screen, r, i, nil, ZP);
	if(f)
		flushimage(display, 1);
}

void
prompt3(void)
{
	Point p;
	Rectangle r;

	p = Bottom();
	r = Rpt(p, scrmax);
	fillrect(r, dwhite, 0);
	fillrect(r, sprompt, 1);

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

/* string() screws up tabs and newlines, eliminate that */
void
stringfmt(char *s)
{
	char *p;
	char fmt[1024];
	char *f;
	char tab[8];
	int tabs[256];
	int *t = tabs;
}

Rune
fnlookup(Rune r)
{
	int i;

	for(i = 0; i < nelem(kbdfn); ++i)
		if(kbdfn[i].r == r)
			return i;
	return -1;
}

void
unused(void)
{
	print("%C: unused\n", *kbd);
}

void
quit(void)
{
	print("done\n");
	exits("done");
}

void
redraw(void)
{
	draw(screen, screen->r, bigscreen, nil, bigpoint);
	flushimage(display, 1);
}

void
logcmd(Keyfn *fn)
{
	static int fd;
	char buf[32];

	if(fd < 0){
		snprint(buf, sizeof buf, "/tmp/log.%d", getpid());
		fd = create(buf, ORDWR, 0666);
	}
	snprint(buf, sizeof buf, "%ld\n", fn->r);
	fprint(fd, buf, strlen(buf));
}

void
scrolldown(void)
{
	int n;

	n = Dy(screen->r)/font->height/10;
	n = (n<1) ? 1 : n;
	print("%d\n", n);
	bigpoint.y += font->height * n;
	alignbigpoint();
 	draw(screen, screen->r, display->white, nil, bigpoint);
 	draw(screen, screen->r, bigscreen, nil, bigpoint);
}

void
scrollup(void)
{
	int n;

	n = Dy(screen->r)/font->height/10;
	n = (n<1) ? 1 : n;
	print("%d\n", n);
	bigpoint.y -= font->height * n;
	alignbigpoint();
 	draw(screen, screen->r, display->white, nil, bigpoint);
 	draw(screen, screen->r, bigscreen, nil, bigpoint);
}

void
search(void)
{
	prompt3();
	mode = Msearch;
}

void
drawprompt(Image *i, char *s)
{
	Point bottom;

	bottom = screen->r.min;
	bottom.y = screen->r.max.y - font->height;

	draw(screen, Rpt(bottom, screen->r.max), i, nil, ZP);
	flushimage(display, 1);
}

void
main(void)
{
	int fn;

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
Loop:
	event(&e);
	fn = fnlookup(*kbd);
	if(fn >= 0)
		kbdfn[fn].f();
	goto Loop;
}
