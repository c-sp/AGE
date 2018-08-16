
// version directive and OpenGL ES precision qualifiers
// are added by the shader loader

uniform sampler2D texture;

varying vec2 v_texture_size;
const vec2 c_pixel_center = vec2(0.5, 0.5);


void main()
{
    vec2 frag_coord = vec2(gl_FragCoord);// + c_pixel_center;

    vec2 tex_coord1 = (frag_coord + vec2(-1.0, 1.0)) / v_texture_size;
    vec2 tex_coord2 = frag_coord / v_texture_size;
    vec2 tex_coord3 = (frag_coord + vec2(1.0, -1.0)) / v_texture_size;

    vec4 c1 = texture2D(texture, tex_coord1);
    vec4 c2 = texture2D(texture, tex_coord2);
    vec4 c3 = texture2D(texture, tex_coord3);

    vec4 color = -c1 + 4.0 * c2 + c3;
    color /= 4.0;

    gl_FragColor = color;
}
