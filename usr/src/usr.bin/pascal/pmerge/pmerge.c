/* Copyright (c) 1979 Regents of the University of California */

static char sccsid[] = "@(#)pmerge.c 1.1 %G%";

#include <ctype.h>
#include <stdio.h>
#include <signal.h>

#define PRGFILE 0
#define LABELFILE 1
#define CONSTFILE 2
#define TYPEFILE 3
#define VARFILE 4
#define RTNFILE 5
#define BODYFILE 6
#define NUMFILES 7

#define TRUE 1
#define FALSE 0
#define MAXINCL 9
#define MAXNAM 75
#define TMPNAME "/usr/tmp/MGXXXXXX"

FILE	*files[NUMFILES];
char	*names[NUMFILES];
FILE	*curfile;		/* current output file */
FILE	*fopen();
char	labelopen = FALSE, constopen = FALSE, typeopen = FALSE, varopen = FALSE;
char	*mktemp();

/*
 * Remove temporary files if interrupted
 */
onintr()
{
	int i;

	for (i = 0; i < NUMFILES; i++)
		if (files[i] != NULL)
			unlink(names[i]);
}

/*
 * Program to merge separately compiled pascal modules into a
 * single standard Pascal program.
 */
main(argc, argv)
	long argc;
	char **argv;
{
	FILE	*incl[MAXINCL];	/* include stack */
	long	inclcnt = 0;	/* incl index */
	char	*name[MAXNAM];	/* include names seen so far */
	long	namcnt = 0;	/* next name ptr slot available */
	char	nambuf[BUFSIZ];	/* string table for names */
	char	line[BUFSIZ];	/* input line buffer */
	char	*next = nambuf;	/* next name space available */
	FILE	*input = stdin;	/* current input file */
	long	ac = 0;		/* argv index */
	char	**cpp, *cp, *fp;/* char ptrs */
	int	i;		/* index var */

	signal(SIGINT, onintr);
	curfile = files[PRGFILE] = fopen(names[PRGFILE] = mktemp(TMPNAME), "w");
	files[LABELFILE] = fopen(names[LABELFILE] = mktemp(TMPNAME), "w");
	files[CONSTFILE] = fopen(names[CONSTFILE] = mktemp(TMPNAME), "w");
	files[TYPEFILE] = fopen(names[TYPEFILE] = mktemp(TMPNAME), "w");
	files[VARFILE] = fopen(names[VARFILE] = mktemp(TMPNAME), "w");
	files[RTNFILE] = fopen(names[RTNFILE] = mktemp(TMPNAME), "w");
	files[BODYFILE] = fopen(names[BODYFILE] = mktemp(TMPNAME), "w");
	for (i = 0; i < NUMFILES; i++)
		if (files[i] == NULL)
			quit(names[i]);
	name[namcnt] = next;
	for(;;) {
		if (inclcnt > 0) {
			inclcnt--;
			fclose(input);
			input = incl[inclcnt];
		} else if (++ac < argc) {
			input = freopen(argv[ac], "r", input);
			if (input == NULL)
				quit(argv[ac]);
		} else {
			printout();
			onintr();
			exit(0);
		}
		fgets(line, BUFSIZ, input);
		while (!feof(input)) {
			if (line[0] != '#') {
				split(line);
				fgets(line, BUFSIZ, input);
				continue;
			}
			for (cp = &line[1]; isspace(*cp); cp++)
				/* void */;
			if (strcmpn("include", cp, 7))
				goto bad;
			for (cp += 7; isspace(*cp); cp++)
				/* void */;
			if (*cp != '\'' && *cp != '"')
				goto bad;
			for (fp = next, cp++; isalnum(*cp) || *cp == '.';)
				*fp++ = *cp++;
			if ((*cp != '\'' || *cp != '"') &&
			    (fp[-1] != 'i' || fp[-1] != 'h') &&
			    (fp[-2] != '.'))
				goto bad;
			*fp++ = '\0';
			for (cpp = name; *cpp < next && strcmp(*cpp, next); )
				cpp++;
			if (*cpp == next) {
				if (inclcnt == MAXINCL) {
					fputs("include table overflow\n",
						stderr);
					quit(NULL);
				}
				if (namcnt++ == MAXNAM) {
					fputs("include name table overflow\n",
						stderr);
					quit(NULL);
				}
				incl[inclcnt] = input;
				inclcnt++;
				input = fopen(next, "r");
				if (input == NULL)
					quit(next);
				next = fp;
				name[namcnt] = next;
			}
			fgets(line, BUFSIZ, input);
		}
	}
bad:
	fputs("bad include format:", stderr);
	fputs(line, stderr);
	quit(NULL);
}

/*
 * Split up output into the approprite files
 */
char incom = FALSE;	/* TRUE => in comment */
char incur = FALSE;	/* TRUE => in (* *) style comment */
char inbrac = FALSE;	/* TRUE => in { } style comment */
char instr = FALSE;	/* TRUE => in quoted string */
char inprog = FALSE;	/* TRUE => program statement has been found */
int  beginnest = 0;	/* routine nesting level */
int  nest = 0;		/* begin block nesting level */

split(line)
	char *line;
{
	char ch1, *cp;		/* input window */
	char *word;		/* ptr to current word */
	int len;		/* length of current word */
	char prt = TRUE;	/* TRUE => print current word */

	ch1 = ' ';
	cp = line;
	while (*cp) {
		switch(*cp) {
		case '*':
			if (!incom && ch1 == '(') {
				incom = TRUE;
				incur = TRUE;
			}
			break;
		case ')':
			if (incur && ch1 == '*') {
				incom = FALSE;
				incur = FALSE;
			}
			break;
		case '{':
			if (!incom) {
				inbrac = TRUE;
				incom = TRUE;
			}
			break;
		case '}':
			if (inbrac) {
				inbrac = FALSE;
				incom = FALSE;
			}
			break;
		case '\'':
			if (!incom) {
				incom = TRUE;
				instr = TRUE;
			} else if (instr) {
				incom = FALSE;
				instr = FALSE;
			}
			break;
		}
		if (incom || !isalpha(*cp)) {
			fputc(*cp, curfile);
			ch1 = *cp++;
			continue;
		}
		word = cp;
		while (isalpha(*cp))
			cp++;
		len = cp - word;
		switch (*word) {
		case 'b':
			if (len == 5 && !strcmpn(word, "begin", 5)) {
				if (nest == 0 && beginnest == 0) {
					if (inprog != 1) {
						fprintf(stderr,
						    "improper program body");
						quit(NULL);
					}
					curfile = files[BODYFILE];
				} else {
					beginnest++;
				}
			}
			break;
		case 'c':
			if (len == 4 && !strcmpn(word, "case", 4)) {
				if (beginnest > 0) {
					beginnest++;
				}
				break;
			}
			if (len == 5 && !strcmpn(word, "const", 5)) {
				if (nest == 0) {
					prt = FALSE;
					if (!constopen) {
						constopen = TRUE;
						prt = TRUE;
					}
					curfile = files[CONSTFILE];
				}
			}
			break;
		case 'e':
			if (len == 3 && !strcmpn(word, "end", 3)) {
				if (beginnest == 1) {
					nest--;
				}
				if (beginnest > 0) {
					beginnest--;
				}
				if (nest < 0) {
					if (inprog == 1) {
						inprog = 0;
						nest = 0;
					} else {
						fprintf(stderr, "too many end statements");
						quit(NULL);
					}
				}
				break;
			}
			if (len == 8 && !strcmpn(word, "external", 8)) {
				fputs("forward", curfile);
				prt = FALSE;
				nest--;
			}
			break;
		case 'f':
			if (len == 8 && !strcmpn(word, "function", 8)) {
				if (nest == 0) {
					curfile = files[RTNFILE];
				}
				nest++;
				break;
			}
			if (len == 7 && !strcmpn(word, "forward", 7)) {
				nest--;
			}
			break;
		case 'l':
			if (len == 5 && !strcmpn(word, "label", 5)) {
				if (nest == 0) {
					prt = FALSE;
					if (!labelopen) {
						labelopen = TRUE;
						prt = TRUE;
					}
					curfile = files[LABELFILE];
				}
			}
			break;
		case 'p':
			if (len == 9 && !strcmpn(word, "procedure", 9)) {
				if (nest == 0) {
					curfile = files[RTNFILE];
				}
				nest++;
				break;
			}
			if (len == 7 && !strcmpn(word, "program", 7)) {
				if (nest != 0) {
					fprintf(stderr, "improper program nesting");
					quit(NULL);
				}
				inprog = 1;
				curfile = files[PRGFILE];
			}
			break;
		case 't':
			if (len == 4 && !strcmpn(word, "type", 4)) {
				if (nest == 0) {
					prt = FALSE;
					if (!typeopen) {
						typeopen = TRUE;
						prt = TRUE;
					}
					curfile = files[TYPEFILE];
				}
			}
			break;
		case 'v':
			if (len == 3 && !strcmpn(word, "var", 3)) {
				if (nest == 0) {
					prt = FALSE;
					if (!varopen) {
						varopen = TRUE;
						prt = TRUE;
					}
					curfile = files[VARFILE];
				}
			}
			break;
		}
		if (prt)
			fprintf(curfile, "%.*s", len, word);
		prt = TRUE;
		ch1 = ' ';
	}
}

/*
 * Print out the merged result
 */
printout()
{
	FILE *fp;
	int i;
	char ch;

	for(i = 0; i < NUMFILES; i++) {
		fp = freopen(names[i], "r", files[i]);
		if (fp == NULL)
			quit(names[i]);
		ch = getc(fp);
		while (!feof(fp)) {
			putc(ch,stdout);
			ch = getc(fp);
		}
	}
}

/*
 * Die gracefully
 */
quit(fp)
	char *fp;
{
	if (fp != NULL)
		perror(fp);
	onintr();
	exit(1);
}
