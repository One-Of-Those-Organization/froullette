#ifndef MENUSHADERS_H_
#define MENUSHADERS_H_

const char *menu_shaders =
"#version 300 es\n"
"precision mediump float;\n"

"uniform float sWidth;\n"
"uniform float sHeight;\n"
"out vec4 fragColor;\n"

"void main() {\n"
"    vec3 orangeColor = vec3(0.97, 0.67, 0.32);\n"
"    vec3 purpleColor = vec3(0.18, 0.01, 0.31);\n"

"    vec2 normalizedCoord = gl_FragCoord.xy / vec2(sWidth, sHeight);\n"
"    float gradientPosition = (normalizedCoord.x + normalizedCoord.y) / 2.5;\n"
"    gradientPosition = pow(gradientPosition, 1.2);\n"
"    vec3 finalColor = mix(purpleColor, orangeColor, gradientPosition);\n"
"    fragColor = vec4(finalColor, 1.0);\n"
"}";

#endif // MENUSHADERS_H_
