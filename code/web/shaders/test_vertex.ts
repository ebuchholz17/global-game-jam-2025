let test_vertex_source = `
struct tri_transform {
    vec2 pos;
    float rotation;
};

uniform tri_transform triTransform;

attribute vec3 position;
attribute vec3 color;

varying vec3 vColor;

void main() {
    mat3 transform = mat3(cos(triTransform.rotation), sin(triTransform.rotation), 0.0, 
                          -sin(triTransform.rotation), cos(triTransform.rotation), 0.0, 
                          triTransform.pos.x, triTransform.pos.y, 1.0);
    vec3 pos = vec3(position.x, position.y, 1.0);
    pos = vec3(transform * pos);
    gl_Position = vec4(pos.x, pos.y, -2.0 * (position.z - 0.5), 1.0); // use 0-1 for z like dx
    gl_Position.y = -gl_Position.y;
    vColor = color;
}`;


let test_fragment_source = `
precision highp float;

varying vec3 vColor;

void main() {
    gl_FragColor = vec4(vColor, 1.0);
}`;

