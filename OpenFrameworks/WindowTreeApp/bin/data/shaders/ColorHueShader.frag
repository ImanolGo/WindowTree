#version 120

uniform sampler2DRect tex0;
uniform vec4 color = vec4(1.0,1.0,1.0, 1.0);

varying vec2 texCoordVarying;

float luma(vec3 color) {
  return dot(color, vec3(0.299, 0.587, 0.114));
}

float luma(vec4 color) {
  return dot(color.rgb, vec3(0.299, 0.587, 0.114));
}

void main()
{
    vec4 tex = texture2DRect(tex0, texCoordVarying);
    float brightness = luma(tex); 
    gl_FragColor = vec4(color.rgb*brightness, tex.a*color.a);
}