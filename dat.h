#define	scrrect	screen->r
#define	scrmax	screen->r.max
#define	scrmin	screen->r.min
#define scrmaxx	scrmax.x
#define scrmaxy	scrmax.y
#define scrminx scrmin.x
#define scrminy	scrmin.y
#define dwhite	display->white
#define dblack	display->black
#define Bottom() Pt(scrminx, scrmaxy - font->height)

typedef void Fn(void);
typedef struct	Keyfn	Keyfn;
struct Keyfn {
	Rune r;
	Fn *f;
};
typedef struct	Strimg	Strimg;
struct Strimg {
	Image *i;
	ulong c;
	char *s;
};

enum {
	Mnormal,
	Msearch,
};

void	quit(void);
void	unused(void);
void	redraw(void);
void	scrolldown(void);
void	scrollup(void);
void	search(void);
void	mark(void);
void	newline(void);

int	dbg;
int	mode;
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
Point	uinput;

Keyfn kbdfn[][32]={
	[Mnormal]{
		Kdown,	scrolldown,
		Kup,	scrollup,
		Kleft,	unused,
		Kright,	unused,
		'q',	quit,
		Kesc,	quit,
		Kdel,	quit,
		'/',	search,
		'r',	redraw,
		'\n',	newline,
		nil,	nil,
	},
	[Msearch]{
		Kdel,	quit,
		Kesc,	quit,
		nil,	nil,
	},
};

Strimg strimg[]={
	nil, DYellow,	"look for",
	nil, DRed,	"not found",
	nil, DGreen,	"found",
};

char	*Search = "look for";
char	*Error	= "not found";
char	*Found	= "found";
char	*Newbuf = "name";
