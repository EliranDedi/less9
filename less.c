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
drawstr(Point p, char *s, int bw)
{
	string(screen, p, bw ? dwhite : dblack, ZP, font, s);
}

void
backspace(void)
{
	Point ss;
	int c;

	c = *ubufpos;
	if(ubufpos > ubuffer)
		*--ubufpos = 0;
	ss = stringsize(font, ubuffer);
}

void
drawfrom(Point p)
{
 	draw(screen, screen->r, dwhite, nil, ZP);
 	draw(screen, screen->r, bigscreen, nil, p);
	flushimage(display, 1);
}

Point
linetop(int n)
{
	Point p;

	p.x = scrminx;
	p.y = scrminy + n * font->height;
	return p;
}

void
badsearch(void)
{
}


void
newline(void)
{
	int i;
	
	if(mode != Msearch)
		sysfatal("newline: mode: %d, expected %d", mode, Msearch);
	if(lastsearch == nil || strcmp(ubuffer, lastsearch)){
		free(lastsearch);
		lastsearch = strdup(ubuffer);
	}
	mode = Mnormal;
	for(i = match;  i < sp-stack; ++i)
		if(strstr(stack[i], ubuffer))
			break;
	if(i >= sp-stack){
		drawfrom(linetop(match));
		return;
	}
	match = i;
	drawfrom(linetop(match));
	++match;
	memset(ubuffer, 0, sizeof ubuffer);
	ubufpos = ubuffer;
}

void
mkgfx(void)
{
	Strimg *si;
	Point ss;
	Image *tmp;

	for(si = strimg; si-strimg < nelem(strimg); ++si){
		ss = stringsize(font, si->s);
		si->i = allocimage(display, Rpt(ZP,ss), screen->chan, 0, DWhite);
		tmp = allocimage(display, Rect(0,0,1,1), screen->chan, 1, si->c);
		if(si->i == nil || tmp == nil)
			sysfatal("mkgfx: %r");
		stringbg(si->i, ZP, dblack, ZP, font, si->s, tmp, ZP);
		freeimage(tmp);
	}
}

void
fillrect(Rectangle r, Image *i, int f)
{
	if(badrect(r) || i == nil)
		sysfatal("fillrect: bad args: %R / %p / %p %p", r, i, getcallerpc(&r));
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
	r = Rpt(p, screen->r.max);
	fillrect(r, dwhite, 0);
	fillrect(r, strimg[Psearch].i, 1);
	uinput = p;
	uinput.x += Dx(strimg[Psearch].i->r) + stringwidth(font, " ");
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
	bigpoint = screen->r.max;
	bigpoint.y = screen->r.min.y + font->height* (ushort)(sp-stack);
	bigrect = Rpt(screen->r.min, bigpoint);
	bigscreen = allocimage(display, bigrect, screen->chan, 0, DWhite);
	if(bigscreen == nil)
		sysfatal("allocimage: bigscreen: nil: %r");
//	undo = allocimage(display, screen->r, screen->chan, 0, DWhite);
//	if(undo == nil)
//		sysfatal("allocimage: undo: nil: %r");

}

Keyfn*
fnlookup(Rune r)
{
	Keyfn *k;

	for(k = kbdfn[mode]; k->r; ++k)
		if(k->r == r)
			return k;
	return nil;
}

void
unused(void)
{
	print("%C: unused\n", *kbd);
}

void
quit(void)
{
	print("done %d\n", mode);
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
	bigpoint.y -= font->height * n;
	alignbigpoint();
 	draw(screen, screen->r, display->white, nil, bigpoint);
 	draw(screen, screen->r, bigscreen, nil, bigpoint);
}

void
search(void)
{
	Point p;
	Rectangle r;
	static int didprompt, didgfx;

	p = Bottom();
	r = Rpt(p, screen->r.max);
	fillrect(r, dwhite, 0);
	fillrect(r, strimg[Psearch].i, 1);
	uinput = p;
	uinput.x += Dx(strimg[Psearch].i->r) + stringwidth(font, " ");
	mode = Msearch;
}

Point
drawchar(Point p, int *c)
{
	return stringn(screen, p, display->black, ZP, font, (char*)c, 1);
}

void
cancel(void)
{
	mode = Mnormal;
 	draw(screen, screen->r, dwhite, nil, bigpoint);
 	draw(screen, screen->r, bigscreen, nil, bigpoint);
	flushimage(display, 1);
}

void
putbuf(void)
{
	*ubufpos++ = *kbd;
	*ubufpos = 0;
}

void
main(void)
{
	Keyfn *kf;

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
	mkgfx();
	einit(Ekeyboard);
Loop:
	event(&e);
	kf = fnlookup(*kbd);
	if(kf != nil){
//		print("%p %d\n", kf, kf-kbdfn[mode]);
		kf->f();
		goto Loop;
	}
	if(mode != Mnormal){
		uinput = drawchar(uinput, kbd);
		putbuf();
	}
	goto Loop;
}
