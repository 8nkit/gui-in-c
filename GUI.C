/* Refer Appendix B in the book if you want to create standalone
   EXE file of this program during compilation */

# include "dos.h"
# include "stdio.h"
# include "graphics.h"
# include "alloc.h"

# define NO 0

int mx, my, mouse_button, maxx, maxy ;

/* array for storing icon image */
unsigned char icon_image[40][32] ;

/* flags to determine whether icon has been boxed */
int icon_flag[6] ;

/* array for storing help messages */
char *messages[] = {
						"Delete a file",
						"Rename a file",
						"Compare two files",
						"Encrypt a file",
						"Decrypt a file",
						"Exit to dos"
				   } ;

main()
{
	int gm = CGAC3, gd = CGA, selected_icon = 0, area ;
	void *image ;

	/* load the mouse driver */
	load_mousedriver() ;

	/* reset the mouse */
	reset_mouse() ;

	/* initialise graphics system in CGA medium resolution graphics mode */
	initgraph ( &gd, &gm, "c:\\tc\\bgi" ) ;

	/* get maximum x, y coordinates of the screen */
	maxx = getmaxx() ;
	maxy = getmaxy() ;

	/* prepare the opening screen */
	rectangle ( 0, 0, maxx, maxy ) ;
	rectangle ( 0, 25, maxx, maxy - 25 ) ;
	settextjustify ( CENTER_TEXT, TOP_TEXT ) ;
	outtextxy ( 160, 9, "Graphical User Interface" ) ;

	/* load icons from disk and display them on screen */
	display_icons() ;

	/* allocate memory and check if successful */
	area = imagesize ( 1, 26, 318, 173 ) ;
	image = malloc ( area ) ;
	if ( image == NULL )
	{
		outtextxy ( 160, 184, "Insufficient memory! Press any key..." ) ;
		getch() ;

		exit ( 1 ) ;
	}

	/* save the screen contents where icons are drawn */
	getimage ( 1, 26, 318, 173, image ) ;

	while ( 1 )
	{
		/* select icon */
		selected_icon = getresponse() ;

		/* clear the screen where icons are drawn */
		setviewport ( 1, 26, 318, 173, 1 ) ;
		clearviewport() ;

		/* reset viewport back to normal */
		setviewport ( 0, 0, maxx, maxy, 1 ) ;

		/* set text justification for displaying messages */
		settextjustify ( LEFT_TEXT, TOP_TEXT ) ;

		/* branch to appropriate function */
		switch ( selected_icon )
		{
			case 1 :
				deletefile() ;
				break ;

			case 2 :
				renamefile() ;
				break ;

			case 3 :
				comparefile() ;
				break ;

			case 4 :
				encryptfile() ;
				break ;

			case 5 :
				decryptfile() ;
				break ;

			case 6 :
				closegraph() ;
				restorecrtmode() ;
				exit ( 0 ) ;
		}

		/* restore the icons from memory */
		putimage ( 1, 26, image, COPY_PUT ) ;

		/* set text justification back to original */
		settextjustify ( CENTER_TEXT, TOP_TEXT ) ;
	}
}

/* loads mouse driver in memory, if not already loaded */
load_mousedriver()
{
	unsigned char far *ms ;

	/* convert the segment:offset address from IVT into a far pointer */
	ms = MK_FP ( peek ( 0, 0x33 * 4 + 2 ), peek ( 0, 0x33 * 4 ) ) ;

	/* check if mouse driver isn't already loaded */
	if ( ms == NULL || *ms == 0xcf )
		system ( "wittyms -p2" ) ;
}

/* initialises the mouse driver */
reset_mouse()
{
	union REGS i, o ;

	/* issue interrupt */
	i.x.ax = 0 ;
	int86 ( 0x33, &i, &o ) ;

	/* if unable to reset mouse */
	if ( o.x.ax == 0 )
	{
		gotoxy ( 20, 15 ) ;
		printf ( "Mouse not available! Press any key..." ) ;
		fflush ( stdin ) ;
		getch() ;

		exit ( 2 ) ;
	}
}

/* loads icons from disk and displays them on screen */
display_icons()
{
	load_icon ( "gdel.icn", 51, 45 ) ;
	load_icon ( "gren.icn", 141, 45 ) ;
	load_icon ( "gcmp.icn", 231, 45 ) ;
	load_icon ( "genc.icn", 51, 125 ) ;
	load_icon ( "gdec.icn", 141, 125 ) ;
	load_icon ( "gdos.icn", 231, 125 ) ;
}

/* loads an icon from file and displays it on screen */
load_icon ( char *file, int x, int y )
{
	int xx, yy ;
	FILE *fp ;

	/* open the specified icon file */
	fp = fopen ( file, "rb" ) ;

	/* if unable to open file */
	if ( fp == NULL )
	{
		outtextxy ( 160, 184, "Unable to open file! Press any key..." ) ;
		getch() ;

		/* shut down the graphics system */
		closegraph() ;
		restorecrtmode() ;

		exit ( 3 ) ;
	}

	/* read the bit image of icon */
	fread ( icon_image, sizeof ( icon_image ), 1, fp ) ;

	/* output the icon on the screen */
	for ( xx = x ; xx < x + 40 ; xx++ )
	{
		for ( yy = y ; yy < y + 32 ; yy++ )
			putpixel ( xx, yy, icon_image[xx - x][yy - y] ) ;
	}

	fclose ( fp ) ;
}

/* displays the mouse pointer */
show_ptr()
{
	union REGS i, o ;

	i.x.ax = 1 ;  /* service number */
	int86 ( 0x33, &i, &o ) ;  /* issue interrupt */
}

/* hides the mouse pointer */
hide_ptr()
{
	union REGS i, o ;

	i.x.ax = 2 ;  /* service number */
	int86 ( 0x33, &i, &o ) ;  /* issue interrupt */
}

/* checks which icon is selected */
getresponse()
{
	int icon_no, selected = NO ;

	/* display the mouse pointer */
	show_ptr() ;

	icon_no = 1 ;

	while ( !selected )
	{
		/* get current mouse position and status of mouse buttons */
		get_position() ;

		/* check whether any of the 6 icons have been selected */
		for ( icon_no = 1 ; icon_no <= 6  ; icon_no++ )
		{
			selected = check_icon ( icon_no ) ;
			if ( selected )
				break ;
		}
	}

	/* hide the mouse pointer */
	hide_ptr() ;

	/* return the selected icon number */
	return ( icon_no ) ;
}

/* gets current coordinates of mouse pointer and status of mouse buttons */
get_position()
{
	union REGS i, o ;

	i.x.ax = 3 ;  /* service number */
	int86 ( 0x33, &i, &o ) ;  /* issue interrupt */

	mx = o.x.cx ;  /* x coordinate */
	my = o.x.dx ;  /* y coordinate */
	mouse_button = o.x.bx & 1 ;  /* store status of mouse buttons */
}

/* checks whether an icon has been selected */
check_icon ( int icon_no )
{
	int sx, sy, ex, ey ;

	/* calculate coordinates of the icon */
	if ( icon_no <= 3 )
	{
		sx = ( 51 + ( icon_no - 1 ) * 90 ) * 2 ;
		sy = 45 ;
	}
	else
	{
		sx = ( 51 + ( icon_no - 4 ) * 90 ) * 2 ;
		sy = 125 ;
	}
	ex = sx + ( 40 * 2 ) ;
	ey = sy + 32 ;

	/* if mouse pointer lies within the boundaries of the icon */
	if ( mx >= sx && mx <= ex && my >= sy && my <= ey )
	{
		/* if box has not been drawn around the icon */
		if ( icon_flag[icon_no - 1] == 0 )
		{
			/* draw a box around the icon */
			icon_box ( sx, ex, sy, ey, 3 ) ;

			/* set flag to indicate that the icon has been boxed */
			icon_flag[icon_no - 1] = 1 ;

			/* display help message */
			outtextxy ( 160, 184, messages[icon_no - 1] ) ;
		}

		/* if the left button of the mouse is pressed */
		if ( mouse_button == 1 )
		{
			/* erase the box around the icon */
			icon_box ( sx, ex, sy, ey, 0 ) ;

			/* set flag to indicate that the icon is no longer boxed */
			icon_flag[icon_no - 1] = 0 ;

			/* erase help message */
			cleartext() ;

			return ( 1 ) ;
		}
	}
	else
	{
		/* if box has been drawn around the icon */
		if ( icon_flag[icon_no - 1] == 1 )
		{
			/* erase the box around the icon */
			icon_box ( sx, ex, sy, ey, 0 ) ;

			/* reset flag */
			icon_flag[icon_no - 1] = 0 ;

			/* erase the help message */
			cleartext() ;
		}
	}

	return ( 0 ) ;
}

/* draws/erases the icon box */
icon_box ( int sx, int ex, int sy, int ey, int color )
{
	hide_ptr() ;
	setcolor ( color ) ;
	rectangle ( sx / 2 - 5, sy - 5, ex / 2 + 5, ey + 5 ) ;
	show_ptr() ;
}

/* erases the row used for displaying messages */
cleartext()
{
	int col ;

	for ( col = 2 ; col <= 39 ; col++ )
	{
		gotoxy ( col, 24 ) ;
		printf ( " " ) ;
	}
}

/* deletes a file */
deletefile()
{
	union REGS i, o ;
	char filename[30] ;

	/* collect file name */
	setcolor ( 3 ) ;
	outtextxy ( 4 * 8, 6 * 8, "Enter file name:") ;
	gotoxy ( 5, 9  ) ;
	fflush ( stdin ) ;
	gets ( filename ) ;

	i.h.ah = 65 ;  /* service number */
	i.x.dx = ( unsigned int ) filename ;  /* store base address */

	/* issue interrupt for deleting file */
	intdos ( &i, &o ) ;

	/* check if successful in deleting file */
	if ( o.x.cflag == 0 )
		outtextxy ( 4 * 8, 11 * 8, "File was successfully deleted!" ) ;
	else
		outtextxy ( 4 * 8, 11 * 8, "Unable to delete file!" ) ;

	outtextxy ( 32, 184, "Press any key..." ) ;
	fflush ( stdin ) ;
	getch() ;
	cleartext() ;
}

/* renames a file */
renamefile()
{
	union REGS i, o ;
	char old[30], new[30] ;

	/* collect old file name */
	setcolor ( 3 ) ;
	outtextxy ( 4 * 8, 6 * 8, "Enter old file name:") ;
	gotoxy ( 5, 9 ) ;
	fflush ( stdin ) ;
	gets ( old ) ;

	/* collect new file name */
	outtextxy ( 4 * 8, 11 * 8, "Enter new file name:") ;
	gotoxy ( 5, 14 ) ;
	fflush ( stdin ) ;
	gets ( new ) ;

	i.h.ah = 86 ;  /* service number */
	i.x.dx = ( int ) old ;
	i.x.di = ( int ) new ;

	/* issue interrupt for renaming file */
	intdos ( &i, &o ) ;

	/* check if successful in renaming file */
	if ( o.x.cflag == 0 )
		outtextxy ( 4 * 8, 16 * 8, "File was successfully renamed!" ) ;
	else
		outtextxy ( 4 * 8, 16 * 8, "Unable to rename file!" ) ;

	outtextxy ( 32, 184, "Press any key..." ) ;
	fflush ( stdin ) ;
	getch() ;
	cleartext() ;
}

/* compares two files */
comparefile()
{
	FILE *fs, *ft ;
	char c, d, file1[30], file2[30] ;

	/* collect first file name */
	setcolor ( 3 ) ;
	outtextxy ( 4 * 8, 6 * 8, "Enter first file name:") ;
	gotoxy ( 5, 9 ) ;
	fflush ( stdin ) ;
	gets ( file1 ) ;

	/* open first file and check if successful */
	fs = fopen ( file1, "r" ) ;
	if ( fs == NULL )
	{
		outtextxy ( 4 * 8, 11 * 8, "Cannot open file!" ) ;
		outtextxy ( 32, 184, "Press any key..." ) ;
		fflush ( stdin ) ;
		getch() ;

		return ;
	}

	/* collect second file name */
	outtextxy ( 4 * 8, 11 * 8, "Enter second file name:") ;
	gotoxy ( 5, 14 ) ;
	fflush ( stdin ) ;
	gets ( file2 ) ;

	/* open second file and check if successful */
	ft = fopen ( file2, "r" ) ;
	if ( ft == NULL )
	{
		outtextxy ( 4 * 8, 16 * 8, "Cannot open file!" ) ;
		outtextxy ( 32, 184, "Press any key..." ) ;
		fflush ( stdin ) ;
		getch() ;

		fclose ( fs ) ;
		return ;
	}

	/* compare files, character by character till a mismatch occurs */
	c = getc ( fs ) ;
	d = getc ( ft ) ;
	while ( c == d )
	{
		c = getc ( fs ) ;
		d = getc ( ft ) ;

		/* if end of any file is reached */
		if ( c == EOF || d == EOF )
			break ;
	}

	fclose ( fs ) ;
	fclose ( ft ) ;

	if ( c == d )
		outtextxy ( 4 * 8, 16 * 8, "Files match!" ) ;
	else
		outtextxy ( 4 * 8, 16 * 8, "Files do not match!" ) ;

	outtextxy ( 32, 184, "Press any key..." ) ;
	fflush ( stdin ) ;
	getch() ;
	cleartext() ;
}

/* encrypts a file using substitution cipher */
encryptfile()
{
	char arr1[97] = "IOP{a}sdfghjkl;'ASDFGHJKL:zxcvbnm,./ZX\
CV BNM<>?1234567890-=~!@#$%^&(*)_\
+|qwertyuiop[Q]WERTYU\\\"\n\t" ;
	char arr2[97] = "`12345 67890-=~!@#$%^&(*)_+|qwertyuiop\
[Q]WERTYUIOP{a}sdfghjkl;'ASDFGHJKL\
:ZXCVBNM,./zxcvbnm<>?\\\"\n\t" ;
	char source[30], target[30], ch ;
	int i ;
	FILE *fs, *ft ;

	/* collect source file name */
	setcolor ( 3 ) ;
	outtextxy ( 4 * 8, 6 * 8, "Enter source file name:") ;
	gotoxy ( 5, 9 ) ;
	fflush ( stdin ) ;
	gets ( source ) ;

	/* open source file and check if successful */
	fs = fopen ( source, "r" ) ;
	if ( fs == NULL )
	{
		outtextxy ( 4 * 8, 11 * 8, "Cannot open source file!" ) ;
		outtextxy ( 32, 184, "Press any key..." ) ;
		fflush ( stdin ) ;
		getch() ;

		return ;
	}

	/* collect target file name */
	outtextxy ( 4 * 8, 11 * 8, "Enter target file name:") ;
	gotoxy ( 5, 14 ) ;
	fflush ( stdin ) ;
	gets ( target ) ;

	/* open target file and check if successful */
	ft = fopen ( target, "w" ) ;
	if ( ft == NULL )
	{
		outtextxy ( 4 * 8, 16 * 8, "Cannot open target file!" ) ;
		outtextxy ( 32, 184, "Press any key..." ) ;
		fflush ( stdin ) ;
		getch() ;

		fclose ( fs ) ;
		return ;
	}

	/* read characters from source file till end of file is reached */
	while ( ( ch = getc ( fs ) ) != EOF )
	{
		/* search the character read in arr1[] */
		for ( i = 0 ; i <= 96 ; i++ )
		{
			if ( ch == arr1[i] )
				break ;
		}

		/* place the corresponding character from arr2[] in target file */
		putc ( arr2[i], ft ) ;
	}

	fclose ( fs ) ;
	fclose ( ft ) ;

	outtextxy ( 4 * 8, 16 * 8, "File was successfully encrypted!" ) ;
	outtextxy ( 32, 184, "Press any key..." ) ;
	fflush ( stdin ) ;
	getch() ;
	cleartext() ;
}

/* decrypts an encrypted file */
decryptfile()
{
	char arr1[97] = "IOP{a}sdfghjkl;'ASDFGHJKL:zxcvbnm,./ZX\
CV BNM<>?1234567890-=~!@#$%^&(*)_\
+|qwertyuiop[Q]WERTYU\\\"\n\t" ;
	char arr2[97] = "`12345 67890-=~!@#$%^&(*)_+|qwertyuiop\
[Q]WERTYUIOP{a}sdfghjkl;'ASDFGHJKL\
:ZXCVBNM,./zxcvbnm<>?\\\"\n\t" ;
	char source[30], target[30], ch ;
	int i ;
	FILE *fs, *ft ;

	/* collect source file name */
	setcolor ( 3 ) ;
	outtextxy ( 4 * 8, 6 * 8, "Enter source file name:") ;
	gotoxy ( 5, 9 ) ;
	fflush ( stdin ) ;
	gets ( source ) ;

	/* open source file and check if successful */
	fs = fopen ( source, "r" ) ;
	if ( fs == NULL )
	{
		outtextxy ( 4 * 8, 11 * 8, "Cannot open source file!" ) ;
		outtextxy ( 32, 184, "Press any key..." ) ;
		fflush ( stdin ) ;
		getch() ;

		return ;
	}

	/* collect target file name */
	outtextxy ( 4 * 8, 11 * 8, "Enter target file name:") ;
	gotoxy ( 5, 14 ) ;
	fflush ( stdin ) ;
	gets ( target ) ;

	/* open target file and check if successful */
	ft = fopen ( target, "w" ) ;
	if ( ft == NULL )
	{
		outtextxy ( 4 * 8, 16 * 8, "Cannot open target file!" ) ;
		outtextxy ( 32, 184, "Press any key..." ) ;
		fflush ( stdin ) ;
		getch() ;

		fclose ( fs ) ;
		return ;
	}

	/* read characters from source file till end of file is reached */
	while ( ( ch = getc ( fs ) ) != EOF )
	{
		/* search the character read in arr2[] */
		for ( i = 0 ; i <= 96 ; i++ )
		{
			if ( ch == arr2[i] )
				break ;
		}

		/* place the corresponding character from arr1[] in target file */
		putc ( arr1[i], ft ) ;
	}

	fclose ( fs ) ;
	fclose ( ft ) ;

	outtextxy ( 4 * 8, 16 * 8, "File was successfully decrypted!" ) ;
	outtextxy ( 32, 184, "Press any key..." ) ;
	fflush ( stdin ) ;
	getch() ;
	cleartext() ;
}
