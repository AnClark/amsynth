#include "imgui_piano.h"
#include <cstdio>

char *ImGui::Piano::keyboardToMidi(int key_id, int velocity, bool note_on)
{
    char *midi_buffer = new char[3];

    midi_buffer[0] = note_on ? MIDI_STATUS_NOTE_ON : MIDI_STATUS_NOTE_OFF;
    midi_buffer[1] = KEY_INDEX_START_WHITE - 0x15; // Relative to A0
    midi_buffer[2] = velocity;

    return midi_buffer;
}

inline bool ImGui::Piano::has_black(int key)
{
    return (!((key - 1) % 7 == 0 || (key - 1) % 7 == 3) && key != keyNum - 1);
}

ImGui::Piano::Piano(int keyNum) : keyNum(keyNum)
{
    // Specify default callbacks
    this->setPianoKeyEventCallback(default_piano_midi_event_callback);
}

void ImGui::Piano::up(int key)
{
    keyStates[key] = 0;
}

void ImGui::Piano::down(int key, int velocity)
{
    keyStates[key] = velocity;
}

void ImGui::Piano::drawKeyboard(ImVec2 size, bool enable_input)
{
    static char key_name[] = {'Q', '2', 'W', '3', 'E', 'R', '5', 'T', '6', 'Y', '7', 'U',
                              'Z', 'S', 'X', 'D', 'C', 'V', 'G', 'B', 'H', 'N', 'J', 'M'};
    char buf[4];
    ImGuiIO &io = ImGui::GetIO();
    ImU32 Black = IM_COL32(0, 0, 0, 255);
    ImU32 White = IM_COL32(255, 255, 255, 255);
    ImU32 White_Bark = IM_COL32(208, 208, 208, 255);
    ImU32 EventSustained = IM_COL32(192, 192, 192, 255);
    ImU32 EventBlack = IM_COL32(255, 0, 0, 255);

    ImGui::BeginGroup();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float key_width = size.x / keyNum;
    float white_key_height = size.y;
    float black_key_height = size.y * 3.f / 5.f;

    /** Draw white keys */
    int cur_key = KEY_INDEX_START_WHITE;
    for (int key = 0; key < keyNum; key++)
    {
        ImRect key_rect(ImVec2(p.x + key * key_width, p.y),
                        ImVec2(p.x + key * key_width + key_width, p.y + white_key_height));
        ImU32 col = White;
        bool draw_text = false;
        char key_name_str = ' ';
        if (enable_input)
        {
            // Press down on current piano key
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && key_rect.Contains(io.MousePos) &&
                (key_rect.Max.y - io.MousePos.y) < (white_key_height - black_key_height))
            {
                keyStates[cur_key] = 127;
                _onPianoKeyPress(MIDI_STATUS_NOTE_ON, cur_key - 0x15, keyStates[cur_key]);
            }

            // Reset current key when mouse swipes out (Real keyboard's behavior)
            if (!key_rect.Contains(io.MousePos) && ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
                (key_rect.Max.y - io.MousePos.y) < (white_key_height - black_key_height))
            {
                keyStates[cur_key] = KEY_RELEASED;
                _onPianoKeyPress(MIDI_STATUS_NOTE_OFF, cur_key - 0x15, 0);
            }

#if 0 // Disable setting octaves by combination keys
            if (IsKeyDown(ImGuiKey_LeftShift) && !IsKeyDown(ImGuiKey_RightShift) && !IsKeyDown(ImGuiKey_RightAlt) && cur_key >= 24 && cur_key < 48)
            {
                col = White;
                draw_text = true;
                key_name_str = key_name[cur_key - 24];
            }
            else if (!IsKeyDown(ImGuiKey_LeftShift) && !IsKeyDown(ImGuiKey_RightShift) && !IsKeyDown(ImGuiKey_RightAlt) && cur_key >= 48 && cur_key < 72)
            {
                col = White;
                draw_text = true;
                key_name_str = key_name[cur_key - 48];
            }
            else if (IsKeyDown(ImGuiKey_RightAlt) && !IsKeyDown(ImGuiKey_LeftShift) && !IsKeyDown(ImGuiKey_RightShift) && cur_key >= 72 && cur_key < 96)
            {
                col = White;
                draw_text = true;
                key_name_str = key_name[cur_key - 72];
            }
            else if (IsKeyDown(ImGuiKey_RightShift) && !IsKeyDown(ImGuiKey_LeftShift) && !IsKeyDown(ImGuiKey_RightAlt) && cur_key >= 96 && cur_key < 120)
            {
                col = White;
                draw_text = true;
                key_name_str = key_name[cur_key - 96];
            }

            else
                col = White_Bark;
#endif
        }

        if (keyStates[cur_key])
        {
            if (keyStates[cur_key] == -1)
                col = EventSustained;
            else if (keyStates[cur_key] > 0)
            {
                int velocity = 255 - (keyStates[cur_key] > 127 ? 127 : keyStates[cur_key]) * 2;
                col = IM_COL32(velocity, 255, velocity, 255);
            }
        }

        draw_list->AddRectFilled(key_rect.Min, key_rect.Max, col, 2, ImDrawCornerFlags_All);
        draw_list->AddRect(key_rect.Min, key_rect.Max, Black, 2, ImDrawCornerFlags_All);
        if (draw_text && key_name_str != ' ')
        {
            ImFormatString(buf, 4, "%c\n", key_name_str);
            auto font_size = ImGui::CalcTextSize(buf);
            auto text_pos =
                ImVec2(key_rect.Min.x + (key_rect.GetWidth() - font_size.x) / 2, key_rect.Max.y - font_size.y - 4);
            draw_list->AddText(text_pos, IM_COL32_BLACK, buf);
        }

        cur_key++;
        if (has_black(key))
        {
            cur_key++;
        }
    }

    /** Draw black keys */
    cur_key = KEY_INDEX_START_BLACK;
    for (int key = 0; key < keyNum; key++)
    {
        if (has_black(key))
        {
            ImRect key_rect(ImVec2(p.x + key * key_width + key_width * 3 / 4, p.y),
                            ImVec2(p.x + key * key_width + key_width * 5 / 4 + 1, p.y + black_key_height));
            ImU32 col = Black;
            bool draw_text = false;
            char key_name_str = ' ';
            if (enable_input)
            {
                // Press down on current piano key
                if (key_rect.Contains(io.MousePos) && ImGui::IsMouseDown(ImGuiMouseButton_Left))
                    keyStates[cur_key] = 127;

                // Reset current key when mouse swipes out (Real keyboard's behavior)
                if (!key_rect.Contains(io.MousePos) && ImGui::IsMouseDown(ImGuiMouseButton_Left))
                    keyStates[cur_key] = 0;
#if 0 // Disable setting octaves by combination keys
                if (IsKeyDown(ImGuiKey_LeftShift) && !IsKeyDown(ImGuiKey_RightShift) && !IsKeyDown(ImGuiKey_RightAlt) && cur_key >= 24 && cur_key < 48)
                {
                    draw_text = true;
                    key_name_str = key_name[cur_key - 24];
                }
                else if (!IsKeyDown(ImGuiKey_LeftShift) && !IsKeyDown(ImGuiKey_RightShift) && !IsKeyDown(ImGuiKey_RightAlt) && cur_key >= 48 && cur_key < 72)
                {
                    draw_text = true;
                    key_name_str = key_name[cur_key - 48];
                }
                else if (IsKeyDown(ImGuiKey_RightAlt) && !IsKeyDown(ImGuiKey_LeftShift) && !IsKeyDown(ImGuiKey_RightShift) && cur_key >= 72 && cur_key < 96)
                {
                    draw_text = true;
                    key_name_str = key_name[cur_key - 72];
                }
                else if (IsKeyDown(ImGuiKey_RightShift) && !IsKeyDown(ImGuiKey_LeftShift) && !IsKeyDown(ImGuiKey_RightAlt) && cur_key >= 96 && cur_key < 120)
                {
                    draw_text = true;
                    key_name_str = key_name[cur_key - 96];
                }
#endif
            }

            if (keyStates[cur_key])
            {
                if (keyStates[cur_key] == KEY_SUSTAIN)
                    col = EventSustained;
                else if (keyStates[cur_key] > 0)
                {
                    int velocity = 255 - (keyStates[cur_key] > 127 ? 127 : keyStates[cur_key]) * 2;
                    col = IM_COL32(255, velocity, velocity, 255);
                }
            }
            draw_list->AddRectFilled(key_rect.Min, key_rect.Max, col, 2, ImDrawCornerFlags_All);
            draw_list->AddRect(key_rect.Min, key_rect.Max, Black, 2, ImDrawCornerFlags_All);
            if (draw_text && key_name_str != ' ')
            {
                ImFormatString(buf, 4, "%c\n", key_name_str);
                auto font_size = ImGui::CalcTextSize(buf);
                auto text_pos =
                    ImVec2(key_rect.Min.x + (key_rect.GetWidth() - font_size.x) / 2, key_rect.Max.y - font_size.y - 4);
                draw_list->AddText(text_pos, IM_COL32_WHITE, buf);
            }

            cur_key += 2;
        }
        else
        {
            cur_key++;
        }
    }
    if (enable_input)
    {
        /*                 1#  2#      4#  5#  6#
         *      ┏━━━┯━━━━┯━━━┯━━━┯━━━┯━━━┯━━━┯━━━┯━━━┯━━━┯━━━┯━━━┯━━━┯━━┓
         *      ┃ ` │ 1  │ 2 │ 3 │ 4 │ 5 │ 6 │ 7 │ 8 │ 9 │ 0 │ - │ = │BS┃
         *      ┠───┼───┬┴──┬┴──┬┴──┬┴──┬┴──┬┴──┬┴──┬┴──┬┴──┬┴──┬┴──┬┴──┨
         *      ┃Tab│Q1f│W2f│E3f│R4f│T5f│Y6f│U7f│ I │ O │ P │ [ │ ] │ | ┃
         *      ┠───┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴───┨
         *      ┃ Cap │ A │S1#│D2#│ F │G4#│H5#│J6#│ K │ L │ ; │ ' │ Ret ┃
         *      ┠─────┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─────┨
         * f2-> ┃ Shift │Z 1│X 2│C 3│V 4│B 5│N 6│M 7│ , │ . │ / │ Shift ┃<-g2
         *      ┠──┬───┬┴──┬┴──┬┴───┴───┴───┴───┴───┼───┼───┼───┼───┬───┨
         *      ┃Fn│Ctl│Alt│Cmd│      Space         │Cmd│Alt│ < │ v │ > ┃
         *      ┗━━┷━━━┷━━━┷━━━┷━━━━━━━━━━━━━━━━━━━━┷━━━┷━━━┷━━━┷━━━┷━━━┛
         *                             clean             g3
         */
        auto check_key = [&](ImGuiKey key, int ckey) {
            if (ImGui::IsKeyDown(key))
                keyStates[ckey] += 127;
            else if (ImGui::IsKeyReleased(key))
            {
                if (keyStates[ckey] > 0)
                {
                    keyStates[ckey] = KEY_RELEASED;
                    _onPianoKeyPress(MIDI_STATUS_NOTE_OFF, ckey - 0x15, 0);
                }
            }
        };

        if (IsKeyDown(ImGuiKey_Space) || ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            for (int i = 0; i < 256; i++)
            {
                if (keyStates[i] > 0)
                {
                    keyStates[i] = KEY_RELEASED;
                    //_onPianoKeyPress(MIDI_STATUS_NOTE_OFF, i - 0x15, 0);
                }
            }
        }
#if 0 // Disable playing with computer keyboard
        else if (IsKeyReleased(ImGuiKey_LeftShift))
        {
            for (int i = 0; i < 48; i++) if (key_states[i] > 0) key_states[i] = KEY_RELEASED;
        }
        else if (IsKeyReleased(ImGuiKey_RightAlt))
        {
            for (int i = 72; i < 96; i++) if (key_states[i] > 0) key_states[i] = KEY_RELEASED;
        }
        else if (IsKeyReleased(ImGuiKey_RightShift))
        {
            for (int i = 96; i < 120; i++) if (key_states[i] > 0) key_states[i] = KEY_RELEASED;
        }
        else
        {
            int cur_key = 48;
            if (IsKeyDown(ImGuiKey_LeftShift) && !IsKeyDown(ImGuiKey_RightShift) && !IsKeyDown(ImGuiKey_RightAlt))
                cur_key -= 12 * 2;
            else if (IsKeyDown(ImGuiKey_RightAlt) && !IsKeyDown(ImGuiKey_LeftShift) && !IsKeyDown(ImGuiKey_RightShift))
                cur_key += 12 * 2;
            else if (IsKeyDown(ImGuiKey_RightShift) && !IsKeyDown(ImGuiKey_LeftShift) && !IsKeyDown(ImGuiKey_RightAlt))
                cur_key += 12 * 4;
            // low key
            check_key(ImGuiKey_Q, cur_key++);   // F1
            check_key(ImGuiKey_2, cur_key++);   // F1#
            check_key(ImGuiKey_W, cur_key++);   // F2
            check_key(ImGuiKey_3, cur_key++);   // F2#
            check_key(ImGuiKey_E, cur_key++);   // F3
            check_key(ImGuiKey_R, cur_key++);   // F4
            check_key(ImGuiKey_5, cur_key++);   // F4#
            check_key(ImGuiKey_T, cur_key++);   // F5
            check_key(ImGuiKey_6, cur_key++);   // F5#
            check_key(ImGuiKey_Y, cur_key++);   // F6
            check_key(ImGuiKey_7, cur_key++);   // F6#
            check_key(ImGuiKey_U, cur_key++);   // F7
            // high key
            check_key(ImGuiKey_Z, cur_key++);   // C1
            check_key(ImGuiKey_S, cur_key++);   // C1#
            check_key(ImGuiKey_X, cur_key++);   // C2
            check_key(ImGuiKey_D, cur_key++);   // C2#
            check_key(ImGuiKey_C, cur_key++);   // C3
            check_key(ImGuiKey_V, cur_key++);   // C4
            check_key(ImGuiKey_G, cur_key++);   // C4#
            check_key(ImGuiKey_B, cur_key++);   // C5
            check_key(ImGuiKey_H, cur_key++);   // C5#
            check_key(ImGuiKey_N, cur_key++);   // C6
            check_key(ImGuiKey_J, cur_key++);   // C6#
            check_key(ImGuiKey_M, cur_key++);   // C7
        }
#endif
    }
    ImGui::InvisibleButton("##keyboard", size);
    ImGui::EndGroup();
}

void ImGui::Piano::reset()
{
    memset(keyStates, 0, sizeof(keyStates));
}

void ImGui::Piano::drawDebugTable()
{
    if (ImGui::BeginTable("##piano_debug", 16))
    {
        for (int i = 0; i < keyNum; i++)
        {
            ImGui::TableNextColumn();
            ImGui::Text("%d", keyStates[i]);
        }
        ImGui::EndTable();
    }
}

static void ImGui::default_piano_midi_event_callback(char midi_status, char note_id, char velocity)
{
    // fprintf(stderr, "[ImGui::Piano] Emit MIDI data: %2x %2x %2x\n", midi_status, note_id, velocity);
}
