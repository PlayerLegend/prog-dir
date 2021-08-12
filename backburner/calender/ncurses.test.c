#include <ncurses.h>
#include <stdbool.h>

int main()
{
    int key;
    int x = 0, y = 0;
    initscr();
    keypad(stdscr, true);

    printw("meme");
    refresh();

    WINDOW * window = newwin (5, 10, 5, 10);
    
    while ( KEY_ENTER != (key = getch()) )
    {
	if (key == KEY_UP && y > 0)
	{
	    y--;
	}
	else if (key == KEY_DOWN)
	{
	    y++;
	}
	else if (key == KEY_LEFT && x > 0)
	{
	    x--;
	}
	else if (key == KEY_RIGHT)
	{
	    x++;
	}

	clear();
	move (y,x);
	printw("meme");
	wborder(window,
		'+',
		'+',
		'+',
		'+',
		'*',
		'*',
		'*',
		'*');
	box(window, 0 , 0);
	refresh();
	wrefresh (window);
    }
    
    endwin();
    
    return 0;
}
