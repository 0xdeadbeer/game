$input a_position
$output f_position

uniform vec4 u_position;
uniform vec4 u_rotation;
uniform vec4 u_scale;

uniform vec4 u_flags;

#include <bgfx_shader.sh>
#include <shaderlib.sh>

void main() {
  gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0f));
  f_position = a_position;
}
