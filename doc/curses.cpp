#include <curses.h>

// Ceci est un exemple de programme mal comment� 
// et mal �crit pr�sentant rapidement les curses.

int main(int argc, char *argv[])
{
    int num = 0;

		// D�but pomp� dans la doc

    /* initialize your non-curses data structures here */

    (void) initscr();      /* initialize the curses library */
    keypad(stdscr, TRUE);  /* enable keyboard mapping */
    (void) nonl();         /* tell curses not to do NL->CR/NL on output */
    (void) cbreak();       /* take input chars one at a time, no wait for \n */
    (void) echo();         /* echo input - in color */

    if (has_colors())
    {
        start_color();
        init_pair(1, COLOR_RED,     COLOR_BLACK);
        init_pair(2, COLOR_GREEN,   COLOR_BLACK);
        init_pair(3, COLOR_YELLOW,  COLOR_GREEN);
        init_pair(4, COLOR_BLUE,    COLOR_BLACK);
        init_pair(5, COLOR_CYAN,    COLOR_YELLOW);
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(7, COLOR_WHITE,   COLOR_BLACK);
    }

		// On joue avec les positions et les affichages 
		// NE PAS SE TROMPER SUR LES X et Y (position = (LIGNE, COLONNE),
		// pas (X,Y) !!!!
		int y=0;
		int sensY=1;
		mvprintw(0,10,"Terminal: Cols: %d\t Lines: %d\t",COLS,LINES);
		for (int x=0; x<COLS; x++)
			{				
				attrset(COLOR_PAIR(num++ % 8));
				move(y,x);
				printw("X");
				mvprintw(7,20,"(%d,%d)",x,y);
				if(!(x%10))
					getch();
				
				if(y+sensY==LINES || y+sensY==-1)
					{ printw("Inversion");
						sensY*=-1;
					}
				y+=sensY;
			}
		
		
		// Et la saisie, �a marche?
		attrset(COLOR_PAIR(num++ % 8));
		mvprintw(1,10,"Entrez une cha�ne");
		char chaine[256];
		getstr(chaine);
		mvprintw(2,10,"Lu: %s",chaine);
		getch();

		/* test des fenetres */

		// On construit
		WINDOW *m=newwin(7,30,0,0);

		// Sans ce truc, la fen�tre ne scrolle pas!
		scrollok(m,TRUE);
		wprintw(m,"Ceci est une longue phrase qui doit d�passer une ligne, voire m�me atteindre les trois lignes. Pressez une touche pour scroller");
		wrefresh(m);
		for (int cpt=0;cpt<5;cpt++)
			{
				getch();
				scroll(m);
				wrefresh(m);
			}
		getch();

		// Toujours faire le m�nage
		delwin(m);
		endwin();
}

