#include "../../external.h"
#include "../../internal.h"

Irc::Irc(Screen &scr, Keyboard &kbd) :
	view(scr, kbd),
	e_input(scr, kbd, 250),
	e_server(scr, kbd, 64),
	e_port(scr, kbd, 64),
	e_nick(scr, kbd, 64),
	e_passwd(scr, kbd, 64),
	scr(scr), kbd(kbd)
{
	last_usr[0] = 0;
	e_input.geom(0, ROWS-1, COLS-1, 1);
	e_input.oneline = true;
	e_server.oneline = e_port.oneline =
		e_nick.oneline = e_passwd.oneline = true;
	e_server.h = e_port.h = e_nick.h = e_passwd.h = 1;
	e_server.w = e_port.w = e_nick.w = e_passwd.w = COLS-1;
	e_server.y = 1;
	e_port.y = 3;
	e_nick.y = 5;
	e_passwd.y = 7;
	view.geom(0, 0, COLS-1, ROWS-1);
	pass[0] = 0;
	sprintf(server, "irc.oftc.net");
	sprintf(nick, "esp-user");
	sprintf(host, "esp32-c3");
	channel[0] = 0;
}

static char*
skip(char *s, char c)
{
        while(*s != c && *s != '\0')
                s++;
        if(*s != '\0')
                *s++ = '\0';
        return s;
}

static void
trim(char *s)
{
        char *e;
        for (e = s + strlen(s); e > s && isspace((unsigned char)*(e - 1)); e--);
        *e = '\0';
}

static char *
eat(char *s, int (*p)(int), int r)
{
        while(*s != '\0' && p((unsigned char)*s) == r)
                s++;
        return s;
}

void
Irc::privmsg(char *chan, char *msg)
{
        char fmt[256];
        if(chan[0] == '\0') {
                view.append("No channel to send to");
                return;
        }
        if (!strcmp(chan, channel))
        	sprintf(fmt, "<%s> %s", nick, msg);
        else
		sprintf(fmt, "%s: <%s> %s", chan, nick, msg);
        view.append(fmt);
        sprintf(fmt, "PRIVMSG %s :%s", chan, msg);
        cli->println(fmt);
}


void
Irc::irc_input(char *s)
{
	char fmt[128];
	char c, *p;
	if (!s[0])
		return;
	skip(s, '\n');
	if (s[0] == '/') { /* as is */
		cli->println(s + 1);
		return;
	}
	if (s[0] != ':') {
		privmsg(channel, s);
		return;
	}
	c = *++s;
	if (c && s[1] == ' ') {
		p = s + 2;
		p += strspn(p, " ");
		switch(c) {
		case 'j':
			sprintf(fmt, "JOIN %s", p);
			cli->println(fmt);
			strcpy(channel, p);
			return;
		case 'l':
			s = eat(p, isspace, 1);
			p = eat(s, isspace, 0);
			if (!*s)
				s = channel;
			if (*p)
				*p++ = 0;
			if (!*p)
				p = "Bye!";
			sprintf(fmt, "PART %s :%s", s, p);
			cli->println(fmt);
			return;
		case 'm':
			s = eat(p, isspace, 1);
			p = eat(s, isspace, 0);
			if(*p)
				*p++ = 0;
			privmsg(s, p);
			return;
		case 's':
			strcpy(channel, p);
			sprintf(fmt, "Channel is '%s'", p);
			view.append(fmt);
			return;
		default:
			view.append(":j <chan>\n:s <chan>\n:m <nick> <msg>\n:l [msg]");
			return;
		}
	}
	privmsg(channel, s);
	return;
}

void
Irc::irc_reply(char *cmd)
{
	char fmt[1024];
	char *usr = host;
	char *par, *txt;
	if (!cmd[strspn(cmd, " \t\n")])
		return;
	if (cmd[0] == ':') {
		usr = cmd + 1;
		cmd = skip(usr, ' ');
		if (!cmd[0])
			return;
		skip(usr, '!');
	}
	skip(cmd, '\r');
	par = skip(cmd, ' ');
	txt = skip(par, ':');
	trim(par);
	if (!strcmp("PONG", cmd))
		return;
	if (!strcmp("PRIVMSG", cmd)) {
		sprintf(fmt, "<%s> %s", usr, txt);
		view.append(fmt);
	} else if (!strcmp("PING", cmd)) {
		sprintf(fmt, "PONG ", txt);
		cli->println(fmt);
	} else {
		if (strcmp(usr, last_usr)) {
			sprintf(fmt, "%s: ", usr);
			strcpy(last_usr, usr);
		} else
			fmt[0] = 0;
		if (*par)
			sprintf(fmt + strlen(fmt), "%s(%s): ", cmd, par);
		else if (*cmd)
			sprintf(fmt + strlen(fmt), "%s: ", cmd);
		strcat(fmt, txt);
//		sprintf(fmt, "%s: >< %s (%s): %s", usr, cmd, par, txt);
		view.append(fmt);
		if (!strcmp("NICK", cmd) && !strcmp(usr, nick))
			strcpy(nick, txt);
	}
}

void
Irc::tail()
{
	view.trim_head(view.h*30);
	if (!view.visible)
		return;
	if (e_input.is_active()) {
		view.tail();
		view.show();
	}
}
void
Irc::background()
{
	char fmt[1024];
	int rate = 0;
	while (cli && cli->available()) {
		String line = cli->readStringUntil('\n');
		strcpy(fmt, line.c_str());
		irc_reply(fmt);
		view.trim_head(view.h*30);
		if (rate++ % view.h == 0)
			break;
//			tail();
	}
}

int
Irc::process()
{
	char fmt[1024];
	background();
	tail();
	int m = App::process();
	if (app == &e_input) {
		if (m == KEY_UP) {
			view.up(view.h);
			view.show();
		} else if (m == KEY_DOWN) {
			view.down(view.h);
			view.show();
		} else if (m == KEY_TAB) {
			select_menu();
		} else if (m == KEY_ENTER) {
			strcpy(fmt, e_input.text());
			e_input.set("");
			e_input.show();
			if (!strcmp(fmt, "/quit") || !strcmp(fmt, "/exit"))
				return APP_EXIT;
			irc_input(fmt);
			sprintf(title, "%s %s", server, channel);
			tail();
		} else if (m == APP_EXIT) {
			if (cli)
				return APP_BG;
			return m;
		}
	} else { // menu
		if (m == APP_EXIT) {
			if (!cli)
				return m;
			e_input.resume();
			tail();
			return APP_NOP;
		}
		for (int i = 0; i < menus_nr; i++) {
			if (app == menus[i]) {
				if (m == KEY_UP || m == KEY_DOWN) {
					i += ((m == KEY_UP)? -1:1);
					i = (i<0)?menus_nr-1:i;
					i = (i>=menus_nr)?0:i;
					if (menus[i] != app) {
						app->stop();
						show_menu();
						push(menus[i]);
					}
					return APP_NOP;
				} else if (m == KEY_ENTER) {
					app->stop();
					strcpy(server, e_server.text());
					port = atoi(e_port.text());
					if (port <= 0)
						port = 6667;
					strcpy(nick, e_nick.text());
					strcpy(pass, e_passwd.text());
					prefs.begin("IRC", false);
					prefs.putString("server", e_server.text());
					prefs.putInt("port", port);
					prefs.putString("nick", e_nick.text());
					prefs.putString("passwd", e_passwd.text());
					prefs.end();
					connect_irc();
					return APP_NOP;
				}
			}
		}
	}
	return m;
}

bool
Irc::connect(const char *host, int port)
{
	if (port == 6697 || port == 9999) {
		cli = new WiFiClientSecure();
		((WiFiClientSecure*)cli)->setInsecure();
	} else
		cli = new WiFiClient();
	view.show();
	if (!cli->connect(host, port)) {
		view.append("Can't connect...");
		return false;
	}
	return true;
}

void
Irc::connect_irc()
{
	if (cli) {
		cli->stop();
		delete cli;
	}
	cli = NULL;
	e_input.resume();
	view.reset();
	if (connect(server, port)) {
		sprintf(title, "%s", server);
		char fmt[256];
		if (pass[0]) {
			sprintf(fmt, "PASS %s", pass);
			cli->println(fmt);
		}
		sprintf(fmt, "NICK %s", nick);
		cli->println(fmt);
		sprintf(fmt, "USER %s localhost %s : %s", nick, host, nick);
		cli->println(fmt);
	}
	view.show();
}

void
Irc::show_menu()
{
	scr.text_clear(0, 0, COLS-1, 1, 0xffff, scr.color(0, 128, 128));
	scr.text(0, 0, "Server");
	e_server.show();
	scr.text_clear(0, 2, COLS-1, 1, 0xffff, scr.color(0, 128, 128));
	scr.text(0, 2, "Port");
	e_port.show();
	scr.text_clear(0, 4, COLS-1, 1, 0xffff, scr.color(0, 128, 128));
	scr.text(0, 4, "Nick");
	e_nick.show();
	scr.text_clear(0, 6, COLS-1, 1, 0xffff, scr.color(0, 128, 128));
	scr.text(0, 6, "Password");
	e_passwd.show();
}

void
Irc::select_menu()
{
	e_server.set(server);
	e_port.set(String(port).c_str());
	e_nick.set(nick);
	e_passwd.set(pass);

	show_menu();

	e_input.pause();
	e_input.show();

	push(&e_server);
	kbd.set_fn_mode(true);
}

void
Irc::resume()
{
	App::resume();
	view.show();
	push(&e_input);
	view.visible = true;
	tail();
}

void
Irc::start()
{
	App::start();
	view.reset();
	view.show();
	push(&e_input);
	prefs.begin("IRC", true);
	String s = prefs.getString("server");
	if (s != "")
		strcpy(server, s.c_str());
	int p = prefs.getInt("port");
	if (p > 0)
		port = p;
	s = prefs.getString("nick");
	if (s != "")
		strcpy(nick, s.c_str());
	s = prefs.getString("passwd");
	if (s != "")
		strcpy(pass, s.c_str());
	prefs.end();
	select_menu();
}
