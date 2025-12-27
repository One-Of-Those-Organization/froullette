// #pragma once
// #include "ArsEng.hpp"
// #include "GameState.hpp"
// #include "../Object/Button.hpp"
// #include "../Object/Text.hpp"
// #include "helper.hpp"

// static void createPlayMenu(ArsEng *engine, int kh_id, Vector2 *wsize, int *z) {

//     (void)kh_id;
//     GameState state = GameState::PLAYMENU;
//     size_t title_size = 64;
//     Color title_color = WHITE;

//     Text *title1 = cText(engine, state, "Fate", title_size, title_color, {0,0});
//     title1->position_info.offset.y = -title1->rec.height;
//     title1->update_using_scale(engine->get_scale_factor(), *wsize);
//     engine->om.add_object(title1, (*z)++);

//     Text *title2 = cText(engine, state, "Roullete", title_size, title_color, {0,0});
//     engine->om.add_object(title2, (*z)++);


//     size_t text_size = 36;
//     size_t padding = 20;

//     Button *btn1 = cButton(engine, "Play", text_size, padding, state, {0,0},
//                            [engine]() { engine->request_change_state(GameState::PLAYMENU); }
//     );
//     btn1->is_resizable = true;
//     btn1->position_info.center_x = true;
//     btn1->position_info.center_y = true;
//     btn1->calculate_rec();
//     btn1->position_info.offset.y = title_size * 3;
//     btn1->update_using_scale(engine->get_scale_factor(), *wsize);
//     engine->om.add_object(btn1, (*z)++);

//     Button *btn2 = cButton(engine, "Conf", text_size, padding - 5, state, {0,0},
//                            [engine]() { engine->request_change_state(GameState::SETTINGS); }
//     );
//     btn2->is_resizable = true;
//     btn2->position_info.center_x = true;
//     btn2->position_info.center_y = true;
//     btn2->calculate_rec();
//     btn2->position_info.offset.y = title_size * 3;
//     btn2->position_info.offset.x = ((btn1->rec.width + btn2->rec.width) * 0.5f) + padding;
//     btn2->update_using_scale(engine->get_scale_factor(), *wsize);
//     engine->om.add_object(btn2, (*z)++);

//     Button *btn3 = cButton(engine, "Exit", text_size, padding - 5, state, {0,0},
//                            [engine]() { engine->req_close = true; }
//     );
//     btn3->is_resizable = true;
//     btn3->position_info.center_x = true;
//     btn3->position_info.center_y = true;
//     btn3->calculate_rec();
//     btn3->position_info.offset.y = title_size * 3;
//     btn3->position_info.offset.x = -((btn1->rec.width + btn3->rec.width) * 0.5f) - padding;
//     btn3->update_using_scale(engine->get_scale_factor(), *wsize);
//     engine->om.add_object(btn3, (*z)++);
//     /*
//     GameState state = GameState::PLAYMENU;
//     size_t title_size = 48;
//     Color title_color = WHITE;

//     Text *title1 = cText(engine, state, "Play Menu", title_size, title_color, {0,0});
//     title1->position_info.offset.y = -title1->rec.height * 2;
//     title1->update_using_scale(engine->get_scale_factor(), *wsize);
//     engine->om.add_object(title1, (*z)++);

//     size_t text_size = 36;
//     size_t padding = 20;

//     Button *btn1 = cButton(engine, "Start Game", text_size, padding, state, {0,0},
//                            [engine]() { engine->request_change_state(GameState::INGAME); }
//     );
//     btn1->is_resizable = true;
//     btn1->position_info.center_x = true;
//     btn1->position_info.center_y = true;
//     btn1->calculate_rec();
//     btn1->position_info.offset.y = title_size * 2;
//     btn1->update_using_scale(engine->get_scale_factor(), *wsize);
//     engine->om.add_object(btn1, (*z)++);

//     Button *btn2 = cButton(engine, "Settings", text_size, padding - 5, state, {0,0},
//                            [engine]() { engine->request_change_state(GameState::SETTINGS); }
//     );
//     btn2->is_resizable = true;
//     btn2->position_info.center_x = true;
//     btn2->position_info.center_y = true;
//     btn2->calculate_rec();
//     btn2->position_info.offset.y = title_size * 2;
//     btn2->position_info.offset.x = ((btn1->rec.width + btn2->rec.width) * 0.5f) + padding;
//     btn2->update_using_scale(engine->get_scale_factor(), *wsize);
//     engine->om.add_object(btn2, (*z)++);

//     Button *btn3 = cButton(engine, "Back to Menu", text_size, padding - 5, state, {0,0},
//                            [engine]() { engine->request_change_state(GameState::MENU);}
//     );*/
// }
