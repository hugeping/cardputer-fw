#include "../../external.h"
#include "../../internal.h"

Gemini::Gemini(Screen &screen, Keyboard &keys) :
	scr(screen), kbd(keys),
	e_addr(screen, keys, 256),
	e_input(screen, keys, 1024),
	m_links(screen, keys, MAX_LINKS),
	view(screen, keys),
	v_status(screen, keys)
{
	e_input.oneline = true;
	e_addr.oneline = true;
	view.geom(0, 0, COLS-1, ROWS);
	m_links.geom(0, 0, COLS-1, ROWS);
	e_input.geom(0, 2, COLS-1, ROWS);
	e_addr.geom(0, 1, COLS-1, ROWS-1);
}

static bool
is_esc(char c)
{
	switch(c) {
	case ' ':
	case '\t':
	case '%':
	case '/':
	case '#':
	case '-':
	case '.':
	case '?':
	case '!':
	case ':':
		return true;
	default:
		return false;
	}
}

static void
url_encode(char *dst, const char *src)
{
	while (*src) {
		if (is_esc(*src)) {
			sprintf(dst, "%%%02x", *src);
			dst += 3;
			src ++;
		} else
			*dst ++ = *src ++;
	}
	*dst = *src;
}


bool
Gemini::req(const char* req)
{
	String ans;
	const char *str;
	client.println(req);
	ans = client.readStringUntil('\n');
	str = ans.c_str();
	int len = ans.length();
	if (len<3 || str[2] != ' ') {
		status[0] = 0;
		meta[0] = 0;
		return false;
	}
	status[0] = str[0]; status[1] = str[1];
	status[2] = 0;
	str += 3;
	len -= 3;
	if (len > 1024)
		memcpy(meta, str, 1024);
	else {
		strcpy(meta, str);
		meta[strcspn(meta, "\r")] = 0;
	}
	meta[1024] = 0;
	return true;
}


static
bool is_space(char c)
{
	return c == ' ' || c == '\t';
}

static
int skip_spaces(String &line, int i)
{
	while (is_space(line[i])) i++;
	return i;
}
static
int find_space(String &line, int s)
{
	for (int i = s; i < line.length(); i ++)
		if (is_space(line[i]))
			return i;
	return -1;
}

void
Gemini::menu_reset()
{
	m_links.reset();
	m_links.append("[Enter address]");
}

void
Gemini::body()
{
	menu_reset();
	links_nr = 0;
	view.reset();
	while (client.connected() || client.available()) {
		String line = client.readStringUntil('\n');
		line.replace("\r", "");
		if (line.startsWith("=>")) {
			String link;
			line.trim();
			int s = skip_spaces(line, 2);
			int idx = find_space(line, s);
			if (idx >=0) {
				link = line.substring(s, idx);
				idx = skip_spaces(line, idx);
				line.remove(s, idx - s);
			} else {
				link = line.substring(s);
			}
			if (links_nr < MAX_LINKS &&
				!link.startsWith("http:") &&
				!link.startsWith("https:")) {
				links[links_nr++] = link;
				m_links.append(line.c_str() + s);
			}
		}
		view.append(line.c_str());
	}

}

static void
normpath(char *path)
{
	const char *src = path;
	char *dst = path;

	while (*src) {
		if (src[0] == '.') {
			if (src[1] == '/') {
				src += 2;
				continue;
			} else if(src[1] == '.' && src[2] == '/') {
				if (dst > path)
					dst --;
				while (dst > path && *(dst-1) != '/') dst--;
				src += 3;
				continue;
			}
		}
		do {
			*dst++ = *src;
		} while(*src && *(src++) != '/');
		src += strspn(src, "/");
	}
}
static char*
dirpath(char *path)
{
	if (!path[strcspn(path, "/")])
		return path;
	char *eptr = path + strlen(path);
	while (*(--eptr) != '/');
	*eptr = 0;
	return path;
}
bool
Gemini::reqURI(const char *uri, bool hist)
{
	char url[1025];
	const char *ptr = uri;
//	Serial.println("reqURI:"+String(uri));
	if (last_url && hist) {
		history[(hist_pos++)%hist_max] = String(last_url);
		hist_size = max(hist_size + 1, hist_max);
	}
	if (!strncmp(uri, "gemini://", 9)) {
		free(server);
		free(last_url);
		last_url = strdup(uri);
		ptr += 9;
		server = strdup(ptr);
		server[strcspn(server, "/")] = 0;
//		view.title = server;
		if (!client.connect(server, 1965))
			return false;
	} else if (server) {
		if (!client.connect(server, 1965))
			return false;
		if (uri[0] != '/')
			if (uri[0] == '?')
				sprintf(url, "%s/%s", last_url, uri);
			else
				sprintf(url, "%s/%s", dirpath(last_url), uri);
		else
			sprintf(url, "gemini://%s%s", server, uri);
		free(last_url);
		last_url = strdup(url);
	} else
		return false;
	normpath(last_url + 9);
	return req(last_url);
}

void
Gemini::message(const char *msg)
{
	view.set(msg);
	view.show();
}

void
Gemini::input(const char *msg)
{
	view.pause();
	int y;
	scr.off2xy(scr.text(0, 0, msg, true, COLS-1, ROWS), NULL, &y);
	y = min(ROWS-1, y+1);
	e_input.geom(0, y, COLS-1, 1);
	push(&e_input);
}

bool
Gemini::request(const char *uri, bool hist)
{
	char fmt[1024];
	bool redirect = false;
	Serial.println("Request:"+String(uri));
	kbd.set_fn_mode(true);
	view.set("Connecting...");
	view.show();
	do {
		if (!reqURI(uri, hist)) {
			client.stop();
			sprintf(fmt, "%s\nConnection error",
				last_url?last_url:"NULL");
			message(fmt);
			return false;
		} else if (status[0] == '1') {
			client.stop();
			input(meta);
			return true;
		} else if (status[0] == '2') {
			body();
			client.stop();
			view.show();
			prefs.begin("gemini", false);
			prefs.putString("last_url", last_url);
			prefs.end();
			return true;
		} else if (status[0] == '3') {
			client.stop();
			if (redirect) {
				message("Multiple redirects");
				return false;
			}
			uri = meta;
			redirect = true;
			continue;
		}
		client.stop();
		sprintf(fmt, "%s\nStatus: %s", last_url, status);
		message(fmt);
		return false;
	} while(1);
}

int
Gemini::process()
{
	char req[1024];
	int m = App::process();
	if (view.is_active() && (m == KEY_TAB || m == KEY_ENTER)) {
		view.pause();
		push(&m_links);
	} else if (app == &e_addr) {
		if (m == KEY_ENTER) {
			e_addr.stop();
			view.resume();
			sprintf(req, "gemini://%s", e_addr.text());
			request(req);
			return APP_NOP;
		} else if(m == APP_EXIT) {
			push(&m_links);
			return APP_NOP;
		}
	} else if (app == &e_input) {
		if (m == KEY_ENTER) {
			view.resume();
			req[0] = '?';
			url_encode(&req[1], e_input.text());
			request(req);
			return APP_NOP;
		} else if(m == APP_EXIT) {
			view.resume();
			return APP_NOP;
		}
	} else if (app == &m_links) {
		if (m == 0) {
			m_links.stop();
			scr.text_clear();
			scr.text(0, 0, "Address");
			push(&e_addr);
		} else if (m > 0) {
			m_links.stop();
			view.resume();
			request(links[m-1].c_str());
		} else if (m == APP_EXIT)
			view.resume();
		return APP_NOP;
	} else if (view.is_active() &&
		(m == KEY_BS || m == KEY_DEL)) {
		if (hist_size > 0) {
			hist_size --;
			request(history[(--hist_pos)%hist_max].c_str(), false);
		}
	} else if (m == APP_EXIT && app == &view) {
		Serial.println("Gemini exit?");
		return m;
	}
	return APP_NOP;
}

void
Gemini::start()
{
//	kbd.set_fn_mode(true);
	client.setInsecure();
	App::start();
	push(&view);
	if (!server) {
		menu_reset();
		prefs.begin("gemini", true);
		String url = prefs.getString("last_url");
		if (url == "")
			request("gemini://geminispace.info");
		else
			request(url.c_str());
		prefs.end();
	}
}
