#version 450

in vec2 frag_uv;
in vec3 frag_normal;
in vec3 positionWorldspace;
in vec3 normalCameraspace;
in vec3 eyeDirectionCameraspace;
in vec3 lightDirectionCameraspace;

out vec4 FragColor;

uniform vec3 lightPos;

void main() {
	vec3 lightColor = vec3(1.0);
	float lightPower = 4.0;

	// Material properties
	vec3 materialDiffuseColor = frag_normal / 0.5 + 0.5;
	vec3 materialAmbientColor = vec3(0.3) * materialDiffuseColor;
	vec3 materialSpecularColor = vec3(1.0);

	// Distance to the light
	float distance = length(lightPos - positionWorldspace);

	// Normal of the computed fragment, in camera space
	vec3 n = normalize(normalCameraspace);
	// Direction of the light (from the fragment to the light)
	vec3 l = normalize(lightDirectionCameraspace);
	// Cosine of the angle between the normal and the light direction,
	// clamped above 0
	//  - light is at the vertical of the triangle -> 1
	//  - light is perpendicular to the triangle -> 0
	//  - light is behind the triangle -> 0
	float cosTheta = clamp(dot(n, l), 0.0, 1.0);

	// Eye vector (towards the camera)
	vec3 E = normalize(eyeDirectionCameraspace);
	// Direction in which the triangle reflects the light
	// vec3 R = (-l) - 2.0 * dot(n, (-l)) * n;
	vec3 R = reflect(-l, n); // TODO: waiting for krafix fix

	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp(dot(E, R), 0.0, 1.0);

	FragColor =
	    vec4(vec3(
	             // Ambient: simulates indirect lighting
	             materialAmbientColor +
	             // Diffuse: "color" of the object
	             materialDiffuseColor * lightColor * lightPower * cosTheta / (distance * distance) +
	             // Specular: reflective highlight, like a mirror
	             materialSpecularColor * lightColor * lightPower * pow(cosAlpha, 5.0) /
	                 (distance * distance)),
	         1.0);
}
