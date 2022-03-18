#ifndef __GUI_H__
#define __GUI_H__

#ifdef ANDROID
	#include "../../lib/imgui/imgui.h"
	#include "../../lib/imgui/backends/imgui_impl_android.h"
#else
	#include "../../lib/imgui/backends/imgui_impl_glfw.h"
	#include "../../lib/imgui/backends/imgui_impl_opengl3.h"
#endif
#include "../../lib/raylib/src/raylib.h"
#include "main.h"

extern Color BG_COLOR;
extern Color FG_COLOR;

void initHitBox();
void grid();
void ImGui_ImplRaylib_LoadDefaultFontAtlas();
void ImGui_ImplRaylib_Render(ImDrawData *draw_data);
int join_window(char *IP_ADDRESS, int *PORT, struct client_data *data);
void matchInfo(struct client_data *data);

#ifdef ANDROID
	#define STTT_TEXT_SIZE 64
void toggleKeyboard(bool show);
#else
	#define STTT_TEXT_SIZE 20
#endif

#endif // __GUI_H__