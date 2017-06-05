#include <exception>
#include <android_native_app_glue.h>
#include <jni.h>
#include <android/log.h>
#define  log(...)  __android_log_print(ANDROID_LOG_INFO,"tuner",__VA_ARGS__)
#define  error(...)  ({ log(__VA_ARGS__); std::terminate(); })
#define assert(expr, message...) ({ if(!(expr)) error(#expr, ## message); })
#define check(expr, message...) ({ assert(expr>=0, ## message); })

struct App {
    android_app* app;

    void render(ANativeWindow* window) {
        if(window == NULL) return;

        ANativeWindow_Buffer buffer;
        check(ANativeWindow_lock(window, &buffer, NULL));

        ANativeWindow_unlockAndPost(window);
    }

    void onTermination() {}

    int inputEvent(AInputEvent* event) {
        if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
            return 1;
        } else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
            log("Key event: action=%d keyCode=%d metaState=0x%x",
                AKeyEvent_getAction(event),
                AKeyEvent_getKeyCode(event),
                AKeyEvent_getMetaState(event));
        }
        return 0;
    }
    void appCmd(int cmd) {
        /**/ if(cmd == APP_CMD_INIT_WINDOW) {
            if (app->window) {
                ANativeWindow_setBuffersGeometry(app->window, ANativeWindow_getWidth(app->window), ANativeWindow_getHeight(app->window), WINDOW_FORMAT_RGBX_8888);
                render(app->window);
            }
        }
        else if(cmd == APP_CMD_TERM_WINDOW) {
            onTermination();
            ANativeWindow_setBuffersGeometry(app->window, ANativeWindow_getWidth(app->window), ANativeWindow_getHeight(app->window), WINDOW_FORMAT_RGBX_8888);
        }
        else if(cmd == APP_CMD_LOST_FOCUS) {
        }
    }
};

static int32_t onInputEvent(android_app* app, AInputEvent* event) {
    return ((App*)app->userData)->inputEvent(event);
}

static void onAppCmd(android_app* app, int32_t cmd) {
    return ((App*)app->userData)->appCmd(cmd);
}

void android_main(android_app* aApp) {
    App app;
    app_dummy();

    aApp->userData = &app;
    aApp->onAppCmd = &onAppCmd;
    aApp->onInputEvent = &onInputEvent;
    app.app = aApp;

    for(;;) {
        constexpr bool running = true;
        for(;;) {
            int events;
            android_poll_source* source;
            if(ALooper_pollAll(running ? 0 : -1, 0, &events, (void**)&source) < 0) break;
            if(source) source->process(aApp, source);
            if(aApp->destroyRequested) { app.onTermination(); return; }
        }
        if(running) app.render(aApp->window);
    }
}
