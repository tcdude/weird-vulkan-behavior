#version 450

in vec3 pos;
in vec2 uv;
in vec3 normal;
uniform vec3 off;
uniform vec3 rot;

out vec2 frag_uv;
out vec3 frag_normal;
out vec3 positionWorldspace;
out vec3 normalCameraspace;
out vec3 eyeDirectionCameraspace;
out vec3 lightDirectionCameraspace;

uniform mat4 mvp;
uniform mat4 V;
uniform vec3 lightPos;

void main() {
	float sx = sin(-rot.x);
	float cx = cos(-rot.x);
	float sz = sin(-rot.y);
	float cz = cos(-rot.y);
	float sy = sin(-rot.z);
	float cy = cos(-rot.z);
	mat4 m = mat4(cx * cy, cx * sy * sz - sx * cz, cx * sy * cz + sx * sz, 0, sx * cy,
	              sx * sy * sz + cx * cz, sx * sy * cz - cx * sz, 0, -sy, cy * sz, cy * cz, 0.0,
	              0.0, 0.0, 0.0, 1.0);

	mat4 t =
	    mat4(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, off.x, off.y, off.z, 1.0);
    m = t * m;
	gl_Position = mvp * (m * vec4(pos, 1.0));

	positionWorldspace = (m * vec4(pos, 1.0)).xyz;

	// Vector that goes from the vertex to the camera, in camera space.
	// In camera space, the camera is at the origin (0,0,0).
	vec3 vertexPositionCameraspace = (V * m * vec4(pos, 1.0)).xyz;
	eyeDirectionCameraspace = vec3(0.0, 0.0, 0.0) - vertexPositionCameraspace;

	// Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
	vec3 lightPositionCameraspace = (V * vec4(lightPos, 1.0)).xyz;
	lightDirectionCameraspace = lightPositionCameraspace + eyeDirectionCameraspace;

	// Normal of the the vertex, in camera space
	normalCameraspace = (V * m * vec4(normal, 0.0)).xyz; // Only correct if modelMatrix does not scale the model! Use its inverse transpose if not.

	frag_uv = uv;
	frag_normal = normal;
}