#version 330 core
out vec4 fragColor;

in vec3 outColor;
in vec2 TexCoord;

uniform sampler2D TextureParam;
uniform vec3 myColor;


void main()
{
	// Vertex color based
	fragColor = vec4(myColor, 1.0f);

	// Texture based
	//fragColor = vec4(myColor, 1.0f) * texture(TextureParam, TexCoord);
}
