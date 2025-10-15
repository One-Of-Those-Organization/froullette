#ifndef BEADSHADERS_H_
#define BEADSHADERS_H_

const char *bead_shaders=
"#version 300 es\n"
"precision mediump float;\n"

"in vec2 fragTexCoord;\n"
"in vec4 fragColor;\n"
"out vec4 finalColor;\n"

"uniform sampler2D texture0;\n"
"uniform vec2 texelSize; // set to (1.0 / texture width, 1.0 / texture height)\n"

"void main() {\n"
"    float alpha = texture(texture0, fragTexCoord).a;\n"
"    if (alpha > 0.0) {\n"
"        finalColor = texture(texture0, fragTexCoord);\n"
"        return;\n"
"    }\n"

"    float outline = 0.0;\n"
"    for (int x = -1; x <= 1; x++) {\n"
"        for (int y = -1; y <= 1; y++) {\n"
"            if (x == 0 && y == 0) continue;\n"
"            vec2 offset = vec2(x, y) * texelSize;\n"
"            outline += texture(texture0, fragTexCoord + offset).a;\n"
"        }\n"
"    }\n"

"    if (outline > 0.0) {\n"
"        finalColor = vec4(1.0, 1.0, 1.0, 0.7); // White outline\n"
"    } else {\n"
"        finalColor = vec4(0.0); // Transparent\n"
"    }\n"
"}\n";

#endif // BEADSHADERS_H_
