#ifndef __APP_H_INCLUDED
#define __APP_H_INCLUDED

#define APP_EXIT -1
#define APP_NOP -2

enum app_state_t {
	APP_ACTIVE,
	APP_BACKGROUND,
	APP_INACTIVE,
	APP_EXITED,
};

class App {
	static const int apps_max = 16;
	App *apps[apps_max] = { NULL, };
public:
	App *app = NULL; /* last active app */
	const char *name = NULL;
	app_state_t state = APP_INACTIVE;
	App() {};
	App(const char *n) { name = n; };
	virtual void start() { state = APP_ACTIVE; };
	virtual void resume() { state = APP_ACTIVE; };
	virtual void pause() { state = APP_BACKGROUND; };
	virtual void stop() {
		for (int i = 0; i < apps_max; i ++) {
			if (apps[i]) {
				apps[i]->stop();
				apps[i] = NULL;
			}
		}
		state = APP_EXITED;
	};
	virtual void background() { };
	virtual bool push(App *a) {
		for (int i = 0; i < apps_max; i ++) {
			if (apps[i])
				continue;
			apps[i] = a;
			a->start();
			return true;
		}
		return false;
	}
	virtual bool is_active() {
		return state == APP_ACTIVE;
	}
	virtual bool is_exited() {
		return state == APP_EXITED;
	}
	virtual int process() {
		int rc = APP_NOP;
		for (int i = 0; i < apps_max; i ++) {
			if (apps[i] && apps[i]->state == APP_INACTIVE) {
				apps[i] = NULL;
				continue;
			}
		}

		for (int i = 0; i < apps_max && rc == APP_NOP; i ++) {
			App *a = apps[i];
			if (!a)
				continue;
			if (a->state == APP_ACTIVE)
				rc = a->process();
			else if (a->state == APP_BACKGROUND)
				a->background();
			if (rc != APP_NOP)
				app = apps[i];
			if (rc == APP_EXIT) {
				apps[i]->stop();
				apps[i] = NULL;
			}
		}
		return rc;
	}
	virtual ~App() {};
};
#endif
