#version 330 core
smooth in vec2 offset;
out vec4 fragment;
uniform sampler2D sampler;
void main() {
  /* Taking the alpha channel here gives you a white texture! */
  float sample = texture(sampler, offset).r;
  float alpha = smoothstep(0.40, 0.51, sample);
  fragment = vec4(1, 1, 1, alpha);
}
