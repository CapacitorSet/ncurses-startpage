#include <curses.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "settings.h"

char latest_key;
_Bool isBlocked = false;

int height, width;
#define HALFH (height / 2)
#define HALFW (width / 2)
#define URL_MAX_LEN 300

void bufUpdate(void);
void textUpdate(void);

void *key_reader(void *args __attribute__((__unused__))) {
	for (;;) {
		latest_key = getch();
		switch (latest_key) {
		case 'f':
			popen("firefox", "r");
			// sleep(1);
			// popen("xdotool windowactivate 39845893", "r");
			break;
		case 'g':
			isBlocked = true; // Todo: disable this, and instead use a func ptr that adds the rest
		urlPickerPickAgain:
			clear();
			mvaddstr(HALFH + 2, HALFW - 4, "'u' for URLs");
			mvaddstr(HALFH + 3, HALFW - 4, "'h' for http://");
			mvaddstr(HALFH + 4, HALFW - 4, "'s' for https://");
			mvaddstr(HALFH + 5, HALFW - 4, "'r' for Reddit");
			mvaddstr(HALFH + 6, HALFW - 4, "'q' to quit");
			refresh();
			char urlTypeChoice = getch();
			char prefix[100] = "";
			char input[300] = "";
			char postfix[100] = "";
			clear();
			switch (urlTypeChoice) {
			case 'h':
				mvaddstr(HALFH + 2, HALFW - 4, "URL: http://");
				echo();
				strcpy(prefix, "http://");
				getstr(input);
				for (int i = 0; i < URL_SHORTCUTS_LENGTH; i++)
					if (strcmp(input, URL_SHORTCUTS[i].shortcut) == 0)
						strcpy(input, URL_SHORTCUTS[i].expansion);
				break;
			case 's':
				mvaddstr(HALFH + 2, HALFW - 4, "URL: https://");
				echo();
				strcpy(prefix, "https://");
				getstr(input);
				for (int i = 0; i < URL_SHORTCUTS_LENGTH; i++)
					if (strcmp(input, URL_SHORTCUTS[i].shortcut) == 0)
						strcpy(input, URL_SHORTCUTS[i].expansion);
				break;
			case 'r':
			redditPickerPickAgain:
				clear();
				mvaddstr(HALFH + 3, HALFW - 4, "Enter for frontpage");
				mvaddstr(HALFH + 4, HALFW - 4, "'r' for subreddits");
				mvaddstr(HALFH + 5, HALFW - 4, "'u' for users");
				mvaddstr(HALFH + 6, HALFW - 4, "'n' for /r/input/new");
				mvaddstr(HALFH + 7, HALFW - 4, "'q' to quit");
				refresh();
				char redditTypeChoice = getch();
				clear();
				switch (redditTypeChoice) {
				case '\n': // Enter
					strcpy(input, "https://www.reddit.com");
					break;
				case 'r':
					mvaddstr(HALFH + 2, HALFW - 4, "Subreddit: /r/");
					echo();
					strcpy(prefix, "https://www.reddit.com/r/");
					strcpy(postfix, "/"); // Used to avoid redirects
					getstr(input);
					for (int i = 0; i < SUBREDDIT_SHORTCUTS_LENGTH; i++)
						if (strcmp(input, SUBREDDIT_SHORTCUTS[i].shortcut) == 0)
							strcpy(input, SUBREDDIT_SHORTCUTS[i].expansion);
					break;
				case 'u':
					mvaddstr(HALFH + 2, HALFW - 4, "User: /u/");
					echo();
					strcpy(prefix, "https://www.reddit.com/u/");
					getstr(input);
					// I know u/me does the trick, but this avoids taking a slow redirect.
					if (strcmp(input, "me") == 0)
						strcpy(input, MY_REDDIT_USERNAME);
					break;
				case 'n':
					mvaddstr(HALFH + 2, HALFW - 4, "Subreddit [new]: /r/");
					echo();
					strcpy(prefix, "https://www.reddit.com/r/");
					strcpy(postfix, "/new");
					getstr(input);
					for (int i = 0; i < SUBREDDIT_SHORTCUTS_LENGTH; i++)
						if (strcmp(input, SUBREDDIT_SHORTCUTS[i].shortcut) == 0)
							strcpy(input, SUBREDDIT_SHORTCUTS[i].expansion);
					break;
				case '\x7f': // Backspace
					goto urlPickerPickAgain;
				case 'q':
					goto urlPickerEnd;
				default:
					mvaddstr(HALFH + 2, HALFW - 4, "Invalid shortcut. Pick one of:");
					goto redditPickerPickAgain;
				}
				break;
			case 'u':
				mvaddstr(HALFH + 2, HALFW - 4, "URL: ");
				echo();
				getstr(input);
				for (int i = 0; i < URL_SHORTCUTS_LENGTH; i++)
					if (strcmp(input, URL_SHORTCUTS[i].shortcut) == 0)
						strcpy(input, URL_SHORTCUTS[i].expansion);
				break;
			case 'q':
				goto urlPickerEnd;
			default:
				mvaddstr(HALFH + 2, HALFW - 4, "Invalid command. Pick one of:");
				goto urlPickerPickAgain;
			}
			char command[URL_MAX_LEN + 9] = "firefox ";
			strcat(command, prefix);
			strcat(command, input);
			strcat(command, postfix);
			popen(command, "r");
			clear();
			mvaddstr(HALFH + 2, HALFW - 4, "Visited: ");
			addstr(prefix);
			addstr(input);
			addstr(postfix);
			refresh();
			sleep(2);
		urlPickerEnd:
			isBlocked = false;
			noecho();
			textUpdate();
			break;
		case 'r':
			isBlocked = true; // Todo: disable this, and instead use a func ptr that adds the rest
			goto redditPickerPickAgain;
		case 'q':
			endwin();
			exit(0);
		}
	}
}

char time_buf[9];
char date_buf[9];
const char separator[9] = "========";

void bufUpdate() {
	time_t my_time;
	struct tm *timeinfo;
	time(&my_time);
	timeinfo = localtime(&my_time);

	sprintf(time_buf, "%02d:%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	// tm_year is the number of years after 1900. To get a double-digit number, we take 100 from it.
	sprintf(date_buf, "%02d/%02d/%2d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year - 100);
}

void textUpdate() {
	clear();

	getmaxyx(stdscr, height, width);

	bufUpdate();

	mvaddstr(HALFH - 1, HALFW - 4, time_buf);
	mvaddstr(HALFH, HALFW - 4, date_buf);
	mvaddstr(HALFH + 1, HALFW - 4, separator);
	mvaddstr(HALFH + 2, HALFW - 4, "'g' for URLs");
	mvaddstr(HALFH + 3, HALFW - 4, "'r' for Reddit resources");
	mvaddstr(HALFH + 4, HALFW - 4, "'q' to quit");

	refresh();
}

int main() {
	initscr();
	curs_set(0); // Invisible
	noecho();
	cbreak();
	keypad(stdscr, TRUE);

	pthread_t key_thread;
	int status = pthread_create(&key_thread, NULL, key_reader, NULL);
	if (status)
		return 1;


	for (;;) {
		if (!isBlocked)
			textUpdate();
		sleep(1);
	}
	getch();
	endwin();

	return 0;
}
