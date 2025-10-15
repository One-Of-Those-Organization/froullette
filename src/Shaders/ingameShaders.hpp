#ifndef INGAME_SHADERS_H_
#define INGAME_SHADERS_H_

const char *ingame_shaders=
"#version 300 es\n"
"precision mediump float;\n"

"uniform float sWidth;\n"
"uniform float sHeight;\n"
"out vec4 fragColor;\n"

"void main() {\n"
"    vec3 darkOrange = vec3(0.57, 0.37, 0.12);\n"
"    vec3 darkPurple = vec3(0.08, 0.00, 0.21);\n"

"    vec2 normalizedCoord = gl_FragCoord.xy / vec2(sWidth, sHeight);\n"
"    float gradientPosition = (normalizedCoord.x + normalizedCoord.y) / 2.5;\n"
"    gradientPosition = pow(gradientPosition, 1.5);\n"

"    vec3 baseColor = mix(darkPurple, darkOrange, gradientPosition);\n"

"    float blurFactor = 0.005;\n"
"    vec3 blurredColor = ("
"        mix(darkPurple, darkOrange, gradientPosition + blurFactor) +"
"        mix(darkPurple, darkOrange, gradientPosition - blurFactor) +"
"        mix(darkPurple, darkOrange, gradientPosition + 2.0 * blurFactor) +"
"        mix(darkPurple, darkOrange, gradientPosition - 2.0 * blurFactor)"
"    ) * 0.25;\n"

"    fragColor = vec4(blurredColor, 1.0);\n"
"}";

#endif // INGAME_SHADERS_H_
