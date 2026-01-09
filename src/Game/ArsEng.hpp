#pragma once

#include <raylib.h>
#include "../Object/ObjectManager.hpp"
#include "../Shader/ShadersManager.hpp"
#include "../Texture/TextureManager.hpp"

static const Vector2 CANVAS_SIZE = Vector2{128 * 1.5, 72 * 1.5};
static const Vector2 BIGCANVAS_SIZE = Vector2{128 * 10, 72 * 10};

enum RequestType {
    DONE = 0,
    RESIZE,
    TFULLSCREEN,
};

struct Request {
    RequestType t;
    union {
        Vector2 v;
    } data;
};

class ArsEng {
public:
    Vector2 window_size;
    RenderTexture2D canvas;
    RenderTexture2D bigcanvas;
    ObjectManager om;
    ShadersManager sm;
    TextureManager tm;
    GameState state;
    GameState oldstate;
    Font font;

    Vector2 canvas_size = {};
    Vector2 cursor = {};
    Vector2 canvas_cursor = {};
    Vector2 bigcanvas_cursor = {};

    bool req_close;

    bool dragging = false;
    int dragged_obj = -1;
    int _last_dragged_obj = -1;
    int active = -1;

    std::vector<Object *> render_later;
    void *additional_data = nullptr;
    Request _req;
    GameState _req_state;

    ArsEng(Vector2 wsize): om(), tm(), state(GameState::MENU), _req_state(GameState::MENU) {
        canvas = LoadRenderTexture(CANVAS_SIZE.x, CANVAS_SIZE.y);
        bigcanvas = LoadRenderTexture(BIGCANVAS_SIZE.x, BIGCANVAS_SIZE.y);
        SetTextureFilter(canvas.texture, TEXTURE_FILTER_POINT);
        SetTextureFilter(bigcanvas.texture, TEXTURE_FILTER_POINT);
        SetTextureWrap(canvas.texture, TEXTURE_WRAP_CLAMP);
        SetTextureWrap(bigcanvas.texture, TEXTURE_WRAP_CLAMP);

        this->window_size = wsize;
        this->req_close = false;
        this->canvas_size.x = this->canvas.texture.width;
        this->canvas_size.y = this->canvas.texture.height;
        this->font =
            LoadFontEx("assets/Pixelify_Sans/PixelifySans-VariableFont_wght.ttf",
                       120, NULL, 95);

        if (this->font.texture.id == 0)
            TraceLog(LOG_FATAL,
                     TextFormat("%s\n",
                                "Try to launch the game from the correct path."
                                " The game expect the `assets` folder in cwd."));

#ifdef MOBILE
        SetGesturesEnabled(GESTURE_TAP);
        SetGesturesEnabled(GESTURE_DOUBLETAP);
        SetGesturesEnabled(GESTURE_DRAG);
        SetGesturesEnabled(GESTURE_NONE);
#endif
    }

    ~ArsEng() {
        UnloadFont(this->font);
    };

    void check_and_recreate_canvas() {
        if (canvas.texture.id == 0) {
            UnloadRenderTexture(canvas);
            canvas = LoadRenderTexture(CANVAS_SIZE.x, CANVAS_SIZE.y);
            SetTextureFilter(canvas.texture, TEXTURE_FILTER_POINT);
            SetTextureWrap(canvas.texture, TEXTURE_WRAP_CLAMP);
            this->canvas_size.x = this->canvas.texture.width;
            this->canvas_size.y = this->canvas.texture.height;
        }

        if (bigcanvas.texture.id == 0) {
            UnloadRenderTexture(bigcanvas);
            bigcanvas = LoadRenderTexture(BIGCANVAS_SIZE.x, BIGCANVAS_SIZE.y);
            SetTextureFilter(bigcanvas.texture, TEXTURE_FILTER_POINT);
            SetTextureWrap(bigcanvas.texture, TEXTURE_WRAP_CLAMP);
        }
    }

    void render() {
        BeginTextureMode(bigcanvas);
        ClearBackground(BLANK);
        for (auto &obj: this->render_later) {
            if (!has_flag(state, obj->state)) continue;
            obj->render();
        }
        render_later.clear();
        EndTextureMode();
    }

    void render_to_canvas() {
        render_later.clear();
        BeginTextureMode(canvas);
        ClearBackground(BLACK);
        for (auto &obj: om.sorted) {
            if (!has_flag(state, obj->state)) continue;
            if (obj->draw_in_canvas) obj->render();
            else render_later.push_back(obj);
        }
        EndTextureMode();
    }

    void update(float dt) {
#ifdef MOBILE
        this->cursor = GetTouchPosition();
#else
        this->cursor = GetMousePosition();
#endif
        this->canvas_cursor.x =
            (this->cursor.x / this->window_size.x) * this->canvas_size.x;
        this->canvas_cursor.y =
            (this->cursor.y / this->window_size.y) * this->canvas_size.y;

        this->bigcanvas_cursor.x = (this->cursor.x / this->window_size.x) * this->bigcanvas.texture.width;
        this->bigcanvas_cursor.y = (this->cursor.y / this->window_size.y) * this->bigcanvas.texture.height;

        for (const auto &o: this->om.sorted) {
            if (!has_flag(state, o->state) || !o->show) continue;
            o->logic(dt);
        }
        if (this->dragged_obj != this->_last_dragged_obj) {
            if (this->_last_dragged_obj != -1) {
                this->om.revert_zindex(this->_last_dragged_obj);
            }

            if (this->dragged_obj != -1) {
                this->om.update_zindex(this->dragged_obj, 1000);
            }

            this->_last_dragged_obj = this->dragged_obj;
        }

        this->_change_state();
    }

    void request_fullscreen() {
        _req.t = TFULLSCREEN;
    }

    void request_resize(Vector2 new_size) {
        _req.t = RESIZE;
        _req.data.v = new_size;
    }

    void revert_state() { this->_req_state = this->oldstate; }
    void request_change_state(GameState state) { this->_req_state = state; }
    void _change_state() {
        if (this->_req_state != this->state) {
            this->oldstate = this->state;
            this->state = this->_req_state;
            this->dragging = false;
        }
    }
};
