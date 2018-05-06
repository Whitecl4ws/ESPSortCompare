#version 330 core
in vec2 point;
in vec2 position;
in vec2 pivot;
in vec2 size;
in vec2 rotation;
in vec4 color;
out vec4 Color;
uniform mat4 projection;
void main() {
  Color = color;
  vec2 Point = (point - pivot) * size;
  if(rotation.x != 0) {
    vec2 Rotation;
    float c = cos(rotation.x);
    float s = sin(rotation.x);
    Rotation.x = Point.x * c - Point.y * s;
    Rotation.y = Point.y * c  + Point.x * s;
    Point = Rotation;
  }

  Point = Point + position;
  gl_Position = projection * vec4(Point, -1, 1);
}
