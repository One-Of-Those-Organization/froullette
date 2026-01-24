#pragma once

static const char *vignete_fs =
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
    "uniform float uStr;\n"
    "uniform vec3 uColor;\n"
    "\n"
    "void main() {\n"
    "    vec2 U = gl_FragCoord.xy / iResolution.xy;\n"
    "    U *= 1.0 - U.yx;\n"
    "\n"
    "    float v = sqrt(sqrt(U.x * U.y * 15.0));\n"
    "    float dark = clamp((1.0 - v) * uStr, 0.0, 1.0);\n"
    "\n"
    "    FRAG_COLOR = vec4(uColor, dark);\n"
    "}\n";

static const char *vignete_vs =
#if defined(PLATFORM_WEB)
    "#version 100\n"
    "precision mediump float;\n"
    "attribute vec3 vertexPosition;\n"
    "attribute vec2 vertexTexCoord;\n"
    "varying vec2 fragTexCoord;\n"
    "uniform mat4 mvp;\n"
    "void main() {\n"
    "    fragTexCoord = vertexTexCoord;\n"
    "    gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
    "}\n";
#else
    "#version 330\n"
    "in vec3 vertexPosition;\n"
    "in vec2 vertexTexCoord;\n"
    "out vec2 fragTexCoord;\n"
    "uniform mat4 mvp;\n"
    "void main() {\n"
    "    fragTexCoord = vertexTexCoord;\n"
    "    gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
    "}\n";
#endif
