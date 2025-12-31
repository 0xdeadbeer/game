$input f_position

#include <bgfx_shader.sh>
#include <shaderlib.sh>

void main() {
  gl_FragColor = vec4(sin(f_position.x), cos(f_position.y), sin(f_position.z), 1.0f);
}
