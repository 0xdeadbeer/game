$input f_position

uniform vec4 u_flags;

#include <bgfx_shader.sh>
#include <shaderlib.sh>

float k = 100.0f; 
vec2 direction = vec2(10.0f, 5.0f); 

void main() {
  float time = u_flags.x; 

  vec4 pos = mul(u_modelViewProj, vec4(f_position, 1.0f)); 
  
  float x = pos.x;
  float y = pos.y;
  
  // float a = sin(pow(x, 2.0f)) - cos(pow(y, 2.0f));
  // 
  // if (abs(a-sin(time*4.0f)) <= 0.5f) {
  //     gl_FragColor = vec4(0.0f, 0.0f, 1.0f, 1.0f); 
  //     return; 
  // }
  // 
  // gl_FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);

  
  float a = sin(k*x+(time*direction.x)) - sin(k*y+(time+direction.y));
  
  if (abs(a) <= 0.2f) {
      gl_FragColor= vec4(0.0f, 1.0f, 0.0f, 1.0f); 
      return; 
  }
  
  gl_FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
