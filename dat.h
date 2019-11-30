#define	scrmax	screen->r.max
#define	scrmin	screen->r.min
#define scrmaxx	scrmax.x
#define scrmaxy	scrmax.y
#define scrminx scrmin.x
#define scrminy	scrmin.y

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
Image	*bigscreen;
Image	*undo;
Image	*oneline;
Rectangle	bigrect;
Point	bigpoint;

enum{
	Rioscroll = 26,
};

char	*Search = "look for";
char	*Error	= "not found";
char	*Found	= "found";
char	*Newbuf = "name";
