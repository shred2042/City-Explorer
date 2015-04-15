#version 330
layout(location = 0) out vec4 out_color;

in vec2 gf_texcoord;
in float f;
uniform sampler2D textura1;
uniform vec3 position, forward;
uniform float angle;
uniform int spotOn;

void main(){

		vec3 tex1 = texture(textura1, gf_texcoord).xyz;
		if (spotOn == 1) {			
			out_color = vec4(tex1, 0) * f;
		}
		else {
			out_color = vec4(tex1, 1);
		}
}