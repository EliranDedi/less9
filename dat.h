#define scrrect	screen->r
#define	scrmax	screen->r.max
#define	scrmin	screen->r.min
#define scrmaxx	scrmax.x
#define scrmaxy	scrmax.y
#define scrminx scrmin.x
#define scrminy	scrmin.y

typedef void Fn(void);
typedef struct	Keyfn	Keyfn;
typedef union	Color	Color;
struct Keyfn {
	Rune r;
	//void(*f)(void);
	Fn *f;
};
union Color {
	ulong u;
	Image *i;
};


void quit(void);
void unused(void);
void redraw(void);
void scrolldown(void);
void scrollup(void);
void search(void);
void mark(void);

Biobuf	*bio;
char	*stack[8192*4];
char	**sp = stack;
char	**spi;
char	*Bstr;
char	*dstr;
Point	drawp;
Point	ptmp;
Reprog	regex;
Event	e;
Mouse 	*m = &e.mouse;
char	kbuf[256];
char	*kbpos = kbuf;
char	*manpages[16];
char	*grepstrings[256];
int	*kbd = &e.kbdc;
int	grepfound[256];
int	*grepf = grepfound;
int	nfound;
int	grepped;
Point	greppoints[256];
int	reading;
int	currentline;
char	**grepl;
int	seekline;
int	printlinenumber = 0;

Image	*promptcolor;
Image	*green;
Image	*red;
Image	*yellow;
Image	*eprompt, *sprompt, *fprompt;
Image	*gray;
Image	*undo;
Image	*oneline;
Image	*bigscreen;
Rectangle	bigrect;
Point	bigpoint;
Point	cmd;

enum{
	Rioscroll = 26,
};

Keyfn kbdfn[]={
	Kdown,	scrolldown,
	Kup,	scrollup,
	Kleft,	unused,
	Kright,	unused,
	'q',	quit,
	Kesc,	quit,
	Kdel,	quit,
	'/',	search,
	'r',	redraw,
	'm',	mark,
};

char	*Search = "look for";
char	*Error	= "not found";
char	*Found	= "found";
char	*Newbuf = "name";
