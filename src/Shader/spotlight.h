#pragma once

static const char *spotlight_fs =
#if defined(PLATFORM_WEB)
"#version 100\n"
"precision mediump float;\n"
"#define FRAG_COLOR gl_FragColor\n"
#else
"#version 330\n"
"out vec4 FRAG_COLOR;\n"
#endif
"\n"
"uniform vec2 iResolution;\n"
"uniform vec2 iMouse;   // screen-space pixels\n"
"uniform vec3 lightColor;\n"
"uniform float radius;\n"
"uniform float str;\n"
"\n"
"void main() {\n"
"    vec2 fragCoord = gl_FragCoord.xy;\n"
"\n"
"    float d = distance(iMouse, fragCoord);\n"
"    float intensity = str - smoothstep(0.0, radius, d);\n"
"\n"
"    vec3 color = lightColor * intensity;\n"
"    FRAG_COLOR = vec4(color, intensity);\n"
"}\n";
static const char *spotlight_vs =
#if defined(PLATFORM_WEB)
"#version 100\n"
"precision mediump float;\n"
"attribute vec3 vertexPosition;\n"
"uniform mat4 mvp;\n"
"void main() {\n"
"    gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
"}\n";
#else
"#version 330\n"
"in vec3 vertexPosition;\n"
"uniform mat4 mvp;\n"
"void main() {\n"
"    gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
"}\n";
#endif
