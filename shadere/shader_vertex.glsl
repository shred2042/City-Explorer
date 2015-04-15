#version 330

layout(location = 0) in vec3 in_position;		
layout(location = 1) in vec3 in_color;		

uniform mat4 model_matrix, view_matrix, projection_matrix;
uniform float h, w;
uniform int flag;
uniform vec3 position, forward;
uniform float angle;
uniform int spotOn;

out vec2 gf_texcoord;
out float f;

void main(){
	gl_Position = projection_matrix*view_matrix*model_matrix*vec4(in_position,1);
	
	float depth = 2000;
	vec3 direction = in_position - position;
	float cosinus = dot(forward, -direction)/sqrt((forward.x*forward.x + forward.y*forward.y + forward.z*forward.z)*
															(direction.x*direction.x + direction.y*direction.y + direction.z*direction.z));
	if (cosinus < 0) {
		cosinus = -cosinus;
	}

	if (cosinus > angle) { 
		f = cosinus - distance(in_position, position)/depth;
		f = max(f, 0);
	}
	else {
		f = 0;
	}
	if (flag == 0) {
		gf_texcoord = vec2(0, 0);
		f = 1;
	}
	else {
		gf_texcoord = vec2(in_position.x/w, in_position.y/h);
	}
}
