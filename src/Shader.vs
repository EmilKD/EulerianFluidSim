#version 330 core
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec3 aColor;

out vec3 vColor;

uniform float pointSize;

void main() {
    gl_Position = vec4(aPosition, 0.0, 1.0);
    gl_PointSize = pointSize; // Adjust size based on needs
    vColor = aColor;
}